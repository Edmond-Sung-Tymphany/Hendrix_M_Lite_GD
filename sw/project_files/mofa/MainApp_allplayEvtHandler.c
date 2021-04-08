/**
*  @file      MainApp_allplayEvtHandler.c
*  @brief     allplay event handler of mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "trace.h"
#include "AllPlaySrv.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "tym_qp_lib.h"
#include "MainApp_priv.h"

/*****************************************************************
 * Global Variable
 *****************************************************************/
extern tPatternData patternConfig[PAT_MAX_NUMBER];

static void MainApp_AllPlayVolChanged(cMainApp * const me, QEvt const * const e);
/*****************************************************************
 * Function Implemenation
 *****************************************************************/

void MainApp_DisplayAllplaySystemMode(cMainApp * const me, enum allplay_system_mode_value currentSystemMode)
{
    MAINAPP_DEBUG_MSG("[%s] mode %d\r\n", __func__, currentSystemMode);
    switch(currentSystemMode)
    {
        case ALLPLAY_SYSTEM_MODE_CONFIGURED:
        {
            AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
            if(allPlayInfo->networkType == ALLPLAY_NETWORK_WIFI)
            {
                patternConfig[SOLID_PAT].color = me->sourceHandler[ALLPLAY_AP_MODE].sourceColor;
                MainApp_SendLedReq(me, SOLID_PAT);
            }
            else
            {
                MainApp_SendLedReq(me, NET_CONNECTING_PAT);
            }
            break;
        }
        case ALLPLAY_SYSTEM_MODE_CONFIGURING:
            //do nothing here
            break;
#ifdef HAS_ALLPLAY_DIRECTMODE
        case ALLPLAY_SYSTEM_MODE_DIRECT:
            patternConfig[SOLID_PAT].color = me->sourceHandler[ALLPLAY_DIRECT_MODE].sourceColor;
            MainApp_SendLedReq(me, SOLID_PAT);
            break;
#endif
        case ALLPLAY_SYSTEM_MODE_UNCONFIGURED:
            MainApp_SendLedReq(me, NET_CONNECTING_PAT);
            break;
        case ALLPLAY_SYSTEM_MODE_UPDATING:
            MainApp_SendLedReq(me, DFU_PROG_PAT);
            break;
        default:
            ASSERT(0);
            break;
    }
}

static void MainApp_AllPlayVolChanged(cMainApp * const me, QEvt const * const e)
{
    AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;
    uint8   vol         = *(uint8*)Setting_Get(SETID_VOLUME);
    int     volCurr     = allPlayEvt->payload.volChangeInfo.volume;
    uint16  allplayVol  = MainApp_mcuVol2AllplayVol(vol);

    me->allplayMaxVol = allPlayEvt->payload.volChangeInfo.max_volume;
    if(allplayVol != volCurr)
    {
        uint8 vol_temp  = MainApp_allplayVol2McuVol(volCurr);
        if((volCurr > allplayVol) && (vol_temp <= vol))
        {/* There is a calculation error cause vol_temp <= vol in this case
          * So a margin of error should be added here.
          */
            vol_temp = vol_temp + MainApp_allplayVol2McuVol_errorMargin(volCurr-allplayVol);
        }
        AudioSrv_SetVolume(vol_temp);
    }
}

void MainApp_BTEvtHandler(cMainApp * const me, QEvt const * const e)
{
    AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;

    switch(allPlayEvt->allPlayState)
    {
        case ALLPLAY_STATE_BT_DISABLE:
        {
            break;
        }
        case ALLPLAY_STATE_BT_PAIRING_DISABLE:
        {
            if(BLUETOOTH == me->audioSource && me->tickHandlers[BT_PAIRING_TIMER].timer > 0)
            {
                AllPlaySrv_BluetoothPairable(TRUE);
            }
            break;
        }
        case ALLPLAY_STATE_BT_PAIRING_ENABLE:
        {
            if(BLUETOOTH == me->audioSource)
            {
                MainApp_SendLedReq(me, BT_PAIRING_ENABLE_PAT);
            }
            break;
        }
        case ALLPLAY_STATE_BT_DEVICE_CONNECTED:
        {
            me->tickHandlers[BT_PAIRING_TIMER].timer = 0;
            me->tickHandlers[BT_RECONNECT_TIMER].timer = 0;
            patternConfig[SOLID_PAT].color = me->sourceHandler[BLUETOOTH].sourceColor;
            MainApp_SendLedReq(me, SOLID_PAT);
            /* Connect to speaker using paired device shall allow BT to become active audio source. */
            if(me->audioSource != BLUETOOTH)
            {
                me->audioSource = BLUETOOTH;
                me->nextState = (QStateHandler*)&MainApp_SourceSwitching;
            }
            break;
        }
        case ALLPLAY_STATE_BT_DEVICE_DISCONNECTED:
        {
            if(me->audioSource == BLUETOOTH)
            {
                MainApp_SendLedReq(me, BT_RECONNECTINT_PAT);
                me->tickHandlers[BT_RECONNECT_TIMER].timer = MAINAPP_BT_RECONNECT_TIMEOUT_IN_MS;
            }
            break;
        }
        default:
            break;
    }
}

void MainApp_AllPlayStateHandler(cMainApp * const me, QEvt const * const e)
{
    AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;
    switch(allPlayEvt->allPlayState)
    {
        /*if necessary, add the handler for more allPlayState here */
        case ALLPLAY_STATE_NETWORK_INFO_CHANGED:
        {
            if(me->audioSource == ALLPLAY_AP_MODE)
            {
                AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
                if((allPlayEvt->payload.networkType == ALLPLAY_NETWORK_WIFI) &&
                (ALLPLAY_SYSTEM_MODE_CONFIGURED == allPlayInfo->eSystemMode))
                {
                    patternConfig[SOLID_PAT].color = GREEN;
                    MainApp_SendLedReq(me, SOLID_PAT);
                }
                else
                {
                    MainApp_SendLedReq(me, NET_CONNECTING_PAT);
                }
            }
            break;
        }
        case ALLPLAY_STATE_MCU_SAM_CONNECTED:
            if(me->audioSource == ALLPLAY_AP_MODE)
            {
                AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
                MainApp_DisplayAllplaySystemMode(me, allPlayInfo->eSystemMode);
            }
            break;
        case ALLPLAY_STATE_MCU_SAM_DISCONNECTED:
            break;
        case ALLPLAY_STATE_SYSTEM_MODE_CHANGED:
            if(me->audioSource == ALLPLAY_AP_MODE)
            {
                MainApp_DisplayAllplaySystemMode(me, allPlayEvt->payload.currentSystemMode);
            }
            /* if SAM is in updating mode, mainApp should switch to upgrading state. */
            if(ALLPLAY_SYSTEM_MODE_UPDATING ==allPlayEvt->payload.currentSystemMode)
            {
                MainApp_DisplayAllplaySystemMode(me, ALLPLAY_SYSTEM_MODE_UPDATING);
                me->nextState = (QStateHandler*)&MainApp_Upgrading;
            }
            break;
        case ALLPLAY_STATE_VOLUME_INFO:
        {   /* re-sync sam's volume to mcu's volume */
            uint8   vol         = *(uint8*)Setting_Get(SETID_VOLUME);
            int     volCurr     = allPlayEvt->payload.volChangeInfo.volume;
            uint16  allplayVol  = MainApp_mcuVol2AllplayVol(vol);

            me->allplayMaxVol = allPlayEvt->payload.volChangeInfo.max_volume;
            if(allplayVol != volCurr)
            {
                AllPlaySrv_SetVol(allplayVol);
            }
            break;
        }
        case ALLPLAY_STATE_VOLUME_CHANGED:
        {
            /*The volume should be able to change by phone App in all source modes. */
            MainApp_AllPlayVolChanged(me, e);
            break;
        }
        case ALLPLAY_STATE_SAM_FACTORY_RESET_DONE:
        {
            NvmDrv_EraseAll(NULL);
            NVM_STORAGE_VALUE_SET(NVM_STORAGE_ADDR_IGNORE_PWR_KEY);
            me->nextState = (QStateHandler*)&MainApp_SoftReset;
            break;
        }
        case ALLPLAY_STATE_PLAYER_PLAYING:
        {
            /* AllPlay audio source shall able to hijack other audio sources. */
            AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
            if(me->audioSource != ALLPLAY_AP_MODE
#ifdef HAS_ALLPLAY_DIRECTMODE
            && me->audioSource != ALLPLAY_DIRECT_MODE
#endif
            && ALLPLAY_AP == allPlayInfo->contentSource)
            {
                me->audioSource = ALLPLAY_AP_MODE;
                me->nextState = (QStateHandler*)&MainApp_SourceSwitching;
            }
            break;
        }
        default:
            break;
    }
}
