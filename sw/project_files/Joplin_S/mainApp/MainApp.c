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
#include "trace.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "projBsp.h" /* for  ProjBsp_SoftReboot()*/
#include "SettingSrv.h"
#include "tym_qp_lib.h"
#include "tym.pb.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "MainApp_bluetoothEvtHandler.h"
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



static QEQueue deferredMainAppReqQue;
static QEvt const *pDeferredMainAppReqQueSto[6];


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
    QActive_subscribe((QActive*)me, BT_STATE_SIG);
    QActive_subscribe((QActive*)me, AUDIO_STATE_SIG);
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);
    QActive_subscribe((QActive*)me, LED_STATE_SIG);
    QActive_subscribe((QActive*)me, POWER_BATT_STATE_SIG);
    QActive_subscribe((QActive*)me, POWER_WAKE_UP_SIG);
    QActive_subscribe((QActive*)me, POWER_STATE_SIG);
    QActive_subscribe((QActive*)me, AUDIO_MUSIC_STREAM_STATE_SIG);

    /* Initial variable*/
    // To Do:  initial MainApp variable and setting
    me->sourceHandler = sourceHandlerList;
    ASSERT(ArraySize(ledIndList) == LED_IND_ID_MAX);
    me->ledInds = ledIndList;
    me->tickHandlers  = (tTickHandlerList *)MainApp_GetTickHandlerList();
    MainApp_InitStatusVariables(me);

#ifdef HAS_IWDG
    IwdgInit(IWDG_Prescaler_256, IWDG_RLR_RL);
    IWDG_ReloadCounter();
    RTC_Initialize();
    RTC_SetUpWakeUpAlarm(IWDG_FEED_PERIOD_SEC);
#endif

    return Q_TRAN(&MainApp_Off);
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

/* Power off state
 * Purpose:Power off system once PWR_EN set to low, MCU also lose it's power
 */
QState MainApp_Off(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, OFF_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            // To Do: power off sync finished here, turn off system here set PWR_EN to low
            MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
            me->vol = DEFAULT_VOLUME*2;
            return Q_HANDLED();
        }
        case POWER_STATE_SIG:
        {
            if (Setting_IsReady(SETID_IS_PWR_SWITH_ON))
            {
                bool onOffSta = *(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON);
                if(onOffSta == PWR_SW_ON)
                {
                    return Q_TRAN(&MainApp_PoweringUp);
                }
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* BT module boot up state
 * Purpose: wait BT module booting  up
 */
QState MainApp_BtModuleBooting(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, WAIT_MODE);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
#ifdef WAIT_BT_MODULE_BOOT_UP
            return Q_HANDLED();

#else
            return Q_TRAN(&MainApp_PoweringUp);
#endif
        }
        case BT_STATE_SIG:
        {
            Proto_Tym_BtMcuMessage *pMessage = (Proto_Tym_BtMcuMessage *)((BtRxEvt *)e)->pData;
            if (pMessage->which_OneOf == Proto_Tym_BtMcuMessage_btMcuEvent_tag && pMessage->OneOf.btMcuEvent.type == Proto_BtMcu_Event_Type_BOOTED)
            {
                return Q_TRAN(&MainApp_PoweringUp);
            }

            return Q_HANDLED();

        }
        case MAINAPP_TIMEOUT_SIG:
        {
            QState trans_state = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(trans_state!=Q_UNHANDLED())
            {
                return trans_state;
            }
            else
            {
                return Q_HANDLED();
            }
        }
        case KEY_STATE_SIG:
        {

            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            me->tickHandlers[TIMER_ID_BT_BOOTING_TIMEOUT].timer = INVALID_VALUE;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* System Powering up state
 * Purpose: wait System boot up
 */
QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, POWERING_UP_MODE);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            return Q_TRAN(&MainApp_Active);
        }
        case BT_STATE_SIG:
        {
            Proto_Tym_BtMcuMessage *pMessage = (Proto_Tym_BtMcuMessage *)((BtRxEvt *)e)->pData;
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
    return Q_SUPER(&QHsm_top);
}

/* Active state
 * Purpose: normal state, BTshould work now
 */
QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);

            if (me->sourceHandler[AUDIO_SOURCE_RCA].bIsValid)
            {
                TP_PRINTF("RCA is activated. \r\n");
            }
            MainApp_SwitchMode(me, NORMAL_MODE);
            MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( NORMAL_MODE != evt->modeId )   //ignore unexpected SYSTEM_MODE_RESP_SIG
            {
                //ASSERT(0);
            }
            else
            {
                /* Set audio source */
                Setting_Set(SETID_AUDIO_SOURCE, &(me->audioSource));
                AudioSrv_SetChannel((QActive *)me, me->sourceHandler[me->audioSource].audioChannel);
                MainApp_SendLedReq(me, me->sourceHandler[me->audioSource].ledInd);
                /* Set volume */
                AudioSrv_SetVolume(me->vol/2);
                MainApp_SetRotaterLedOn(me, LED_VOL_0, vol_led_mapping[me->vol/2]);

                AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, me->bass/2, NULL);
                MainApp_SetRotaterLedOn(me, LED_BAS_0, me->bass/2);

                AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, me->treble/2, NULL);
                MainApp_SetRotaterLedOn(me, LED_TRE_0, me->treble/2);

                /* Unmute System */
                me->muteStatus = FALSE;
                AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, me->muteStatus);
            }
            return Q_HANDLED();
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            AudioMusicDetectStateEvt* pAudioMusicStateEvt = (AudioMusicDetectStateEvt*)e;
            // To DO: send AudioMusicDetectState.
            //TP_PRINTF("\r\nReport to ASE: Aux-in music states =%d\r\n\r\n", pAudioMusicStateEvt->hasMusicStream);
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            QState trans_state= MainApp_KeyHandler(me,e);
            if(trans_state!=Q_UNHANDLED())
            {
                return trans_state;
            }
            break;
        }
        case POWER_STATE_SIG:
        {
            if (Setting_IsReady(SETID_IS_PWR_SWITH_ON))
            {
                bool onOffSta = *(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON);
                if(onOffSta == PWR_SW_ON)
                {
                    MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                    return Q_TRAN(&MainApp_PoweringDown);
                }
            }
            return Q_HANDLED();
        }
        case BT_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            QState trans_state= MainApp_BluetoothEvtHandler(me,e);
            if(trans_state!=Q_UNHANDLED())
            {
                return trans_state;
            }
            else
            {
                return Q_HANDLED();
            }
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            QState trans_state = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(trans_state!=Q_UNHANDLED())
            {
                return trans_state;
            }
            else
            {
                return Q_HANDLED();
            }
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

    if(Q_UNHANDLED() != ret)
    {
        return ret;
    }
    else
    {
        return Q_SUPER(&MainApp_Base);
    }
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
            /* Stay in Powering down state 2 seconds to finish LED fading off. */
            me->tickHandlers[TIMER_ID_POWER_DOWN_TIMEOUT].timer = MAINAPP_POWER_DOWN_TIMEOUT_IN_MS;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            PersistantObj_RefreshTick((cPersistantObj*)me, 2000);
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            return Q_HANDLED();
        }

        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            QState trans_state= MainApp_KeyHandler(me,e);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if(evt->keyEvent == KEY_EVT_UP && evt->keyId == POWER_KEY)
            {
                return Q_TRAN(&MainApp_Off);
            }
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


/* Standby state
*Purpose: LED dim to let user know system is at idle state
*/
QState MainApp_Standby(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RES", e->sig);
            return Q_HANDLED();
        }

        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);
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

            // To Do:  reboot process

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


