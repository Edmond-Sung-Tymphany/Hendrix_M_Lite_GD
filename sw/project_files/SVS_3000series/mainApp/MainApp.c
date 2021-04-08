/**
*  @file      main_app.c
*  @brief     Main application for BnO CA17
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
#include "tym_qp_lib.h"

#include "AudioSrv.h"
#include "KeySrv.h"
#include "projBsp.h" /* for  ProjBsp_SoftReboot()*/
#include "SettingSrv.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "MainApp_keyEvtHandler.h"
#include "MainApp_tickEvtHandler.h"
#include "MainApp_bleEvtHandler.h"
#include "piu_common.h"
#include "BleSrv.h"

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
static QEQueue deferredReqQue;
static QEvt const *pDeferredReqQueSto[6];

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

    QEQueue_init(&deferredReqQue, pDeferredReqQueSto, Q_DIM(pDeferredReqQueSto));
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

    /* suppress the compiler warning about unused parameter */
    (void)(e);

    /* Subsrcribe to all the SIGS */
    QActive_subscribe((QActive*)me, AUDIO_STATE_SIG);
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);
    QActive_subscribe((QActive*)me, AUDIO_MUSIC_STREAM_STATE_SIG);

    //
    MainApp_InitGpioLeds();
    MainApp_InitBrightnessList();

    me->tickHandlers = (tTickHandlerList *)MainApp_GetTickHandlerList();
    me->systemStatus = SYSTEM_STA_ON;

    me->musicPlaying = FALSE;
    me->sysTempLevel = AMP_TL_NORMAL;
    me->dcSense = DC_IN_STA_ON;
    me->pageSetting  = PAGE_SETTING_VOL;

    IwdgInit(IWDG_Prescaler_256, IWDG_RLR_RL);
    IwdgReloadActiveMode();

    //RTC_Initialize();
    //RTC_SetUpWakeUpAlarm(IWDG_FEED_PERIOD_SEC);

    /*save data data */
    Setting_Set(SETID_SW_VER,  MCU_VERSION);
    Setting_Set(SETID_DSP_VER, DSP_VERSION);

    return Q_TRAN(&MainApp_PreActive);
}

/* PowerInit state
 * Purpose: system initialize before power up
 */
static QState MainApp_PreActive(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);

            // initial database, ensure all the data is corret even reading from flash encounter errors.
            me->pMenuData = (int16 *)Setting_Get(SETID_MENU_DATA);
            MainApp_ValidateData(me);
            me->standbyMode = me->prevStandbyMode = MainApp_GetMenuData(me, PAGE_SETTING_STANDBY);

            MainApp_SwitchMode(me, POWERING_UP_MODE); //next: SYSTEM_MODE_RESP_SIG
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);

            //Start stack usage check and never stop
            me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = MAINAPP_STACK_CHECK_TIMEOUT_IN_MS;

            TP_PRINTF("\r\n-------------------------------------------------------\r\n");
            TP_PRINTF(TP_PRODUCT " SW ver: v%s, built on %s %s\r\n", MCU_VERSION, __DATE__, __TIME__);
            TP_PRINTF("-------------------------------------------------------\r\n");

            // display powering up led before entering active
            MainApp_PoweringUpLeds(me);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_POWERING_UP_IN_MS);
            return Q_HANDLED();
        }
        case JACK_STATE_SIG:
        {
            PowerSrvJackStateEvt *pJackState = (PowerSrvJackStateEvt*)e;
            MainApp_JackHandler(me, pJackState->type, pJackState->param);
            //QActive_defer((QActive*)me, &deferredReqQue, e);
            return Q_HANDLED();
        }
        case SYSTEM_SLEEP_RESP_SIG:
        {
            return Q_TRAN(&MainApp_DeActive);
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            return Q_TRAN(&MainApp_Active);
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
        {
            break;
        }
    }

    return Q_SUPER(&QHsm_top);
}

/* Active state
 * Purpose: normal state, ASE-TK should work now
 */
QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    QState trans_state = Q_UNHANDLED();

    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, NORMAL_MODE);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( NORMAL_MODE != evt->modeId )
            {
                //ignore unexpected SYSTEM_MODE_RESP_SIG
                ASSERT(0);
            }
            else
            {
                me->systemStatus = SYSTEM_STA_ON;
                /* update ticks */
                me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
                me->tickHandlers[TIMER_ID_IDLE_CHECK_TIMEOUT].timer = MainApp_GetIdleTimeout(me);
                me->tickHandlers[TIMER_ID_ENTER_STANDBY_TIMEOUT].timer = MainApp_GetStandbyTimeout(me);
                me->tickHandlers[TIMER_ID_JACK_LOW_TIMEOUT].timer = MainApp_GetJackLowTimeout(me);
                me->tickHandlers[TIMER_ID_STARTUP_AMP_TIMEOUT].timer = TIMER_ID_STARTUP_AMP_TIMEOUT_IN_MS;
                /* Now AudioDrv already update temp and level, then check overheat here
                 */
                eAmpTempLevel tempAmp = *((eAmpTempLevel*)Setting_Get(SETID_AMP_TEMP_LEVEL));
                if( tempAmp >= AMP_TL_CRITICAL)
                {
                    TP_PRINTF("\r\n*** do not allow boot when overheat ***\r\n\r\n");
//                    return Q_TRAN(&MainApp_Standby);
                }

                TymQP_QActive_RecallAllFIIO((QActive*)me, &deferredReqQue);

                /* turn on led bar*/
                bool screenStatus = TRUE;
                Setting_Set(SETID_SCREEN_STATUS, &screenStatus);

                /* update the LED*/
                MainApp_UpdateLeds(me);

                PowerSrv_Set((QActive*)me, POWER_SET_ID_CTRL, TRUE);
                PowerSrv_Set((QActive*)me, POWER_SET_ID_BD2242G, TRUE);

                /* startup amp */
                AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, FALSE, 0, 0);
                AudioSrv_SendMuteReq((QActive*)me, AUDIO_DSP_DACOUT_MUTE, FALSE);

                /* set the volume */
                AudioSrv_SetVolume(MainApp_GetMenuData(me, PAGE_SETTING_VOL));

                PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            }

            return Q_HANDLED();
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            //AudioMusicDetectStateEvt* pAudioMusicStateEvt = (AudioMusicDetectStateEvt*)e;
            if(STANDBY_MODE_AUTO == me->standbyMode)
            {
                me->tickHandlers[TIMER_ID_ENTER_STANDBY_TIMEOUT].timer = MainApp_GetStandbyTimeout(me);
            }

            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            trans_state = MainApp_KeyHandler(me, e);

            if(Q_HANDLED() == trans_state)
            {
                me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
                me->tickHandlers[TIMER_ID_IDLE_CHECK_TIMEOUT].timer = MainApp_GetIdleTimeout(me);
                me->tickHandlers[TIMER_ID_ENTER_STANDBY_TIMEOUT].timer = MainApp_GetStandbyTimeout(me);
                me->tickHandlers[TIMER_ID_JACK_LOW_TIMEOUT].timer = MainApp_GetJackLowTimeout(me);
                Setting_Set(SETID_MENU_DATA, me->pMenuData);

                bool screenStatus = *(bool*)Setting_Get(SETID_SCREEN_STATUS);
                if (screenStatus)
                {
                    MainApp_UpdateLeds(me);
                }
                else
                {
                    MainApp_CleanLeds(me);
                }
            }

            break;
        }
        case SAFETY_ALARM_OVERTEMPERATURE:
        {
            return Q_HANDLED();
        }
        case JACK_STATE_SIG:
        {
            PowerSrvJackStateEvt *pJackState = (PowerSrvJackStateEvt*)e;
            trans_state = MainApp_JackHandler(me, pJackState->type, pJackState->param);
            trans_state = (Q_UNHANDLED() == trans_state) ? Q_HANDLED() : trans_state;
            me->tickHandlers[TIMER_ID_ENTER_STANDBY_TIMEOUT].timer = MainApp_GetStandbyTimeout(me);
            me->tickHandlers[TIMER_ID_JACK_LOW_TIMEOUT].timer = MainApp_GetJackLowTimeout(me);
            MainApp_UpdateLeds(me);
            break;
        }
        case POWER_STATE_SIG:
        {
            PowerSrvInfoEvt *pPowerState = (PowerSrvInfoEvt*)e;
            trans_state = MainApp_PowerHandler(me, pPowerState->dcInStatus);
            trans_state = (Q_UNHANDLED() == trans_state) ? Q_HANDLED() : trans_state;
            break;
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            trans_state = MainApp_ActiveTickEvtHandler(me, e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);

            /* this signal must be set to handled,
                otherwise it will be transited to other state-handler
             */
            trans_state = (Q_UNHANDLED() == trans_state) ? Q_HANDLED() : trans_state;

            break;
        }
        case BLE_WRITE_DATA_REQ_SIG:
        case BLE_READ_DATA_REQ_SIG:
        case BLE_RESET_ITEM_REQ_SIG:
        case BLE_MENU_FEATURE_REQ_SIG:
        case BLE_UART_VERIFY_REQ_SIG:
        case MAINAPP_DFU_REQ_SIG:
        case MAINAPP_VERSION_REQ_SIG:
        case MAINAPP_PRODUCT_NAME_REQ_SIG:
        {
            trans_state = MainApp_BleEvtHandler(me, e);
            if(Q_HANDLED() == trans_state)
            {
                /* update led and ticks*/
                me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
                me->tickHandlers[TIMER_ID_IDLE_CHECK_TIMEOUT].timer = MainApp_GetIdleTimeout(me);
                me->tickHandlers[TIMER_ID_ENTER_STANDBY_TIMEOUT].timer = MainApp_GetStandbyTimeout(me);
                me->tickHandlers[TIMER_ID_JACK_LOW_TIMEOUT].timer = MainApp_GetJackLowTimeout(me);
                Setting_Set(SETID_MENU_DATA, me->pMenuData);
                MainApp_UpdateLeds(me);
            }

            /* this signal must be set to handled,
                otherwise it will be transited to other state-handler
             */
            trans_state = (Q_UNHANDLED() == trans_state) ? Q_HANDLED() : trans_state;

            break;
        }
        case MAINAPP_SET_VOL_REQ_SIG:
        {
            MainAppReqEvt *reqEvt = (MainAppReqEvt *)e;
            MainApp_SetVolume(me, reqEvt->value);
            MainApp_UpdateLeds(me);
            break;
        }
        case MAINAPP_GET_VOL_REQ_SIG:
        {
            MainApp_GetVolume(me);
            break;
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
             me->tickHandlers[TIMER_ID_STARTUP_AMP_TIMEOUT].timer = INVALID_VALUE;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
        {
            break;
        }
    }

    if(Q_UNHANDLED() == trans_state)
    {
        return Q_SUPER(&QHsm_top);
    }
    else
    {
        return trans_state;
    }
}

QState MainApp_DeActive(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_ENTER_STANDBY_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_IDLE_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_JACK_LOW_TIMEOUT].timer = INVALID_VALUE;
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_DSP_DACOUT_MUTE, TRUE);
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_MUTE, TRUE);

            PowerSrv_Set((QActive*)me, POWER_SET_ID_CTRL, FALSE);
            PowerSrv_Set((QActive*)me, POWER_SET_ID_BD2242G, FALSE);
            /*switch controller mode, let all servers enter deactive mode */
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);

            MainApp_SetGpioLeds(me, GPIO_OUT_LED_LPF, FALSE);
            MainApp_SetGpioLeds(me, GPIO_OUT_LED_PHASE, FALSE);
            MainApp_SetGpioLeds(me, GPIO_OUT_LED_RESET, FALSE);
            return Q_HANDLED();
        }
        case POWER_STATE_SIG:
        {
            /* reset system when got the power signal again
                i don't think so this is necessary, it will re-write the flash, that is not good for internal flash
            */
            //SettingSrv_BookkeepingEx(me);
            BSP_SoftReboot();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default: break;
    }

    return Q_SUPER(&QHsm_top);
}

QState MainApp_Idle(cMainApp * const me, QEvt const * const e)
{
    QState trans_state = Q_UNHANDLED();
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));

            me->systemStatus = SYSTEM_STA_IDLE;
            me->pageSetting = PAGE_SETTING_VOL;
            me->tickHandlers[TIMER_ID_IDLE_CHECK_TIMEOUT].timer = INVALID_VALUE;    // disable

            /* following sentence will update the trigger timeout when trigger is inserted at MainApp_Active state.
             * me->tickHandlers[TIMER_ID_JACK_LOW_TIMEOUT].timer = MainApp_GetJackLowTimeout(me);
             */

            bool screenStatus = *(bool*)Setting_Get(SETID_SCREEN_STATUS);
            if (screenStatus)
            {
                MainApp_DimLeds(me);
            }
            else
            {
                MainApp_CleanLeds(me);
            }

            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }        
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            //AudioMusicDetectStateEvt* pAudioMusicStateEvt = (AudioMusicDetectStateEvt*)e;

            if(STANDBY_MODE_AUTO == me->standbyMode)
            {
                me->tickHandlers[TIMER_ID_ENTER_STANDBY_TIMEOUT].timer = MainApp_GetStandbyTimeout(me);
            }
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            trans_state = MainApp_KeyHandler(me, e);
            if(Q_HANDLED() == trans_state)
            {
                return Q_TRAN(MainApp_Active);
            }
        }
        case SAFETY_ALARM_OVERTEMPERATURE:
        {
            break;
        }
        case JACK_STATE_SIG:
        {
            PowerSrvJackStateEvt *pJackState = (PowerSrvJackStateEvt*)e;
            trans_state = MainApp_JackHandler(me, pJackState->type, pJackState->param);
            me->tickHandlers[TIMER_ID_JACK_LOW_TIMEOUT].timer = MainApp_GetJackLowTimeout(me);
            MainApp_DimLeds(me);
            trans_state = (Q_UNHANDLED() == trans_state) ? Q_HANDLED() : trans_state;

            break;
        }
        case POWER_STATE_SIG:
        {
            PowerSrvInfoEvt *pPowerState = (PowerSrvInfoEvt*)e;
            trans_state = MainApp_PowerHandler(me, pPowerState->dcInStatus);
            trans_state = (Q_UNHANDLED() == trans_state) ? Q_HANDLED() : trans_state;

            break;
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            trans_state = MainApp_ActiveTickEvtHandler(me, e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            /* this signal must be set to handled,
                otherwise it will be transited to other state-handler
             */
            trans_state = (Q_UNHANDLED() == trans_state) ? Q_HANDLED() : trans_state;
            break;
        }
        case BLE_WRITE_DATA_REQ_SIG:
        case BLE_READ_DATA_REQ_SIG:
        case BLE_RESET_ITEM_REQ_SIG:
        case BLE_MENU_FEATURE_REQ_SIG:
        case BLE_UART_VERIFY_REQ_SIG:
        case MAINAPP_DFU_REQ_SIG:
        case MAINAPP_VERSION_REQ_SIG:
        case MAINAPP_PRODUCT_NAME_REQ_SIG:
        case MAINAPP_SET_VOL_REQ_SIG:
        case MAINAPP_GET_VOL_REQ_SIG:
        {
           QActive_defer((QActive*)me, &deferredReqQue, e);
           return Q_TRAN(MainApp_Active);
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
        {
            break;
        }
    }

    if(Q_UNHANDLED() == trans_state)
    {
        return Q_SUPER(&QHsm_top);
    }
    else
    {
        return trans_state;
    }
}

QState MainApp_Standby(cMainApp * const me, QEvt const * const e)
{
    QState trans_state = Q_UNHANDLED();
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->systemStatus = SYSTEM_STA_STANDBY;
            me->pageSetting = PAGE_SETTING_VOL;
            PowerSrv_Set((QActive*)me, POWER_SET_ID_CTRL, FALSE);
            PowerSrv_Set((QActive*)me, POWER_SET_ID_BD2242G, FALSE);

            /* turn on led bar*/
            bool screenStatus = *(bool*)Setting_Get(SETID_SCREEN_STATUS);

            if (screenStatus)
            {
                MainApp_StandbyLeds(me);
            }

            /* update ticks */
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_IDLE_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_ENTER_STANDBY_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SHUTDOWN_AMP_TIMEOUT].timer = TIMER_ID_SHUTDOWN_AMP_TIMEOUT_IN_MS;

            AudioSrv_SendMuteReq((QActive *)me, AUDIO_DSP_DACOUT_MUTE, TRUE);
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_MUTE, TRUE);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            bool musicStatus = *(bool*)Setting_Get(SETID_MUSIC_DET);

            if(STANDBY_MODE_AUTO == me->standbyMode && musicStatus)
            {
                return Q_TRAN(MainApp_Active);
            }

            return Q_HANDLED();
        }
        case JACK_STATE_SIG:
        {
            PowerSrvJackStateEvt *pJackState = (PowerSrvJackStateEvt*)e;
            trans_state = MainApp_JackHandler(me, pJackState->type, pJackState->param);
            me->tickHandlers[TIMER_ID_JACK_LOW_TIMEOUT].timer = MainApp_GetJackLowTimeout(me);
            trans_state = (Q_UNHANDLED() == trans_state) ? Q_HANDLED() : trans_state;

            break;
        }
        case POWER_STATE_SIG:
        {
            PowerSrvInfoEvt *pPowerState = (PowerSrvInfoEvt*)e;
            trans_state = MainApp_PowerHandler(me, pPowerState->dcInStatus);
            trans_state = (Q_UNHANDLED() == trans_state) ? Q_HANDLED() : trans_state;

            break;
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            trans_state = MainApp_ActiveTickEvtHandler(me, e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            /* this signal must be set to handled,
                otherwise it will be transited to other state-handler
             */
            trans_state = (Q_UNHANDLED() == trans_state) ? Q_HANDLED() : trans_state;
            break;
        }
        case KEY_STATE_SIG:
        {
            trans_state = MainApp_KeyHandler(me, e);
            if(Q_HANDLED() == trans_state)
            {
                return Q_TRAN(MainApp_Active);
            }
        }
        case BLE_WRITE_DATA_REQ_SIG:
        case BLE_READ_DATA_REQ_SIG:
        case BLE_RESET_ITEM_REQ_SIG:
        case BLE_MENU_FEATURE_REQ_SIG:
        case BLE_UART_VERIFY_REQ_SIG:
        case MAINAPP_DFU_REQ_SIG:
        case MAINAPP_VERSION_REQ_SIG:
        case MAINAPP_PRODUCT_NAME_REQ_SIG:
        {
           QActive_defer((QActive*)me, &deferredReqQue, e);
           return Q_TRAN(MainApp_Active);
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->tickHandlers[TIMER_ID_SHUTDOWN_AMP_TIMEOUT].timer = INVALID_VALUE;
            return Q_HANDLED();
        }
        default:
        {
            break;
        }
    }

    if(Q_UNHANDLED() == trans_state)
    {
        return Q_SUPER(&QHsm_top);
    }
    else
    {
        return trans_state;
    }
}

QState MainApp_FactoryReset(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));

            /* Mute the amp first */
            AudioSrv_SendMuteReq((QActive*)me, AUDIO_DSP_DACOUT_MUTE, TRUE);
            AudioSrv_SendMuteReq((QActive*)me, AUDIO_AMP_MUTE, TRUE);
            /*switch controller mode, let all servers enter deactive mode */
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);

            MainApp_ResetSettings(me);

            /* Reset BTLE */
            BleSrv_SendBleCmd(BLE_FACTORY_RESET_CMD);

            // initial database
            me->pageSetting  = PAGE_SETTING_VOL;

            /* during powering up, APP will read the value from MenuData,
            * but PAGE_SETTING_STANDBY might be not change.
            * workaround solution: update it to default value during powering up
            */
            MainApp_CleanLeds(me);

            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_FACTORY_RESET_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            SettingSrv_BookkeepingEx();
            BSP_SoftReboot();
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
        {
            break;
        }
    }

    return Q_SUPER(&QHsm_top);
}

QState MainApp_EnterUpgrading(cMainApp * const me, QEvt const * const e)
{
    static bool startJumpFlag = FALSE;

    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);

            QTimeEvt_disarm(TIME_EVT_OF(me));
            /* cancel all the timeout handlers */
            MainApp_DeleteAllTickHandlers();

            me->systemStatus = SYSTEM_STA_STANDBY;
            startJumpFlag = FALSE;

            /* update leds in jumper/bootloader*/
            MainApp_UpgradingLeds(me);

            /* Mute the amp first */
            //AudioSrv_SendMuteReq((QActive*)me, AUDIO_DSP_DACOUT_MUTE, TRUE);
            PowerSrv_Set((QActive*)me, POWER_SET_ID_BD2242G, FALSE);

            /* stop DSP running to save power */

            /* save the boot mode */
            if(UPGRADE_BY_APP == me->upgradeMethod)
            {
                uint32 bootMode = BOOT_MODE_BOOTLOADER;
                Setting_Set(SETID_BOOT_MODE, &bootMode);
            }

            //SettingSrv_BookkeepingEx();
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);

            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            /* There is an issue here, but I didn't find the root cause yet.
            *  normally, QP will send "MAINAPP_TIMEOUT_SIG" signal after setup the time
            *  but here, sometimes it will also send "MAINAPP_TIMEOUT_SIG" before setup time
            *  that is sometimes "SYSTEM_MODE_RESP_SIG" can't trigger, but also can trigger "MAINAPP_TIMEOUT_SIG"
            */
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);

            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if(POWERING_DOWN_MODE == evt->modeId)
            {
                PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_UPGRADE_TIMEOUT_IN_MS);
                startJumpFlag = TRUE;
            }
            else
            {
                ASSERT(0);
            }

            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me,"(%d)MAINAPP_TIMEOUT_SIG\r\n",e->sig);

            if(startJumpFlag)
            {
                if(UPGRADE_BY_APP == me->upgradeMethod)
                {
                    BSP_SoftReboot();
                }
                else if(UPGRADE_BY_USB == me->upgradeMethod)
                {
                    jump2Address(ADDR_SYSTEM_MEMORY);
                }
            }

            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
        {
            break;
        }
    }

    return Q_SUPER(&QHsm_top);
}


