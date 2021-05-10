/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Setting Server
                  -------------------------

                  SW Module Document




@file        SettingSrv.c
@brief       This implement the server for all settings store and retrieve
@author      Wesley Lee
@date        2014-06-09
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2014-06-09     Wesley Lee
DESCRIPTION: First Draft. Generated by newclass.py
SCO/ERROR  :
-------------------------------------------------------------------------------
*/

#include "product.config"
#ifdef HAS_SETTING

//#include "./SettiResetIRConditionsngSrv_priv.h"
#include "StorageDrv.h"
#ifdef HAS_NVM
#include "NvmDrv.h"
#endif
#include "controller.h"
#include "trace.h"
#include "bsp.h"
#include "AudioDrv.h"
#include "SettingSrv_priv.h"
#include "SettingSrv.config"
#include "SettingSrv.h"
#ifdef HAS_PRINTOUT_WHEN_WRITE_INTO_FLASH
#include "DebugSSrv.h"
#endif

#define CAST_ME cSettingSrv * SettingSrv = (cSettingSrv *) me;

/* SETTING_HAS_ROM_DATA should be defined if there is ROM data
 *  - direct write data from Debug Server, i.e. TP_Sneak
 *  - writing flash with delay,
 */

#ifdef SETTING_HAS_ROM_DATA
/* Private functions / variables. Declare and drivers here */
/* Internal event queue - Size as needed */
static QEvt const *SettingEvtQue[SETT_SRV_EVENT_Q_SIZE];

cNvmDrv nvmDrv;

//use for the timer of Setting_Set() function
static cSettingSrv * pSettingSrv= NULL;

#define TIMER_IS_NOT_SETUP  (-1)
static int16 save_timeout = TIMER_IS_NOT_SETUP;
static int16 idle_timeout = TIMER_IS_NOT_SETUP;

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void SettingSrv_StartUp(cPersistantObj *me)
{
    CAST_ME;
    pSettingSrv = SettingSrv;
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(SettingSrv);
    QS_OBJ_DICTIONARY(SettingSrv_Active);
    QS_OBJ_DICTIONARY(SettingSrv_DeActive);

    SettingSrv->pStorageDrv = (cStorageDrv *)&nvmDrv;
    StorageDrv_Ctor(SettingSrv->pStorageDrv,
                    (tStorageDevice*)getDevicebyId(INT_FLASH_DEV_ID, NULL));

    /* start up the object and let it run. including the timer*/
    Server_Ctor((cServer*)me, Q_STATE_CAST(&SettingSrv_Initial), SETTING_TIMEOUT_SIG,
                       SettingEvtQue, Q_DIM(SettingEvtQue), SETTING_SRV_ID);
    /* Subscribe */
}

void SettingSrv_ShutDown(cPersistantObj *me)
{
    /* Clean memory and shut-down. Called by the controller */
    Server_Xtor((cServer*)me);
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
QState SettingSrv_Initial(cSettingSrv * const me, QEvt const * const e)
{
    SettingSrv_LoadStoredValue(me);
    return Q_TRAN(&SettingSrv_DeActive);
}

/*   Active state - first state where "normal" service begins  */
QState SettingSrv_Active(cSettingSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        PersistantObj_RefreshTick((cPersistantObj*)me, GET_TICKS_IN_MS(TIMER_PERIOD_MS));
        return Q_HANDLED();
    }
    case SYSTEM_ACTIVE_REQ_SIG:
    {
        CommonReqEvt* pReq = (CommonReqEvt*) e;
        CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
        return Q_HANDLED();
    }
    case SYSTEM_SLEEP_REQ_SIG:
    {
        CommonReqEvt* pReq = (CommonReqEvt*) e;
        me->pRequestor = pReq->sender;
        SettingSrv_Bookkeeping(me);
        return Q_TRAN(&SettingSrv_DeActive);
    }
    case SETTING_WRITE_OFFSET_REQ_SIG:
    {
        SettingWriteOffsetReqEvt* pReq = (SettingWriteOffsetReqEvt*) e;
        me->processed = 0;
        me->processingId = pReq->setting_id;
        if (pReq->size > settingDB[pReq->setting_id].size)
        {
            SettingSrv_Write(me, pReq->data, settingDB[pReq->setting_id].size, pReq->offset);
        }
        else
        {
        SettingSrv_Write(me, pReq->data, pReq->size, pReq->offset);
        }
        SettingSrv_SettUpdateNotification(pReq->setting_id, GetServerID(pReq->sender));
        if (getRomAddr(pReq->setting_id))
        {
            if(pSettingSrv == NULL)
            { /* if pSettingSrv is not assigned, assert it*/
                ASSERT(0);
            }
            else
            {
                save_timeout = 0;
            }
        }
        return Q_HANDLED();
    }
    case SETTING_READ_OFFSET_REQ_SIG:
    {
        SettingReadOffsetReqEvt* pReq = (SettingReadOffsetReqEvt*)e;
        me->processed = 0;
        me->pRequestor = pReq->sender;
        me->processingId = pReq->setting_id;
        SettingReadOffsetRespEvt *pResp = Q_NEW(SettingReadOffsetRespEvt,SETTING_READ_OFFSET_RESP_SIG);
        pResp->offset = pReq->offset;
        pResp->size = pReq->size;
        pResp->setting_id = pReq->setting_id;
        memset(pResp->data,0x00,sizeof(pResp->data));
        SettingSrv_Read(me, pResp->data, pReq->size, pReq->offset);
        pResp->evtReturn = RET_SUCCESS;
        QACTIVE_POST(me->pRequestor, (QEvt*)pResp, me);
        return Q_HANDLED();
    }
    case SETTING_START_REQ_SIG:
    {
        SettingStartReqEvt* pReq = (SettingStartReqEvt*) e;
        CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SETTING_START_RESP_SIG);
        if (settingDB[pReq->id].attr & SETTING_ATTR_NVM)
        {
            me->pRequestor  = pReq->sender;
            me->processed   = 0;
            me->resendCnt   = 0;
            me->processingId= pReq->id;

            // TODO: publish a signal to let everyone know setting server is busy

            // TODO: wait a timeout to make sure that everyone is agree to write something to flash
            // TODO: ask Miro for how to know the interaction between server if one is in special situation, wait, signal?
            // TODO: if it is in critical situation, don't save anything
            if (pReq->direction == SETTING_DIRECTION_DOWNLOAD)
            {
                me->size = pReq->size;
                return Q_TRAN(&SettingSrv_BusyWrite);
            }
            else
            {
                me->size = settingDB[me->processingId].size;
                return Q_TRAN(&SettingSrv_BusyRead);
            }
        }
        return Q_HANDLED();
    }
    case SETTING_FLASH_REQ_SIG:
    {
        SettingFlashReqEvt* pReq = (SettingFlashReqEvt*)e;
        if (pReq->bIsSave)
        {
            SettingSrv_Bookkeeping(me);
        }
        else
        {
            SettingSrv_LoadStoredValue(me);
        }
        return Q_HANDLED();
    }
    case SETTING_TIMEOUT_SIG:
      {
        if (save_timeout > 0)
        {
            save_timeout -= TIMER_PERIOD_MS;
        }
        else
        {
            if (save_timeout != TIMER_IS_NOT_SETUP)
            {
                SettingSrv_Bookkeeping(me);
                save_timeout = TIMER_IS_NOT_SETUP;
            }
        }
        if (idle_timeout)
        {
            idle_timeout -= TIMER_PERIOD_MS;
        }
        else
        {
            if (idle_timeout != TIMER_IS_NOT_SETUP)
            {
                idle_timeout = TIMER_IS_NOT_SETUP;
                return Q_TRAN(&SettingSrv_Active);
            }
        }
        PersistantObj_RefreshTick((cPersistantObj*)pSettingSrv, GET_TICKS_IN_MS(TIMER_PERIOD_MS));
        return Q_HANDLED();
      }
    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}

QState SettingSrv_BusyRead(cSettingSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
      if (me->size < SETTING_CHUNK_SIZE)
      {
          SettingEndReqEvt* pReq = Q_NEW(SettingEndReqEvt, SETTING_END_REQ_SIG);
          SettingSrv_Read(me, pReq->data, me->size, NO_OFFSET);
          pReq->rest_size = me->size;
          QACTIVE_POST(me->pRequestor, (QEvt*)pReq, me);
      }
      else
       {
        SettingDataReqEvt* pReq = Q_NEW(SettingDataReqEvt, SETTING_DATA_REQ_SIG);
        me->pStorageDrv->GetValue(me->pStorageDrv, getRomAddr(me->processingId),
                                    pReq->data, SETTING_CHUNK_SIZE);
        QACTIVE_POST(me->pRequestor, (QEvt*)pReq, me);
       }
        idle_timeout = IDLE_TIMEOUT_MS;

        return Q_HANDLED();
    }
    case SETTING_START_REQ_SIG:
    {
        SettingDataReqEvt* pReq = (SettingDataReqEvt*) e;
        CommonEvtResp((QActive*)me, pReq->sender, RET_BUSY, SETTING_DATA_RESP_SIG);
        return Q_HANDLED();
    }
    case SETTING_DATA_RESP_SIG:
    {
        SettingDataRespEvt* pResp = (SettingDataRespEvt*) e;

        // check if last sent is success
        if (pResp->evtReturn == RET_SUCCESS)
        {
            // if last data is success, iterate to send new packet of data
            me->processed   += SETTING_CHUNK_SIZE;
            me->resendCnt   =  0;
        }
        else
        {
            ++me->resendCnt;
            if (me->resendCnt > SETTING_RESEND_LIMIT)
            {
                idle_timeout = TIMER_IS_NOT_SETUP;
                // if reach the maximum number of resend
                return Q_TRAN(&SettingSrv_Active);
            }
        }

        // send the data
        if (SETTING_CHUNK_SIZE <= me->size - me->processed)
        {
            SettingDataReqEvt* pReq = Q_NEW(SettingDataReqEvt, SETTING_DATA_REQ_SIG);
            SettingSrv_Read(me, pReq->data, SETTING_CHUNK_SIZE, NO_OFFSET);
            QACTIVE_POST(me->pRequestor, (QEvt*)pReq, me);
        }
        else
        {
            SettingEndReqEvt* pReq = Q_NEW(SettingEndReqEvt, SETTING_END_REQ_SIG);
            pReq->rest_size = me->size - me->processed;
            SettingSrv_Read(me, pReq->data, pReq->rest_size, NO_OFFSET);
            QACTIVE_POST(me->pRequestor, (QEvt*)pReq, me);
        }
        idle_timeout = IDLE_TIMEOUT_MS;
        return Q_HANDLED();
    }
    case SETTING_END_RESP_SIG:
    {
        SettingEndRespEvt* pResp = (SettingEndRespEvt*) e;

        if (pResp->evtReturn != RET_SUCCESS)
        {
            ++me->resendCnt;
            if (me->resendCnt > SETTING_RESEND_LIMIT)
            {
                idle_timeout = TIMER_IS_NOT_SETUP;
                // if reach the maximum number of resend
                return Q_TRAN(&SettingSrv_Active);
            }

            // if last packet of data is not successfully sent
            // resend the last packet of data
            // wait for another SETTING_END_RESP_SIG
            SettingEndReqEvt* pReq = Q_NEW(SettingEndReqEvt, SETTING_END_REQ_SIG);
            pReq->rest_size = me->size - me->processed;
            SettingSrv_Read(me, pReq->data, pReq->rest_size, NO_OFFSET);
            QACTIVE_POST(me->pRequestor, (QEvt*)pReq, me);
            idle_timeout = IDLE_TIMEOUT_MS;
            return Q_HANDLED();
        }
        idle_timeout = TIMER_IS_NOT_SETUP;
        // TODO: publish a signal to let everyone know setting server is idle
        return Q_TRAN(&SettingSrv_Active);
    }
    default:
        break;
    }
    return Q_SUPER(&SettingSrv_Active);
}

QState SettingSrv_BusyWrite(cSettingSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        if (settingDB[me->processingId].p == NULL)
        {
            uint16 i = 0;

            for ( ; i <= me->size; i+=PAGE_SIZE)
            {
                me->pStorageDrv->ErasePage(me->pStorageDrv, getRomAddr(me->processingId) + i);
            }
        }
        idle_timeout = IDLE_TIMEOUT_MS;
        return Q_HANDLED();
    }
    case SETTING_START_REQ_SIG:
    {
         SettingDataReqEvt* pReq = (SettingDataReqEvt*) e;
        CommonEvtResp((QActive*)me, pReq->sender, RET_BUSY, SETTING_DATA_RESP_SIG);
        return Q_HANDLED();
    }
    case SETTING_DATA_REQ_SIG:
    {
        SettingDataReqEvt* pReq = (SettingDataReqEvt*) e;

        SettingSrv_Write(me, pReq->data, SETTING_CHUNK_SIZE, NO_OFFSET);
        CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SETTING_DATA_RESP_SIG);
        idle_timeout = IDLE_TIMEOUT_MS;
        return Q_HANDLED();
    }
    case SETTING_END_REQ_SIG:
    {
        SettingEndReqEvt* pReq = (SettingEndReqEvt*) e;

        // write the last chunk of data
        SettingSrv_Write(me, pReq->data, pReq->rest_size, NO_OFFSET);
        CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SETTING_END_RESP_SIG);

        // Notification on setting updated
        SettingSrv_SettUpdateNotification(me->processingId, GetServerID(pReq->sender));

        if (getRomAddr(me->processingId))
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, GET_TICKS_IN_MS(TIMER_PERIOD_MS));
        }
        idle_timeout = TIMER_IS_NOT_SETUP;
        return Q_TRAN(&SettingSrv_Active);
    }
    default:
        break;
    }
    return Q_SUPER(&SettingSrv_Active);
}

QState SettingSrv_DeActive(cSettingSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        CommonEvtResp((QActive*)me, me->pRequestor, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
        me->pRequestor = NULL;
        return Q_HANDLED();
    }
    case SYSTEM_ACTIVE_REQ_SIG:
    {
        CommonReqEvt* pReq = (CommonReqEvt*) e;
        CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
        return Q_TRAN(&SettingSrv_Active);
    }
    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}
#endif /* #ifdef SETTING_HAS_ROM_DATA */

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
#ifdef SETTING_HAS_ROM_DATA

/* @brief       Read the data from RAM/ROM
 * @param[in]   me      Setting Sever object
 * @param[in]   pData   pointer to expected memory to be filled
 * @param[in]   size    size of the expected data
 */
static void SettingSrv_Read(cSettingSrv * const me, uint8* pData, uint16 size, uint16 offset)
{
    if (settingDB[me->processingId].p)
    {
        // entity in RAM
        uint8 *p = settingDB[me->processingId].p;
        p += me->processed;
        p += offset;
        memcpy(pData, p, size);
    }
    else
    {
        // entity ONLY in ROM
        me->pStorageDrv->GetValue(me->pStorageDrv, getRomAddr(me->processingId) + me->processed,
                                 pData, size);
    }

}

/* @brief       Write the data to RAM/ROM
 * @param[in]   me      Setting Sever object
 * @param[in]   pData   pointer to incoming data to be written
 * @param[in]   size    size of the incoming data
 * @param[in]   offset  offset from starts of settingId address
 */
static void SettingSrv_Write(cSettingSrv * const me, uint8* pData, uint16 size, uint16 offset)
{
    if (settingDB[me->processingId].p)
    {
        // entity in RAM
        uint8 *p = settingDB[me->processingId].p;
        p += me->processed;
        p += offset;
        memcpy(p, pData, size);
    }
    else
    {
        // entity ONLY in ROM
        me->pStorageDrv->SetValue(me->pStorageDrv, getRomAddr(me->processingId) + me->processed + offset,
                                    pData, size);
    }
    me->processed += size;
}

/* @brief       Load the data from ROM to RAM
 * @param[in]   me      Setting Sever object
 */
static void SettingSrv_LoadStoredValue(cSettingSrv * const me)
{
    uint16 i = 0;
    bool result = TRUE;

    for ( ; i<ArraySize(settingRomMap); i++)
    {
        eSettingId id = settingRomMap[i].id;
        if (settingDB[id].p)
        {
#ifdef HAS_CBUFFER
            result = me->pStorageDrv->GetValueCBuffer(me->pStorageDrv, i, settingDB[id].p);
#else
            me->pStorageDrv->GetValue(me->pStorageDrv, settingRomMap[i].addr,
                     settingDB[id].p, settingDB[id].size);
#endif
            if (TRUE == result)
            {
                TYM_SET_BIT(settingDB[id].attr, SETTING_ATTR_SET);
            }
        }
    }
}



/* @brief       Save the data from RAM to ROM
 * @param[in]   me      Setting Sever object
 */
void SettingSrv_BookkeepingEx()
{
    if(pSettingSrv) 
    {
        SettingSrv_Bookkeeping(pSettingSrv);
    }
}



/* @brief       Save the data from RAM to ROM
 * @param[in]   me      Setting Sever object
 */
static void SettingSrv_Bookkeeping(cSettingSrv * const me)
{
    uint16 i = 0;
    /* PIC32 do NOT require an extra erase page opration before writing data.
     * The erase page opration will be done in the write API : NVMProgram()
     */
#ifndef HAS_CBUFFER
    if(me->pStorageDrv->ErasePage)
    {
        me->pStorageDrv->ErasePage(me->pStorageDrv, SETT_PAGE_ROM_ADDR);
    }
#endif

    for ( ; i<ArraySize(settingRomMap); i++)
    {
        if (settingRomMap[i].addr)
        {
            eSettingId id = settingRomMap[i].id;
            if ((settingRomMap[i].addr) && ( (settingDB[id].p) ))
            {
#ifdef SETT_ELEMENT_MIN_SIZE
                /* Note as the requirement of nvm driver, data stream should be in size of multiple of 4
                 * So if the size of item is less than 4 bytes, just write 4 bytes into flash.
                 * but if the size of item is lager than 4 bytes, please make sure it is multiple of 4 bytes.
                 */
                if(settingDB[id].size < SETT_ELEMENT_MIN_SIZE)
                {
                    uint32 buf = 0;
                    memcpy(&buf, settingDB[id].p, settingDB[id].size);
#ifdef HAS_CBUFFER
                    me->pStorageDrv->SetValueCBuffer(me->pStorageDrv, i, settingDB[id].p);
#else
                    me->pStorageDrv->SetValue(me->pStorageDrv, settingRomMap[i].addr, (uint8 *)&buf, sizeof(buf));
#endif
                }
                else
#endif //#ifdef SETT_ELEMENT_MIN_SIZE
                {
#ifdef HAS_CBUFFER
                    me->pStorageDrv->SetValueCBuffer(me->pStorageDrv, i, settingDB[id].p);
#else
                    me->pStorageDrv->SetValue(me->pStorageDrv, settingRomMap[i].addr, settingDB[id].p, settingDB[id].size);
#endif
                }
            }
        }
    }
#ifdef HAS_PRINTOUT_WHEN_WRITE_INTO_FLASH
        DebugSSrv_PrintStr("DATA SAVED INTO FLASH.OK");
#endif
}
#endif

/* @brief       Get the ROM address of the setting ID
 * @param[in]   id      ID of the entity to be obtained
 * @return      ROM address
 */
static uint32 getRomAddr(eSettingId id)
{
    uint32 ret = NULL;
    uint16 i = 0;
    for ( ; i<ArraySize(settingRomMap); i++)
    {
        if (settingRomMap[i].id == id)
        {
            ret = settingRomMap[i].addr;
            break;
        }
    }
    return ret;
}

/*****************************************************************************************************************
 *
 * public functions
 *
 *****************************************************************************************************************/
void SettingSrv_InitDB(void)
{
    eSettingId i = SETID_START;

    for ( ; i < SETID_MAX; i++)
    {
        // mark all the entities are not being set at start-up
        TYM_CLR_BIT(settingDB[i].attr, SETTING_ATTR_SET);
    }
}

/* @brief       Check if the ID is valid
 * @param[in]   id      ID of the entity to be obtained
 * @return      TRUE when the entity ID is valid
 */
bool Setting_IsIdValid(eSettingId id)
{
    bool ret = settingDB[id].attr & SETTING_ATTR_VALID ? TRUE : FALSE;
    return ret;
}

/* @brief       Check if the entity in RAM is ready to get
 * @param[in]   id      ID of the entity to be obtained
 * @return      TRUE when the entity ID is valid
 */
bool Setting_IsReady(eSettingId id)
{
    bool ret = NULL;
    if (Setting_IsIdValid(id))
    {
        ret = settingDB[id].attr & SETTING_ATTR_SET ? TRUE : FALSE;
    }
    return ret;
}


/* @brief       Set ID to unset status
 * @param[in]   id      ID of the entity to be obtained
 */
void Setting_Reset(eSettingId id)
{
    ASSERT(id < SETID_MAX);
    
    /* Reset function not support EEPROM */
    ASSERT( (settingDB[id].attr & SETTING_ATTR_EEPROM) == 0 );
    
    settingDB[id].attr &= (~SETTING_ATTR_SET);
}


/* @brief       Get the desired information value. Return default value is not ready
 * @param[in]   id      ID of the entity to be obtained
 * @retval      void*   pointer to the desired object
 */
const void* Setting_GetEx(eSettingId id, const void* pDefault)
{
    ASSERT(id < SETID_MAX);
    
    const void* ret = NULL;
    uint32 addr = getRomAddr(id);

    if (Setting_IsReady(id))
    {
        // if the value is avaliable in RAM
        ret = settingDB[id].p;
    }
    else if (addr)
    {
        // if the value is avaliable in ROM
        ret = (void*)addr;
    }
    else
    {
        ret= pDefault;
    }

    return ret;
}

/* @brief       Get the desired information value
 * @param[in]   id      ID of the entity to be obtained
 * @retval      void*   pointer to the desired object
 */
const void* Setting_GetAddr(eSettingId id)
{
    const void* ret = NULL;

    ret = settingDB[id].p;

    return ret;
}


/* @brief       Get the desired information value
 * @param[in]   id      ID of the entity to be obtained
 * @retval      void*   pointer to the desired object
 */
const void* Setting_Get(eSettingId id)
{
    const void* ret = NULL;
    ret= Setting_GetEx(id, ret);
    ASSERT(ret);
    return ret;
}


/* @brief       Get the size of value
 * @param[in]   id      ID of the entity to be obtained
 * @retval      data size
 */
uint32 Setting_GetSize(eSettingId id)
{
    ASSERT(id < SETID_MAX);
    return (uint32)(settingDB[id].size);
}


/* @brief       Save the information to setting to RAM
 * @param[in]   id      ID of the entity to be set
 * @param[in]   pValue  the pointer to the target value to be set
 */
void Setting_Set(eSettingId id, const void* pValue)
{
    ASSERT(id < SETID_MAX);
    ASSERT(pValue);
    
    if(Setting_IsIdValid(id) && settingDB[id].p)
    {
        if (pValue != settingDB[id].p)
        {
          memcpy(settingDB[id].p, pValue, settingDB[id].size);
        }
        // mark the entity as set
        TYM_SET_BIT(settingDB[id].attr, SETTING_ATTR_SET);

#ifdef SETTING_HAS_ROM_DATA
        // if the saved data is in ROM, trigger the countdown timer for bookkeeping
        if (getRomAddr(id))
        {
            if(pSettingSrv == NULL)
            { /* if pSettingSrv is not assigned, assert it*/
                ASSERT(0);
            }
            else
            {
                save_timeout = SETTING_SAVE_MS;
                PersistantObj_RefreshTick((cPersistantObj*)pSettingSrv, GET_TICKS_IN_MS(TIMER_PERIOD_MS));
            }
        }
#endif
#ifdef ENABLE_SETTING_UPDATE_PUBLISH
        /* When data is set internally, the publish event will always carry with the LAST_SRV_ID */
        SettingSrv_SettUpdateNotification(id, LAST_SRV_ID);
#endif
    }
}

void SettingSrv_FlashReq(QActive* sender, bool bIsSave)
{
    SettingFlashReqEvt* req  = Q_NEW(SettingFlashReqEvt, SETTING_FLASH_REQ_SIG);
    req->sender  = sender;
    req->bIsSave = bIsSave;
    SendToServer(SETTING_SRV_ID, (QEvt*)req);
}

#if defined(ENABLE_SETTING_UPDATE_PUBLISH) || defined(SETTING_HAS_ROM_DATA)
static void SettingSrv_SettUpdateNotification(eSettingId setting_id, uint16 server_id)
{
    // Notification on setting updated
    SettingUpdateEvt* pUpdMsg = Q_NEW(SettingUpdateEvt, SETTING_UPDATE_SIG);
    pUpdMsg->setting_id = setting_id;
    pUpdMsg->server_id = server_id;
    QF_PUBLISH((QEvt*)pUpdMsg, me);
}
#endif

#endif  // HAS_SETTING