/**
*  @file      main_app.c
*  @brief     Main application for BnO CA16
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
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);
    QActive_subscribe((QActive*)me, LED_STATE_SIG);
    QActive_subscribe((QActive*)me, AUDIO_MUSIC_STREAM_STATE_SIG);

    ASSERT(ArraySize(sourceHandlerList) == AUDIO_SOURCE_MAX);
    me->sourceHandler = sourceHandlerList;
    ASSERT(ArraySize(ledIndList) == LED_IND_ID_MAX);
    me->ledInds = ledIndList;
    me->tickHandlers  = (tTickHandlerList *)MainApp_GetTickHandlerList();
    me->combinedKey = COM_KEY_INVALID;
    Setting_Set(SETID_SW_VER, PRODUCT_VERSION_MCU);
    me->audioSource = AUDIO_SOURCE_MAX;
    Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
    me->systemStatus = SYSTEM_STA_STANDBY_LOW;
    me->aseBtleEnable = FALSE;
    me->absoluteVol = INVALID_VALUE;
    me->isVolChanged = FALSE;
    me->isTonePlaying = FALSE;

    IwdgInit(IWDG_Prescaler_256, IWDG_RLR_RL);
    IwdgReloadActiveMode();

    RTC_Initialize();
    RTC_SetUpWakeUpAlarm(IWDG_FEED_PERIOD_SEC);

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
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            
            //Write version if stored version is wrong
            bl_writeVersion((void*)FEP_ADDR_FIRMWARE_VER, SW_MAJOR_VERSION, SW_MINOR_VERSION1, SW_MINOR_VERSION2, SW_MINOR_VERSION3);

            char * pHwVer = (char *)Setting_Get(SETID_HW_VER);
            TP_PRINTF("\r\n-------------------------------------------------------\r\n");
            TP_PRINTF(TP_PRODUCT "HW ver: %s SW ver: v%s, built on %s %s\r\n", pHwVer, PRODUCT_VERSION_MCU, __DATE__, __TIME__);
            TP_PRINTF("-------------------------------------------------------\r\n");
            if (PowerDrv_IsHwVerCorrect(pHwVer) == FALSE)
            {
                printf("hwVersion(%s) is NOT correct, it should be %s \r\n", pHwVer, HARDWARE_VERSION_STRING);
#ifdef NDEBUG
                printf("Note: as HW version is not expected, so system can not power on in release SW build.  \r\n");
#else
                TP_PRINTF("Note: as HW version is not expected, so system will power on 5 seconds later. \r\n");
                PersistantObj_RefreshTick((cPersistantObj*)me, 5000); // LED indicate HW error for 5 seconds then power on system.
#endif

                MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);
                return Q_HANDLED();
            }
            else
            {
                MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP);
#ifdef HAS_ASE_TK
                return Q_TRAN(&MainApp_AseTkBootingUp);
#else
                return Q_TRAN(&MainApp_Active); 
#endif
            }
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP);
#ifdef HAS_ASE_TK
            return Q_TRAN(&MainApp_AseTkBootingUp);
#else
            return Q_TRAN(&MainApp_Active); 
#endif
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            break;
        }
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

/* Standby low (OFF) state
 * Purpose: let user think system power off
 */
QState MainApp_Off(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, OFF_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            me->audioSource = AUDIO_SOURCE_MAX;
            Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG keyEvent=%d, keyId=%d", e->sig, evt->keyEvent, evt->keyId);
  
            if(STANDBY_KEY == (evt->keyId) || POWER_IR_KEY == (evt->keyId))
            {
                if(KEY_EVT_DOWN == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, POWER_IN_HOLD);
                }
                else if(KEY_EVT_HOLD == (evt->keyEvent) || KEY_EVT_SHORT_PRESS == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, POWER_IN_L_HOLD);
                }
                if(me->combinedKey == COM_KEY_POWER_ON)
                {
                    me->combinedKey = COM_KEY_INVALID;
                    return Q_TRAN(&MainApp_PowerInit);
                }
            }
            return Q_HANDLED();
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadOffMode();
            QEvt* pEvt = Q_NEW(QEvt, POWER_MCU_SLEEP_SIG);
            SendToServer(POWER_SRV_ID, pEvt);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->combinedKey = COM_KEY_INVALID;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
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
            me->tickHandlers[TIMER_ID_POWER_DOWN_TIMEOUT].timer = MAINAPP_POWER_DOWN_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, 1000);
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);
            MainApp_SendLedReq(me, LED_IND_ID_ALL_TRANS_OFF);
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
                //MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
            }
            return Q_HANDLED();
        }
        case LED_STATE_SIG:
        {
            LedStateEvt *pe = (LedStateEvt *)e;
            if(pe->patternId == TRANS_OFF_PAT_1)
            {
                MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                return Q_TRAN(&MainApp_Off);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {   /* system should never come to here in normal cases. */
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QState trans_state = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_POWER_DOWN_TIMEOUT].timer = 0;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}


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
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = MAINAPP_UPGRADE_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            /* In WAIT_MODE, key server will be deactived. */
            MainApp_SwitchMode(me, WAIT_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( WAIT_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG", e->sig);
            QState trans_state = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }

        }
        case SAFETY_ALARM_OVERTEMPERATURE:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SAFETY_ALARM_OVERTEMPERATURE", e->sig);
            return Q_HANDLED();
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = 0;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
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
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = MAINAPP_UPGRADE_TIMEOUT_IN_MS;
            break;
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
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = 0;
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
            me->tickHandlers[TIMER_ID_FACTORY_RESET_TIMEOUT].timer = MAINAPP_FACTORY_RESET_TIMEOUT_MS;
            MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP);
            break;
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
            me->tickHandlers[TIMER_ID_FACTORY_RESET_TIMEOUT].timer = 0;
            break;
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Wait);
}

/* Soft Reset state*/
QState MainApp_SoftReset(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, OFF_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RES", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( OFF_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else {
                BSP_SoftReboot();
            }
            return Q_HANDLED();
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
        }
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
            me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer = MAINAPP_ASE_TK_BOOTING_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, 1000);
            //me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer = 23*1000;
            //PersistantObj_RefreshTick((cPersistantObj*)me, 23*1000);
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
                eFepFirmwareStatus fw_status = bl_getFirmwareStatus();
                if(FEP_FIRMWARE_NEW == fw_status)
                {
                    bl_setFirmwareStatus(FEP_FIRMWARE_NORMAL);  // avoid skipping wait for ASE-TK boot-up for next power cycle.
                    return Q_TRAN(&MainApp_Active); // go to active state directly after upgraded firmware.
                }
                else
                {
                    //MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP);
                }
            }
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            //Boot up when ASE-TK is ready
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            AseTkStateIndEvt* aseTkEvt = (AseTkStateIndEvt*) e;
            if( aseTkEvt->aseFepCmd.which_OneOf==AseFepMessage_aseFepEvent_tag &&
                aseTkEvt->aseFepCmd.OneOf.aseFepEvent.event==AseFepEvent_Event_ASE_BOOTED )
            {
                return Q_TRAN(&MainApp_Active);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            QState trans_state = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }

        }
        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if(STANDBY_KEY == (evt->keyId) || POWER_IR_KEY == (evt->keyId))
            {
                if(KEY_EVT_DOWN == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, POWER_IN_HOLD);
                }
                else if(KEY_EVT_HOLD == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, POWER_IN_L_HOLD);
                }
                if(me->combinedKey == COM_KEY_POWER_ON)
                {
                    me->combinedKey = COM_KEY_INVALID;
                    return Q_TRAN(&MainApp_PoweringDown);
                }
            }
            break;
        }
        case MAINAPP_FAST_BOOT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_FAST_BOOT_SIG", e->sig);
            return Q_TRAN(&MainApp_FastBoot);
        }
		case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer = 0;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Active state
 * Purpose: normal state, ASE-TK should work now
 */
QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    static bool started = FALSE;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            
            /* TODO: maybe this boot up is after FEP upgrade, means ASE-TK boot up and wait long time.
             *       FEP may miss many events (ex. network-standby or ON), thus we need request SUE
             *       to send state event again
             */
            
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, NORMAL_MODE);

            AudioSrv_SetAudio(AUDIO_RESET_LINEIN_JACK_AND_MUSIC_STA_SETT_ID, /*No used*/TRUE, /*No used*/0);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            me->tickHandlers[TIMER_ID_VOLUME_CHANGE_TIMEOUT].timer = MAINAPP_VOLUME_CHANGE_TIMEOUT_IN_MS;
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
                MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
            }
            return Q_HANDLED();
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            AudioMusicDetectStateEvt* pAudioMusicStateEvt = (AudioMusicDetectStateEvt*)e;
            FepAseEvent_Event fepAseEvt = FepAseEvent_Event_LINE_SENSE_ACTIVE;
            eAudioSource audioSrc = AUDIO_SOURCE_MAX;
            if (pAudioMusicStateEvt->hasMusicStream)
            {
                if (pAudioMusicStateEvt->jackId == AUXIN_JACK)
                {
                    fepAseEvt = FepAseEvent_Event_LINE_SENSE_ACTIVE;
                    AudioSrv_SetAudio(AUDIO_LINEIN_MULTI_ROOM_CHANNEL_SETT_ID, TRUE, me->sourceHandler[AUDIO_SOURCE_AUXIN].audioChannel);
#ifdef SOURCE_HIJACK_IN_MCU
                    audioSrc = AUDIO_SOURCE_AUXIN;
#endif
                }
                else if (pAudioMusicStateEvt->jackId == SPDIF1_IN_JACK)
                {
                    fepAseEvt = FepAseEvent_Event_TOSLINK_SENSE_ACTIVE;
                    AudioSrv_SetAudio(AUDIO_LINEIN_MULTI_ROOM_CHANNEL_SETT_ID, TRUE, me->sourceHandler[AUDIO_SOURCE_SPDIF_IN].audioChannel);
#ifdef SOURCE_HIJACK_IN_MCU
                    audioSrc = AUDIO_SOURCE_SPDIF_IN;
#endif

                }
                else if (pAudioMusicStateEvt->jackId == JACK_IN_INVALID) //ASE-TK source
                {
                    audioSrc = AUDIO_SOURCE_WIFI;
                }
            }
            else
            {
                if (pAudioMusicStateEvt->jackId == AUXIN_JACK)
                {
                    fepAseEvt = FepAseEvent_Event_LINE_SENSE_INACTIVE;
                    audioSrc = AUDIO_SOURCE_WIFI;
                }
                else if (pAudioMusicStateEvt->jackId == SPDIF1_IN_JACK)
                {
                    fepAseEvt = FepAseEvent_Event_TOSLINK_SENSE_INACTIVE;
                    audioSrc = AUDIO_SOURCE_WIFI;
                }
#ifdef SOURCE_HIJACK_IN_MCU
                else if (pAudioMusicStateEvt->jackId == JACK_IN_INVALID) //ASE-TK source
                {
                    if (*(bool*)Setting_Get(SETID_AUXIN_MUSIC_DET))
                    {
                        fepAseEvt = FepAseEvent_Event_LINE_SENSE_ACTIVE;
                        audioSrc = AUDIO_SOURCE_AUXIN;
                    }
                    else if (*(bool*)Setting_Get(SETID_SPDIF_IN_MUSIC_DET))
                    {
                        fepAseEvt = FepAseEvent_Event_TOSLINK_SENSE_ACTIVE;
                        audioSrc = AUDIO_SOURCE_SPDIF_IN;
                    }
                    if(audioSrc != AUDIO_SOURCE_MAX)
                    {
                        AseTkSrv_SendFepAseEvent(fepAseEvt);
                    }
                }
#endif
            }
            if(pAudioMusicStateEvt->jackId != JACK_IN_INVALID)
            {
                //AseTkSrv_SendFepAseEvent(fepAseEvt);
            }
#ifdef SOURCE_HIJACK_IN_MCU
            if(audioSrc != AUDIO_SOURCE_MAX)
            {
                MainApp_SwitchAudioSource(me, audioSrc);
            }
#endif
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            if (!started)
            {
                KeyStateEvt *evt = (KeyStateEvt*)e;

                printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!![%d]\n",evt->keyEvent);
                break;
            }
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            QState trans_state= MainApp_KeyHandler(me,e);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            break;
        }
        case ASE_TK_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            QState trans_state= MainApp_AseTkEvtHandler(me,e);
            if(trans_state!=Q_UNHANDLED()) 
            {
                return trans_state;
            }
            else 
            {
                return Q_HANDLED();
            }
        }
        case SAFETY_ALARM_OVERTEMPERATURE:
        {
            break;
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            static uint32 vol_delay = 0;
            
            if(++vol_delay == 10)
            {
                AudioSrv_SetVolume(DEFAULT_VOLUME);                

                MainApp_SwitchAudioSource(me, 2);
 
                started = TRUE;
            }
            
            QState trans_state = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->combinedKey = COM_KEY_INVALID;
            return Q_HANDLED();
        }
        default:
            break;
    }
    
    if(Q_UNHANDLED() != ret)
    {
        return ret;
    }
    else
    {
        return Q_SUPER(&QHsm_top);
    }
}

QState MainApp_FastBoot(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
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
                MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                MainApp_SendLedReq(me, LED_IND_ID_FAST_BOOT_MODE);
                AudioSrv_SetVolume(DEFAULT_VOLUME);
                /* In fast boot mode switch to Auxin by default.*/
                MainApp_SwitchAudioSource(me, AUDIO_SOURCE_AUXIN);
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
            if(STANDBY_KEY == (evt->keyId))
            {
                if(KEY_EVT_DOWN == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, POWER_IN_HOLD);
                }
                else if(KEY_EVT_HOLD == (evt->keyEvent) || KEY_EVT_SHORT_PRESS == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, POWER_IN_L_HOLD);
                }
                if(me->combinedKey == COM_KEY_POWER_ON)
                {
                    me->combinedKey = COM_KEY_INVALID;
                    MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                    TP_PRINTF("\r\n\r\n*** Power off when MainApp_FastBoot ***\r\n");
                    return Q_TRAN(&MainApp_PoweringDown);
                }
            }
            return Q_HANDLED();
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->combinedKey = COM_KEY_INVALID;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}


