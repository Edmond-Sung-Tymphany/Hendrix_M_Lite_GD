/**
*  @file      main_app.c
*  @brief     Main application for BnO FS1 / FS2
*  @author    Daniel Qin
*  @date      17-Dec-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "bl_common.h" /* for bl_getStblStatus */
#include "trace.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "projBsp.h" /* for  ProjBsp_SoftReboot()*/
#include "SettingSrv.h"
#include "PowerDrv.h"
#include "tym_qp_lib.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "MainApp_aseTkEvtHandler.h"
#include "MainApp_keyEvtHandler.h"
#include "MainApp_tickEvtHandler.h"
#include "MainApp.Config"


/*****************************************************************
 * Definition
 *****************************************************************/
#ifdef Q_SPY
#define CAST_ME cMainApp * MainApp = (cMainApp *) me;
#else
#define CAST_ME
#endif

/*****************************************************************
 * Global Variable
 *****************************************************************/
/* Internal evt queue */
static QEvt const *eventQue[MAINAPP_EVENT_Q_SIZE];



/*****************************************************************
 * Function Implemenation
 *****************************************************************/


/*****************************************************************************************************************
 *
 * Startup/Shutdown functions
 *
 *****************************************************************************************************************/
/* Start function*/
void MainApp_StartUp( cPersistantObj *me)
{
     /* start up the object and let it run. including the timer*/
    Application_Ctor((cApplication*)me, Q_STATE_CAST(&MainApp_Initial), MAINAPP_TIMEOUT_SIG,
                            eventQue, Q_DIM(eventQue), MAIN_APP_ID);
}
/* Shut down function*/
void MainApp_ShutDown( cPersistantObj *me)
{
    /* zero all memory that resets an AObject*/
    Application_Xtor((cApplication*)me);
}



/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/*Intial state*/
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    QS_OBJ_DICTIONARY(MainApp);
    QS_OBJ_DICTIONARY(MainApp_Active);

    (void)e; /* suppress the compiler warning about unused parameter */
    /* Subsrcribe to all the SIGS */
    QActive_subscribe((QActive*)me, ASE_TK_STATE_SIG);
    QActive_subscribe((QActive*)me, AUDIO_STATE_SIG);
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);
    QActive_subscribe((QActive*)me, LED_STATE_SIG);
#ifdef BnO_fs1
    QActive_subscribe((QActive*)me, POWER_BATT_STATE_SIG);
    QActive_subscribe((QActive*)me, POWER_WAKE_UP_SIG);
#endif
    QActive_subscribe((QActive*)me, AUDIO_MUSIC_STREAM_STATE_SIG);


    me->sourceHandler = NULL;
    ASSERT(ArraySize(ledIndList) == LED_IND_ID_MAX);
    me->ledInds = ledIndList;
    me->tickHandlers  = (tTickHandlerList *)MainApp_GetTickHandlerList();
    me->combinedKey = COM_KEY_INVALID;
    MainApp_InitStatusVariables(me);
            
    //init power status
    me->powerEvt.dcInStatus= DC_IN_STA_MAX;     
    me->powerEvt.batteryInfo.battStatus= BatteryStatus_LEVEL_HIGH;
    me->powerEvt.batteryInfo.chargerState= CHARGER_STA_CHARGING;
    
    me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_CONN_PRIV;
    me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_PROD_PREV;

    //system sleep status
    bool systemSleep= FALSE;
    Setting_Set(SETID_SYSTEM_SLEEP, &systemSleep);


/* To let production line distinguish debug and release version,
 * debug version have addition "d". For example,
 *   debug: 4.0.0d
 *   release: 4.0.0
 */    
#ifndef NDEBUG
    Setting_Set(SETID_SW_VER, PRODUCT_VERSION_MCU "d"); //debug
#else
    Setting_Set(SETID_SW_VER, PRODUCT_VERSION_MCU); //release
#endif

#ifdef HAS_IWDG
    IwdgInit(IWDG_Prescaler_256, IWDG_RLR_RL);
    IWDG_ReloadCounter();
    RTC_Initialize();
    RTC_SetUpWakeUpAlarm(IWDG_FEED_PERIOD_SEC);
#endif

#ifdef HAS_DYNAMIC_MEMORY_CONTROL
    DynamicAnalysis();
#endif

    return Q_TRAN(&MainApp_PowerInit);
}

/* PowerInit state
 * Purpose: system initialize before power up
 */
QState MainApp_PowerInit(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, LED_BOOTING_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);

            char piu_ver[20];
            char *piu_ver_tmp= bl_readVersion( (void*)FEP_ADDR_PIU_VER );
            snprintf(piu_ver, sizeof(piu_ver), "%s", piu_ver_tmp);

            char ubl_ver[20];
            char *ubl_ver_tmp= bl_readVersion( (void*)FEP_ADDR_UBL_VER );
            snprintf(ubl_ver, sizeof(ubl_ver), "%s", ubl_ver_tmp);
			
            printf("\r\n------------------------------------------------------------------\r\n");
            printf(TP_PRODUCT "\r\n");
            printf("  SW:  PIU:%s  /  UBL:%s  /  SW:%s\r\n", piu_ver, ubl_ver, PRODUCT_VERSION_MCU);
            printf("------------------------------------------------------------------\r\n");

            
#ifdef DEBUG_DEMO_FEATURE
            //Show RED when first time boot
            //MainApp_SendLedReq(me, LED_IND_ID_FIRST_BOOT, /*force:*/FALSE); //stop connective LED
#endif

            //Write version if stored version is wrong
            bl_writeVersion((void*)FEP_ADDR_FIRMWARE_VER, SW_MAJOR_VERSION, SW_MINOR_VERSION1, SW_MINOR_VERSION2, SW_MINOR_VERSION3);
      
            //Start stack usage check and never stop
            me->tickHandlers[TIMER_ID_SYS_HEALTH_CHECK_TIMEOUT].timer = MAINAPP_SYS_HEALTH_CHECK_TIMEOUT_IN_MS;

            /* FS1: 
             *   after upgrade: boot direclty
             *   normal boot: wait first power event, and boot directly (DC mode), or power off (battery mode)
             * FS2:
             *   boot direclty
             */
            eFepFirmwareStatus fw_status = bl_getFirmwareStatus();
            uint32 boot_req= BOOT_REQ_NORMAL;
            boot_req= *(uint32*)Setting_GetEx(SETID_BOOT_REQUEST, /*default:*/&boot_req);
            
            TP_PRINTF("\r\n*** fw_status=%d, boot_req=%d ***\r\n", fw_status, boot_req);
            if(FEP_FIRMWARE_NEW==fw_status)
            {
                TP_PRINTF("\r\n*** After upgrade ==> boot up ***\r\n\r\n\r\n");
                return Q_TRAN(&MainApp_LedBootingUp);
            }
            
            if(BOOT_REQ_POWER_UP==boot_req)
            {
                TP_PRINTF("\r\n*** After crash ==> boot up ***\r\n\r\n\r\n");
                MainApp_WriteBootRequest(me, BOOT_REQ_NORMAL);
                return Q_TRAN(&MainApp_LedBootingUp);
            }
            else if(BOOT_REQ_POWER_DOWN==boot_req)
            {
                TP_PRINTF("\r\n*** After crash on OFF mode ==> power down ***\r\n\r\n\r\n");
                MainApp_WriteBootRequest(me, BOOT_REQ_NORMAL);
                return Q_TRAN(&MainApp_PoweringDown);
            }
            else if(BOOT_REQ_NORMAL==boot_req)
            {
                //do nothing
#ifndef BnO_fs1 //FS2
                TP_PRINTF("\r\n*** Unknown boot request %d ***\r\n\r\n\r\n", boot_req);
                return Q_TRAN(&MainApp_LedBootingUp);
#endif
            }
            else
            {
                TP_PRINTF("\r\n*** Unknown boot request %d ***\r\n\r\n\r\n", boot_req);
                MainApp_WriteBootRequest(me, BOOT_REQ_NORMAL);
            }
            break;
        }
        
#ifdef BnO_fs1
        case POWER_BATT_STATE_SIG:
        {   /* from Power server */
            PowerSrvInfoEvt *evt = (PowerSrvInfoEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG dc:%d, ch:%d, batt-level:%d", 
                e->sig, evt->dcInStatus, evt->batteryInfo.chargerState, evt->batteryInfo.battStatus);
            me->powerEvt= *evt;

            ASSERT(evt->dcInStatus!=DC_IN_STA_MAX);
            if( evt->dcInStatus==DC_IN_STA_ON )
            {
                TP_PRINTF("\r\n\r\n*** First boot, DC=1  ==> boot up ***\r\n\r\n\r\n");
                return Q_TRAN(&MainApp_LedBootingUp);
            }
            else //DC_IN_STA_OFF
            {
                TP_PRINTF("\r\n\r\n*** First boot, DC=0  ==> shutdown ***\r\n\r\n\r\n");
                me->ledPowerDownConnId= LED_IND_ID_FIRST_BOOT;
                me->ledPowerDownProdId= LED_IND_ID_FIRST_BOOT;
                return Q_TRAN(&MainApp_PoweringDown);
            }
        }
#endif

        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            
            //Enalbe repeated timer before boot up
#ifdef HAS_DYNAMIC_MEMORY_CONTROL            
            me->tickHandlers[TIMER_ID_DYNAMIC_ANALYZE_TIMEOUT].timer = MAINAPP_DYNAMIC_ANALYZE_IN_MS;
#endif
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}


/* Base parent state to handle common events
 */
QState MainApp_Base(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        
#ifdef HAS_IWDG
        case TIMER_FEED_WD_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)TIMER_FEED_WD_SIG", e->sig);
            IWDG_ReloadCounter();
            return Q_HANDLED();
        }
#endif

        case MAINAPP_TIMEOUT_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG", e->sig);
            QState trans_state= MainApp_ActiveTickEvtHandler(me, e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);

            if(trans_state!=Q_UNHANDLED())
            {
                return trans_state;
            }
            return Q_HANDLED();
        }

        //For production test
        case MAINAPP_FAST_BOOT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_FAST_BOOT_SIG", e->sig);
            return Q_TRAN(&MainApp_FastBoot);
        }
        
#ifdef BnO_fs1
        case POWER_BATT_STATE_SIG:
        {   /* from Power server */
            PowerSrvInfoEvt *evt = (PowerSrvInfoEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG dc:%d, ch:%d, batt-level:%d", 
                e->sig, evt->dcInStatus, evt->batteryInfo.chargerState, evt->batteryInfo.battStatus);
            me->powerEvt= *evt;

            //Update DSP
            ASSERT(evt->dcInStatus!=DC_IN_STA_MAX);
            MainApp_SetDcStatus(me, (bool)evt->dcInStatus);

            return Q_HANDLED();

            //ASE-TK will shutdown the system
        }
#endif
        
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

QState MainApp_OffCharging(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, OFF_CHARGING_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = MAINAPP_ERROR_UI_CHECK_TIMEOUT_IN_MS;

            if( me->powerEvt.batteryInfo.battStatus==BatteryStatus_LEVEL_HIGH ) // Battert capacity 81%~100%
            {
                MainApp_SendLedReq(me, LED_IND_ID_PROD_OFF, /*force:*/TRUE);
            }
            else  // Battert capacity 7%~80%
            {
                MainApp_SendLedReq(me, LED_IND_ID_BATT_LOW_HAVE_DC, /*force:*/TRUE);
            }

            return Q_HANDLED();
        }
        
#ifdef BnO_fs1
        case POWER_BATT_STATE_SIG:
        {   /* from Power server */
            PowerSrvInfoEvt *evt = (PowerSrvInfoEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG dc:%d, ch:%d, batt-level:%d",
                e->sig, evt->dcInStatus, evt->batteryInfo.chargerState, evt->batteryInfo.battStatus);
            me->powerEvt= *evt;

            //Update DSP
            ASSERT(evt->dcInStatus != DC_IN_STA_MAX);
            MainApp_SetDcStatus(me, (bool)evt->dcInStatus);

            if(!(evt->dcInStatus))
            {
                MainApp_SendLedReq(me, LED_IND_ID_ALL_BG_OFF, /*force:*/TRUE);  //next: LED_STATE_SIG
                MainApp_SendLedReq(me, LED_IND_ID_ALL_FG_OFF, /*force:*/TRUE);
                return Q_TRAN(&MainApp_Off);
            }

            MainApp_UpdateProdLed(me);
            return Q_HANDLED();
        }
#endif
        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG keyEvent=%d, keyId=%d", e->sig, evt->keyEvent, evt->keyId);

            if( (evt->keyId==POWER_KEY     && evt->keyEvent==KEY_EVT_DOWN) ||
                (evt->keyId==TOUCH_TAP_KEY && evt->keyEvent==KEY_EVT_HOLD) )
            {
                TP_PRINTF("\r\n\r\n*** Wakeup by %s key ***\r\n", (evt->keyId==POWER_KEY)?"Power":"Touch" );
                return Q_TRAN(&MainApp_LedBootingUp);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);  
            me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = 0;        
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&MainApp_Base);
}

/* Standby low (OFF) state
 * Purpose: let user think system power off
 */
QState MainApp_Off(cMainApp * const me, QEvt const * const e)
{
    static bool debouncing = FALSE;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SendLedReq(me, LED_IND_ID_ALL_FG_OFF, /*force:*/TRUE);  //next: LED_STATE_SIG
            MainApp_SendLedReq(me, LED_IND_ID_ALL_BG_OFF, /*force:*/TRUE);  //next: LED_STATE_SIG
            MainApp_SwitchMode(me, OFF_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            /* Indicate system is sleep.
             * If all wakeup source debounce fail, will go back to sleep
             */
            bool systemSleep = TRUE;
            Setting_Set(SETID_SYSTEM_SLEEP, &systemSleep);

            /* Notify PowerDrv to trigge sleep
             */
            QEvt* pEvt = Q_NEW(QEvt, POWER_MCU_SLEEP_SIG);
            SendToServer(POWER_SRV_ID, pEvt);
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            return Q_HANDLED();
        }
        case LED_STATE_SIG:
        {
            LedStateEvt* pLedEvt = (LedStateEvt*) e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d) patt=%d", e->sig, pLedEvt->patternId);			
            if(me->ledInds[LED_IND_ID_ALL_BG_OFF].patternId==pLedEvt->patternId)
            {
              //TODO: go to sleep
            }
            return Q_HANDLED();
        }
        case POWER_WAKE_UP_SIG:
        {
            PowerSrvWakeUpEvent* pEvt = (PowerSrvWakeUpEvent*) e;

            if (MAX_WAKE_UP_TYPE == pEvt->powerSrvWakeUpType)
            {
                TP_PRINTF("\r\n\r\n*** Start debounce!! ***\r\n");
                debouncing = TRUE;
                PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_WAKE_UP_SOURCES_DEBOUNCE_MS);
                return Q_HANDLED();
            }
            else
            {
                bool systemSleep = FALSE;
                Setting_Set(SETID_SYSTEM_SLEEP, &systemSleep);

                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_WAKE_UP_SIG, wakeupType=%d", e->sig, pEvt->powerSrvWakeUpType);
                TP_PRINTF("\r\n\r\n*** Wakeup by DC Plug in ***\r\n");
                return Q_TRAN(&MainApp_LedBootingUp);
            }
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            debouncing = FALSE;

            /* Notify PowerDrv to trigge sleep
             */
            QEvt* pEvt = Q_NEW(QEvt, POWER_MCU_SLEEP_SIG);
            SendToServer(POWER_SRV_ID, pEvt);
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG", e->sig);
            return Q_HANDLED();
        }
        
#ifdef HAS_IWDG
        case TIMER_FEED_WD_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)TIMER_FEED_WD_SIG", e->sig);
            IWDG_ReloadCounter();

            //Back to sleep for OFF mode
            if(!debouncing)
            {
                QEvt* pEvt = Q_NEW(QEvt, POWER_MCU_SLEEP_SIG);
                QF_PUBLISH(pEvt, 0);
            }
            return Q_HANDLED();
        }
#endif
        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG keyEvent=%d, keyId=%d", e->sig, evt->keyEvent, evt->keyId);
  
            if( (evt->keyId==POWER_KEY     && evt->keyEvent==KEY_EVT_DOWN) ||
                (evt->keyId==TOUCH_TAP_KEY && evt->keyEvent==KEY_EVT_HOLD) )
            {
                TP_PRINTF("\r\n\r\n*** Wakeup by %s key ***\r\n", (evt->keyId==POWER_KEY)?"Power":"Touch" );
                bool systemSleep= FALSE;
                Setting_Set(SETID_SYSTEM_SLEEP, &systemSleep);                    
                return Q_TRAN(&MainApp_LedBootingUp);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            debouncing = FALSE;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);
}



/* Powering Down state
 * Purpose: show LED
 */
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            me->tickHandlers[TIMER_ID_POWER_DOWN_TIMEOUT].timer = MAINAPP_POWER_DOWN_TIMEOUT_IN_MS;
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( POWERING_DOWN_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else {
                TP_PRINTF("MainApp_PoweringDown: (1)send power down LED\r\n");
                
                //Change audio mode to NORMAL, to enable LED change
                me->audioMode= AUDIO_MODE_NORMAL;

                //Update LED
                MainApp_SendLedReq(me, me->ledPowerDownConnId, /*force:*/TRUE);
                MainApp_SendLedReq(me, me->ledPowerDownProdId, /*force:*/TRUE);
                MainApp_SendLedReq(me, LED_IND_ID_ALL_BG_OFF, /*force:*/TRUE);  //next: LED_STATE_SIG
                
                //Enable fast charging on OFF mode
                PowerSrv_Set((QActive *)me, POWER_SET_ID_SLOW_CHARGER, /*slow_ch:*/FALSE);  
            }
            return Q_HANDLED();
        }
        case LED_STATE_SIG:
        {
            LedStateEvt* pLedEvt = (LedStateEvt*) e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)LED_STATE_SIG patt=%d", e->sig, pLedEvt->patternId);
            ePattern replyPattConn= MainApp_GetReplyLedPattern(me, me->ledPowerDownConnId);
            ePattern replyPattProd= MainApp_GetReplyLedPattern(me, me->ledPowerDownProdId);
            if( pLedEvt->patternId==replyPattConn || pLedEvt->patternId==replyPattProd )
            {
                //Set back to initial value
                me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_CONN_PRIV;
                me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_CONN_PRIV;
#ifdef BnO_fs1
                if(me->powerEvt.dcInStatus)
                {
                    TP_PRINTF("MainApp_PoweringDown: (2)get LED_STATE_SIG, goto MainApp_OffCharging()\r\n");
                    return Q_TRAN(&MainApp_OffCharging);
                }
                else
#endif
                {
                    TP_PRINTF("MainApp_PoweringDown: (2)get LED_STATE_SIG, goto MainApp_Off()\r\n");
                    return Q_TRAN(&MainApp_Off);
                }
            }
            return Q_HANDLED();
        }
        
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->tickHandlers[TIMER_ID_POWER_DOWN_TIMEOUT].timer = 0; //disable timer
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);
}


/* Powering Down state
 * Purpose: show LED
 */
#ifdef BnO_fs1 
QState MainApp_Storage(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( POWERING_DOWN_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else {
                TP_PRINTF("MainApp_Storage: (1)send FG_WHITE_QUICK_FLASH_ONCE_PAT\r\n");
                //Change audio mode to NORMAL, to enable LED change
                me->audioMode= AUDIO_MODE_NORMAL;
                
                //Update LED
                MainApp_SendLedReq(me, LED_IND_ID_ALL_BG_OFF, /*force:*/TRUE);
                MainApp_SendLedReq(me, LED_IND_ID_ENTER_STORAGE, /*force:*/TRUE);
            }
            return Q_HANDLED();
        }
        case LED_STATE_SIG:
        {
            LedStateEvt* pLedEvt = (LedStateEvt*) e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)LED_STATE_SIG patt=%d", e->sig, pLedEvt->patternId);
            if( pLedEvt->patternId==ledIndList[LED_IND_ID_ENTER_STORAGE].patternId )
            {
                TP_PRINTF("MainApp_Storage: (2)get LED_STATE_SIG, shutdown battery\r\n");
                PowerSrv_Set((QActive *)me, POWER_SET_ID_FUEL_GAUGE_SHUT_DOWN, /*unused*/0);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);
}
#endif /* #ifdef BnO_fs1  */



/* Wait Mode 
 * Purpose: do not show key LED, and wait ASE-TK reboot
 */
QState MainApp_Wait(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, WAIT_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
            AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, TRUE, /* NO USED*/0, /* NO USED*/0); //wakeup
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( WAIT_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = MAINAPP_ERROR_UI_CHECK_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = MAINAPP_ERROR_SAFETY_CHECK_TIMEOUT_IN_MS;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = 0;
            me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = 0; 
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);
}


/* Wait Upgrade Mode 
 * Purpose: do not show key LED, and wait ASE-TK reboot
 */
QState MainApp_WaitUpgrade(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = MAINAPP_UPGRADE_TIMEOUT_IN_MS;
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            AseTkStateIndEvt* aseTkEvt = (AseTkStateIndEvt*) e;
            if( aseTkEvt->aseFepCmd.which_OneOf==AseFepMessage_aseFepEvent_tag )
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG, which_OneOf=%d, event=%d", e->sig, aseTkEvt->aseFepCmd.which_OneOf, aseTkEvt->aseFepCmd.OneOf.aseFepEvent.event);
                //It seems ASE-TK do not send UPDATE_FAILED or UPDATE_FINISHED , we must check BOOTED
                if( aseTkEvt->aseFepCmd.OneOf.aseFepEvent.event==AseFepEvent_Event_SW_UPDATE_FINISHED ||
                    aseTkEvt->aseFepCmd.OneOf.aseFepEvent.event==AseFepEvent_Event_SW_UPDATE_FAILED ||
                    aseTkEvt->aseFepCmd.OneOf.aseFepEvent.event==AseFepEvent_Event_ASE_BOOTED )
                {
                    return Q_TRAN(&MainApp_Active);
                }
            }
            else
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG, which_OneOf=%d", e->sig, aseTkEvt->aseFepCmd.which_OneOf);
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = 0; //disable timer
            break;
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Wait);
}



/* Wait Factory Reset Mode 
 * Purpose: do not show key LED, and wait ASE-TK reboot
 */
QState MainApp_WaitFactoryReset(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            me->tickHandlers[TIMER_ID_FACTORY_RESET_TIMEOUT].timer = MAINAPP_FACTORY_RESET_TIMEOUT_MS;
            
            //Change audio mode to NORMAL, to enable LED change
            me->audioMode= AUDIO_MODE_NORMAL;
            MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP, /*force:*/FALSE);
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            AseTkStateIndEvt* aseTkEvt = (AseTkStateIndEvt*) e;
            if( aseTkEvt->aseFepCmd.which_OneOf==AseFepMessage_aseFepEvent_tag )
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG, which_OneOf=%d, event=%d", e->sig, aseTkEvt->aseFepCmd.which_OneOf, aseTkEvt->aseFepCmd.OneOf.aseFepEvent.event);
                //BOOTED is not necessary, we always receive FACTORY_RESET_DONE after factory reset
                if( aseTkEvt->aseFepCmd.OneOf.aseFepEvent.event==AseFepEvent_Event_FACTORY_RESET_DONE ||
                    aseTkEvt->aseFepCmd.OneOf.aseFepEvent.event==AseFepEvent_Event_ASE_BOOTED ) 
                {
                    return Q_TRAN(&MainApp_Active);
                }
            }
            else
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG, which_OneOf=%d", e->sig, aseTkEvt->aseFepCmd.which_OneOf);
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_FACTORY_RESET_TIMEOUT].timer = 0; //disable timer
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Wait);
}


/* Ase Tk booting up state 
 * Purpose: wait ASE-TK boot up
 */
QState MainApp_AseTkBootingUp(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, ASE_TK_BOOTING_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( ASE_TK_BOOTING_MODE != evt->modeId ) {
                //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else
            {
                me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer = MAINAPP_ASE_TK_BOOTING_TIMEOUT_IN_MS;
                //me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = MAINAPP_ERROR_UI_CHECK_TIMEOUT_IN_MS; 
                me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = MAINAPP_ERROR_SAFETY_CHECK_TIMEOUT_IN_MS;
                eFepFirmwareStatus fw_status = bl_getFirmwareStatus();
                if(FEP_FIRMWARE_NEW == fw_status)
                {
                    bl_setFirmwareStatus(FEP_FIRMWARE_NORMAL);  // avoid skipping wait for ASE-TK boot-up for next power cycle.
                    return Q_TRAN(&MainApp_Active); // go to active state directly after upgraded firmware.
                }
            }            
            return Q_HANDLED();
        }
#ifdef BnO_fs1
        case POWER_BATT_STATE_SIG:
        {   /* from Power server */
            PowerSrvInfoEvt *evt = (PowerSrvInfoEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG dc:%d, ch:%d, batt-level:%d",
                e->sig, evt->dcInStatus, evt->batteryInfo.chargerState, evt->batteryInfo.battStatus);
            me->powerEvt= *evt;

            //Update DSP
            ASSERT(evt->dcInStatus!=DC_IN_STA_MAX);
            MainApp_SetDcStatus(me, (bool)evt->dcInStatus);

            //Shutdown
            if( !(evt->dcInStatus) && (evt->batteryInfo.battStatus==BatteryStatus_LEVEL_CRITICAL
                 && !(*(bool*)Setting_Get(SETID_SYSTEM_SLEEP))) ) // Battert capacity 0%~2%
            {
                //Shutdown for low capacity battery
                uint16 batt_rsoc= 0;
                uint16 batt_rsoc_user= 0;
                uint16 batt_asoc= 0;
                batt_rsoc=      *(uint16*)Setting_GetEx(SETID_BATTERY_CAPACITY_RSOC, &batt_rsoc);
                batt_rsoc_user= *(uint16*)Setting_GetEx(SETID_BATTERY_CAPACITY_RSOC_USER, &batt_rsoc_user);
                batt_asoc=      *(uint16*)Setting_GetEx(SETID_BATTERY_CAPACITY_ASOC, &batt_asoc);
                TP_PRINTF("\r\n\r\n*** MCU low battery shutdown (RSOC=%d%%, RSOC_User=%d%%, ASOC=%d%%) ***\r\n\r\n\r\n", batt_rsoc, batt_rsoc_user, batt_asoc);
                me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_ERROR;
                me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_ERROR;
                return Q_TRAN(&MainApp_PoweringDown);
            }
            return Q_HANDLED();
        }
#endif

        case ASE_TK_STATE_SIG:
        {
            //Boot up when ASE-TK is ready
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            AseTkStateIndEvt* aseTkEvt = (AseTkStateIndEvt*) e;
            if( aseTkEvt->aseFepCmd.which_OneOf==AseFepMessage_aseFepEvent_tag &&
                aseTkEvt->aseFepCmd.OneOf.aseFepEvent.event==AseFepEvent_Event_ASE_BOOTED )
            {
                TP_PRINTF("AseFepEvent_Event_ASE_BOOTED\r\n");
                MainApp_PrintVersion(me);
                return Q_TRAN(&MainApp_Active);
            }
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
  
            if(POWER_KEY==(evt->keyId))
            {
                if(KEY_EVT_DOWN == evt->keyEvent)
                {
                    // 1st step
                    TYM_SET_BIT(me->combinedKey, POWER_IN_HOLD);
                }
                else if(KEY_EVT_SHORT_PRESS==evt->keyEvent && me->combinedKey==POWER_IN_HOLD)
                {   // 3rd step
                    me->combinedKey = COM_KEY_INVALID;
                    MainApp_SendLedReq(me, LED_IND_ID_KEY_SP, /*force:*/FALSE); 
                    TP_PRINTF("\r\n\r\n*** Power off when MainApp_AseTkBootingUp ***\r\n");
                    me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_CONN_PRIV;
                    me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_PROD_WHITE;
                    return Q_TRAN(&MainApp_PoweringDown);
                }
#ifdef BnO_fs1
                else if(KEY_EVT_VERY_LONG_HOLD == evt->keyEvent)
                {
                    bool dcStatus= TRUE;
                    dcStatus= *(bool*)Setting_GetEx(SETID_IS_DC_PLUG_IN, /*default:*/&dcStatus);
                    if(dcStatus)
                    {
                        TP_PRINTF("\r\n\r\n*** AseTkBootingUp: Very Long HOLD power key, dc=0 ==> do nothing ***\r\n"); 
                    }
                    else
                    {
                        TP_PRINTF("\r\n\r\n*** AseTkBootingUp: Very Long HOLD power key, dc=1 ==> shutdown battery ***\r\n"); 
                        return Q_TRAN(&MainApp_Storage);
                    }
                }
#endif
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer = 0;
            //me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = 0; 
            me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = 0;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);
}


/* Because DSP initialize need near 1 seconds, too let user see LED ealier,
 * we initialize LED here before DSP
 */
QState MainApp_LedBootingUp(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, LED_BOOTING_MODE);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);

            //Clear error, to check error again after boot up
            uint32 errorReason= 0;
            Setting_Set(SETID_ERROR_REASON, &errorReason);   

            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( LED_BOOTING_MODE != evt->modeId ) {
                //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else
            {
                if( SET==FLASH_OB_GetRDP() )
                {
                    /* On FS1 EVT, production line enable flash protection and cause DFU fail.
                     * To avoid issue occur again, we check here, and flash RED when flash protection enable
                     * debug build: ASSERT()
                     * release build: repeated flash RED led
                     */         
                    TP_PRINTF("\r\n\r\n\r\n*** ERROR: Flash read prtection is enabled, it will cause firmware upgrade fail ***\r\n\r\n\r\n");
                    ASSERT(0);
                    MainApp_SendLedReq(me, LED_IND_ID_ERROR, /*force:*/TRUE);
                }
                else
                {
                    MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP, /*force:*/FALSE);
                    //me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = MAINAPP_ERROR_UI_CHECK_TIMEOUT_IN_MS; 
                    me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = MAINAPP_ERROR_SAFETY_CHECK_TIMEOUT_IN_MS;
                    return Q_TRAN(&MainApp_AseTkBootingUp);
                }                
            }            
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            //me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = 0; 
            me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = 0;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);
}


/* Active state
 * Purpose: normal state, ASE-TK should work now
 */
QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_InitStatusVariables(me);
            MainApp_SwitchMode(me, NORMAL_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( NORMAL_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else
            {
#ifdef BnO_fs1
                /* For FS1, the first PowerStatus event is handled by PowerInit Status,
                 * but on that moment, Audio driver is not ready and can not set DC stauts.
                 * Thus we set DC status to DSP when ASE-TK is ready.
                 *
                 * When after upgrade, FEP go to Active status very soon and DC status may not ready,
                 * thus we set DC status only if DC is ready
                 */                
                if ( me->powerEvt.dcInStatus!=DC_IN_STA_MAX )
                {
                    MainApp_SetDcStatus(me, (bool)me->powerEvt.dcInStatus);
                }
#endif                

                MainApp_SendLedReq(me, LED_IND_ID_ETHERNET_MODE_IDLE, /*force:*/FALSE);            
                MainApp_UpdateProdLed(me);
                MainApp_SwitchAudioSource(me, AUDIO_SOURCE_ASETK);
                me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = MAINAPP_ERROR_UI_CHECK_TIMEOUT_IN_MS;               
                me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = MAINAPP_ERROR_SAFETY_CHECK_TIMEOUT_IN_MS;
                me->tickHandlers[TIMER_ID_VOLUME_CHANGE_TIMEOUT].timer = MAINAPP_VOLUME_CHANGE_TIMEOUT_IN_MS;
                me->tickHandlers[TIMER_ID_POWER_STATUS_UPDATE_TIMEOUT].timer = MAINAPP_POWER_STATUS_UPDATE_TIMEOUT_IN_MS;
            }
            return Q_HANDLED();
        }
        
        //for production line test
        case MAINAPP_SET_AUDIO_MODE_SIG:
        {
            MainAppSetAudioEvt* evt = (MainAppSetAudioEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_SET_AUDIO_MODE_SIG: audio_mode=%d", e->sig, evt->audio_mode);
            MainApp_SetAudioMode(me, (eAudioMode)evt->audio_mode);
            return Q_HANDLED();
        }
        
        case AUDIO_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)AUDIO_STATE_SIG", e->sig);
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            QState trans_state= MainApp_KeyHandler(me,e);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            QState trans_state= MainApp_AseTkEvtHandler(me,e);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            return Q_HANDLED();
        }

        case POWER_WAKE_UP_SIG:
        {
            TP_PRINTF("\r\n\r\n\r\nPOWER_WAKE_UP_SIG\r\n\r\n\r\n");
            return Q_HANDLED();
        }

        case POWER_BATT_STATE_SIG:
        {
#ifdef BnO_fs1
            PowerSrvInfoEvt *evt = (PowerSrvInfoEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG dc:%d, ch:%d, batt-level:%d", 
                e->sig, evt->dcInStatus, evt->batteryInfo.chargerState, evt->batteryInfo.battStatus);
            me->powerEvt= *evt;

            ASSERT(evt->dcInStatus!=DC_IN_STA_MAX);
            MainApp_SetDcStatus(me, evt->dcInStatus);

            /* Let AE-TK decide power off on Active mode
             */
            //if( !(evt->dcInStatus) && evt->batteryInfo.battStatus==BatteryStatus_LEVEL_CRITICAL ) // Battert capacity 0%~2%
            //{
            //    //TODO: report battery status to ASE-TK, and wait for OFF
            //    //Shutdown for low capacity battery
            //    TP_PRINTF("\r\n\r\n*** Low battery shutdown ***\r\n\r\n\r\n");
            //    me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_ERROR;
            //    return Q_TRAN(&MainApp_PoweringDown);
            //}
            MainApp_UpdateProdLed(me);
#endif
            return Q_HANDLED();
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            AudioMusicDetectStateEvt* evt = (AudioMusicDetectStateEvt*)e;
            MainApp_MusicDetectSignalHandler(me, evt->hasMusicStream);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            
            //disable timer
            me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = 0; 
            me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = 0;
            me->tickHandlers[TIMER_ID_ENTER_STORAGE_MODE_TIMEOUT].timer = 0;
            me->tickHandlers[TIMER_ID_VOLUME_CHANGE_TIMEOUT].timer = 0;
            me->tickHandlers[TIMER_ID_POWER_STATUS_UPDATE_TIMEOUT].timer = 0;            
            return Q_HANDLED();
        }
        default:
            break;
    }
    
    return Q_SUPER(&MainApp_Base);
}

QState MainApp_FastBoot(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, FAST_BOOT_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( FAST_BOOT_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else
            {              
                //Change audio mode to NORMAL, to enable LED change
                me->audioMode= AUDIO_MODE_NORMAL;

                MainApp_SendLedReq(me, LED_IND_ID_FAST_BOOT, /*force:*/FALSE);
                MainApp_AudioShutdown(me, FALSE);
                AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, /*mute:*/FALSE);
                MainApp_SwitchAudioSource(me, AUDIO_SOURCE_EXT_SOURCE); //SPDIF
                AudioSrv_SetVolume(DEFAULT_VOLUME);
                
#ifdef BnO_fs1
                /* For FS1, the first PowerStatus event is handled by PowerInit Status,
                 * but on that moment, Audio driver is not ready and can not set DC stauts.
                 * Thus we set DC status to DSP when ASE-TK is ready.
                 *
                 * When after upgrade, FEP go to Active status very soon and DC status may not ready,
                 * thus we set DC status only if DC is ready
                 */                
                if ( me->powerEvt.dcInStatus!=DC_IN_STA_MAX )
                {
                    MainApp_SetDcStatus(me, (bool)me->powerEvt.dcInStatus);
                }
#endif
                me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = MAINAPP_ERROR_UI_CHECK_TIMEOUT_IN_MS; 
                me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = MAINAPP_ERROR_SAFETY_CHECK_TIMEOUT_IN_MS;
            }
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG should be igored in MainApp_FastBoot mode", e->sig);
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
  
            if(POWER_KEY==(evt->keyId))
            {
                if(KEY_EVT_DOWN == evt->keyEvent)
                {
                    // 1st step
                    TYM_SET_BIT(me->combinedKey, POWER_IN_HOLD);
                }
                else if(KEY_EVT_UP==evt->keyEvent && me->combinedKey==POWER_IN_HOLD)
                {   // 3rd step
                    me->combinedKey = COM_KEY_INVALID;
                    MainApp_SendLedReq(me, LED_IND_ID_KEY_SP, /*force:*/FALSE); 
                    TP_PRINTF("\r\n\r\n*** Power off when MainApp_FastBoot ***\r\n");
                    me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_CONN_PRIV;
                    me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_PROD_WHITE;
                    return Q_TRAN(&MainApp_PoweringDown);
                }
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = 0; 
            me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = 0;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);
}


