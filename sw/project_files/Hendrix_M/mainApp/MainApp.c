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
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "MainApp_bluetoothKeyEvtHandler.h"
#include "MainApp_keyEvtHandler.h"
#include "MainApp_tickEvtHandler.h"
#include "MainApp.Config"
#include "IdleDlg.h"


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

static cIdleDlg IdleDlg = {{0}};
static cIdleDlg *pIdleDlg = NULL;



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
    QActive_subscribe((QActive*)me, POWER_STATE_SIG);
    QActive_subscribe((QActive*)me, AUDIO_MUSIC_STREAM_STATE_SIG);

    /* Initial variable*/
    // To Do:  initial MainApp variable and setting
    me->sourceHandler = NULL;
    ASSERT(ArraySize(ledIndList) == LED_IND_ID_MAX);
    me->ledInds = ledIndList;
    me->tickHandlers  = (tTickHandlerList *)MainApp_GetTickHandlerList();
    MainApp_InitStatusVariables(me);
    RingBuf_Ctor(&me->btCueRBufObj, me->btCueBuf, BT_CUE_QUEUE_SIZE);

#ifdef HAS_IWDG
    IwdgInit(IWDG_Prescaler_256, IWDG_RLR_RL);
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
        case MAINAPP_SHUTDOWN_SIG:
        {
            return Q_TRAN(&MainApp_PoweringDown);
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
			TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_CleanAllTickHandlerTimer(me);
            MainApp_SwitchMode(me, OFF_MODE);

            // waiting in off status by MAINAPP_STAY_IN_OFF_TIMEOUT_IN_MS
            // wait for write FLASH
            me->tickHandlers[TIMER_ID_WAIT_IN_OFF_TIMEOUT].timer = MAINAPP_STAY_IN_OFF_TIMEOUT_IN_MS;
            // delay 10s to RESET MCU
            me->tickHandlers[TIMER_ID_DELAYED_ERROR_REBOOT].timer = MAINAPP_WAIT_IN_OFF_REBOOT_TIMEOUT_IN_MS;
            // first time waiting MAINAPP_TICK_IN_OFF_TIMEOUT_IN_MS to timeout tick
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TICK_IN_OFF_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if(OFF_MODE == evt->modeId)
            {
                me->systemStatus = SYSTEM_STA_OFF;
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            QState trans_state= MainApp_ActiveTickEvtHandler(me, e);
            if(trans_state!=Q_UNHANDLED())
            {
                return trans_state;
            }

            if(me->systemStatus != SYSTEM_STA_OFF)
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "MAINAPP_TIMEOUT_SIG,STA,ERR");
                return Q_HANDLED();
            }
            if(!Setting_IsReady(SETID_IS_DC_PLUG_IN)    ||
               !Setting_IsReady(SETID_IS_PWR_SWITH_ON)  ||
               !Setting_IsReady(SETID_BATTERY_STATUS)   )
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "MAINAPP_TIMEOUT_SIG,NOTREADY");
                return Q_HANDLED();
            }

            //Not tran to Shop directly, to make sure power srv is actived in pwering up mode.
            uint32 shopword = *(uint32*)Setting_Get(SETID_SHOP_MODE_WORD);
            if((shopword == SHOP_MODE_VALUE) ||
               (shopword == LS_SAMPLE_VALUE))
            {
                return Q_TRAN(&MainApp_PoweringUp);
            }

            // if DC plugged
            if (TRUE == *(bool*)Setting_Get(SETID_IS_DC_PLUG_IN))
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG,DC", e->sig);
                return Q_TRAN(&MainApp_OffCharging);   //DC plug In change to Off charging state
            }
            //not DC
            // switch up then powering up
            if ((*(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON) == PWR_SW_ON))
            {
                // low battery then sleep
                if(BatteryStatus_LEVEL_LOW >= (BatteryStatus)*(uint8*)Setting_Get(SETID_BATTERY_STATUS))
                {
                    TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG,LOW_BATT", e->sig);
                    return Q_TRAN(&MainApp_Sleep);
                }
                else
                {
                    TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG,SWH ON", e->sig);
                    return Q_TRAN(&MainApp_PoweringUp);
                }
            }
			//BSP_SoftReboot();		//edmond_20210715

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
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_WAIT_IN_OFF_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_DELAYED_ERROR_REBOOT].timer = INVALID_VALUE;
            QTimeEvt_disarm(TIME_EVT_OF(me));
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
            MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP);
            // To Do: Initial process for enter powering up
            me->systemStatus = SYSTEM_STA_POWERING_UP;
            MainApp_SwitchMode(me, POWERING_UP_MODE);
            me->isBTenabled = FALSE;
            me->isNoButtonChanel = TRUE;
            me->isCueChanel = FALSE;
            me->BQBTestPlay = FALSE;
            me->BQBTestVol = FALSE;
            me->isBTStreaming = FALSE;
            me->ConnectedCue = TRUE;
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if(POWERING_UP_MODE == evt->modeId)
            {
                me->currBattLed = 0;
                //reset over temp status;
                me->isCriticalTemp = FALSE;

                if (*(uint32*)Setting_Get(SETID_SHOP_MODE_WORD) == SHOP_MODE_VALUE)
                {
                    return Q_TRAN(&MainApp_Shop);
                }
                else if(*(uint32*)Setting_Get(SETID_SHOP_MODE_WORD) == LS_SAMPLE_VALUE)
                {}
                else
                {
                    //start led timer to show battery led up
                    me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = MAINAPP_TIMEOUT_IN_MS;
                }
                return Q_TRAN(&MainApp_WaitBTPoweringUp);
            }
        }
#ifdef HAS_IWDG
        case TIMER_FEED_WD_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)TIMER_FEED_WD_SIG", e->sig);
            IWDG_ReloadCounter();
            return Q_HANDLED();
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



/* System Powering up state
 * Purpose: wait System boot up
 */
QState MainApp_WaitBTPoweringUp(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            // To Do: Initial process for enter powering up
            me->systemStatus = SYSTEM_STA_WAIT_BT_UP;
            MainApp_SwitchMode(me, WAIT_MODE);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            me->tickHandlers[TIMER_ID_BT_BOOTING_TIMEOUT].timer = MAINAPP_BT_BOOTING_TIMEOUT_IN_MS;
            RingBuf_Reset(&me->btCueRBufObj);
            me->isBTenabled = TRUE;
            me->isBTStreaming = FALSE;
            me->isCuePlaying = FALSE;
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if(WAIT_MODE == evt->modeId)
            {
                me->audioSource = *((eAudioSource*)Setting_Get(SETID_AUDIO_SOURCE));
                if(me->audioSource < AUDIO_SOURCE_MIN || me->audioSource > AUDIO_SOURCE_MAX)
                {
                    me->audioSource = AUDIO_SOURCE_BT;
                    Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
                }

                if(me->audioSource == AUDIO_SOURCE_BT)
                {
                    MainApp_SendLedReq(me, LED_IND_ID_PROD_TRANS_RED);
                    MainApp_SetChannel(me,AUDIO_CHANNEL_BT);
                }
                else
                {
                    MainApp_SendLedReq(me, LED_IND_ID_AUXIN);
                    MainApp_SetChannel(me,AUDIO_CHANNEL_AUXIN);
                }

                if(*(uint32*)Setting_Get(SETID_SHOP_MODE_WORD) == LS_SAMPLE_VALUE)
                    MainApp_SendLedReq(me, LED_IND_ID_BATT_LV_10);

                return Q_HANDLED();
            }
        }
        case BT_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)BT_STATE_SIG", e->sig);
            if (Setting_IsReady(SETID_BT_STATUS))
            {
                uint8_t btStatus = *(uint8_t*)Setting_Get(SETID_BT_STATUS);
                if(IsBluetoothPowerOn(btStatus))
                {
                    TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)BT_STATE_SIG BT_OK", e->sig);
                    // [IN:013812] set default state as connectable
                    me->CurrBTStatus = BT_CONNECTABLE_STA;
                    // if BT already pwrup, then delay to avoid the connected cue
                    if(me->tickHandlers[TIMER_ID_BT_TO_ACTIVE_TIMEOUT].timer <= 0)
                    {
                        me->tickHandlers[TIMER_ID_BT_TO_ACTIVE_TIMEOUT].timer = MAINAPP_BT_TO_ACTIVE_TIMEOUT_IN_MS;
                    }
                    else if(btStatus == BT_CONNECTED_STA &&
                            RingBuf_IsEmpty(&me->btCueRBufObj))
                    {
                        // for bug [IN:013899]
                        uint8 btcmd = BT_PAIRING_SUCCESS_CUE_CMD;
                        RingBuf_Push(&me->btCueRBufObj, &btcmd, sizeof(uint8));
                    }
                }
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_BT_BOOTING_TIMEOUT].timer = INVALID_VALUE;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);

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
            me->systemStatus = SYSTEM_STA_ON;
            MainApp_SwitchMode(me, NORMAL_MODE);

            BluetoothSrv_SendBtCmd((QActive*)me, BT_ANSWER_CMD);
            me->battChgTemp_bk =TL_NORMAL;
            me->battDischgTemp_bk =TL_NORMAL;
#ifdef HAS_AUTO_BOOST_CONTROL
            if(*(uint8*)Setting_Get(SETID_IS_DC_PLUG_IN))
                PowerSrv_Set((QActive*)me, POWER_SET_ID_BOOST_ENABLE,FALSE);
            else
                PowerSrv_Set((QActive*)me, POWER_SET_ID_BOOST_ENABLE,TRUE);
#endif
            PowerSrv_Get((QActive*)me, POWER_GET_ID_CHARGER_STAT);

            me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer= MAINAPP_LED_DIM_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_AMP_ERR_CHECK_TIMEOUT].timer = MAINAPP_AMP_ERR_CHECK_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = MAINAPP_STACK_CHECK_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_POWER_SWITCH_CHK_TIMEROUT].timer = MAINAPP_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_SOURCE_CHANGE_TIMEOUT].timer = MAINAPP_START_AUIDO_SWTH_TIMEOUT_IN_MS;
#ifdef HAS_SYSTEM_GAIN_CONTROL
            me->tickHandlers[TIMER_ID_SYS_GAIN_ADJUST_TIMEOUT].timer = MAINAPP_SYS_GAIN_ADJUST_IN_MS;
#endif
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            ret = Q_HANDLED();
            break;
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if(NORMAL_MODE == evt->modeId)
            {
                if(!pIdleDlg)
                {
                    pIdleDlg = &IdleDlg;
                    IdleDlg_Ctor(pIdleDlg, (QActive *)me);
                }
                MainApp_UpdateBattLedStatus(me,e);
                // for bug [IN:013899]
                if(!RingBuf_IsEmpty(&me->btCueRBufObj))
                {
                    uint8 btcmd;
                    RingBuf_Pop(&me->btCueRBufObj, (uint8*)&btcmd, sizeof(uint8));
                    MainApp_SendBTCueCmd(me, (eBtCmd)btcmd);
                }

                MainApp_Mute(me,FALSE);
                ret = Q_HANDLED();
            }
            break;
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            if(me->systemStatus == SYSTEM_STA_STANDBY)
            {
                MainApp_SendToMainApp(me,MAINAPP_ACTIVE_SIG);
                ret = Q_HANDLED();
            }
            else
            {
                me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer = MAINAPP_LED_DIM_TIMEOUT_IN_MS;
                ret = MainApp_KeyHandler(me,e);
                if(ret != Q_UNHANDLED())
                    MainApp_UpdateProdLedStatus(me,e);
            }
            break;
        }
        case BT_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)BT_STATE_SIG", e->sig);
            ret = MainApp_BluetoothEvtHandler(me,e);
            MainApp_UpdateProdLedStatus(me,e);
            break;
        }
        case POWER_BATT_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG", e->sig);
            if (MainApp_IsBatteryRemoved(me) &&
                TRUE == *(bool*)Setting_Get(SETID_IS_DC_PLUG_IN))
            {
                MainApp_SendLedReq(me, LED_IND_ID_BATT_LV_10);
                ret = Q_HANDLED();
                break;
            }
            ret = MainApp_BatteryEvtHandler(me,e);
            // if need to tran, don't update led.
            if(ret == Q_UNHANDLED())
                MainApp_UpdateBattLedStatus(me,e);
            break;
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            ret = Q_HANDLED();
            AudioMusicDetectStateEvt* pAudioMusicStateEvt = (AudioMusicDetectStateEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "AUDIO_MUSIC_STREAM_STATE_SIG:%d", pAudioMusicStateEvt->jackId);
            if(me->isCuePlaying ||
               me->CurrBTStatus == BT_DISCOVERABLE_STA ||
               //workaround for issue [IN:013756]
               me->tickHandlers[TIMER_ID_Connected_cue_TIMEOUT].timer > 0||me->isCueChanel)
            {
                me->isCueChanel = FALSE;
                break;
            }
            MainApp_UpdateAudioChannel(me,(uint8)pAudioMusicStateEvt->jackId);
            MainApp_UpdateProdLedStatus(me,e);
            break;
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            ret = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            break;
        }
        case MAINAPP_ACTIVE_SIG:
        {
            me->systemStatus = SYSTEM_STA_ON;

            me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer= MAINAPP_LED_DIM_TIMEOUT_IN_MS;

            me->currBattLed = 0;

            MainApp_UpdateBattLedStatus(me,e);
            MainApp_UpdateProdLedStatus(me,e);
            ret= Q_HANDLED();
            break;
        }
        case MAINAPP_STANDBY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_STANDBY_SIG", e->sig);
            me->systemStatus = SYSTEM_STA_STANDBY;
            me->currBattLed = 0;

            //because POWER_BATT_STATE_SIG will not comes immediately, so need to update batt led.
            MainApp_UpdateBattLedStatus(me,e);
            MainApp_UpdateProdLedStatus(me,e);
            ret = Q_HANDLED();
            break;
        }
        //Sleep msg sent by IdleDlg. to Trigger Sleep Procedure
        case IDLE_TIMEOUT_SIG:
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)IDLE_TIMEOUT_SIG", e->sig);
        case MAINAPP_SLEEP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_SLEEP_SIG", e->sig);
            me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
            //wait MAINAPP_BATTERY_LED_CHANGE_TIMEOUT_IN_MS ms to let led turn off
            me->tickHandlers[TIMER_ID_SLEEP_TIMEOUT].timer = MAINAPP_SLEEP_TIMEOUT_IN_MS;
            IWDG_ReloadCounter();
            MainApp_SendLedReq(me,LED_IND_ID_ALL_OFF);
            MainApp_Mute(me, TRUE);
            ret = Q_HANDLED();
            break;
        }
        case MAINAPP_SHOP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_SHOP_SIG", e->sig);
            ret = Q_TRAN(&MainApp_Shop);
            break;
        }
        case MAINAPP_SHOW_VERSION_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_SHOW_VERSION_SIG", e->sig);
            MainApp_ShowVersion();
            ret = Q_HANDLED();
            break;
        }
        case MAINAPP_FACTORY_RESET_SIG:
        {
            ret = Q_TRAN(&MainApp_FactoryReset);
            break;
        }
        case MAINAPP_SWITCH_CHANNEL_SIG:
        {
            AudioChannelSwitchReqEvt *pReq = (AudioChannelSwitchReqEvt*)e;
            if(pReq->channel == AUDIO_CHANNEL_AUXIN)
                MainApp_SetChannel(me,AUDIO_CHANNEL_AUXIN);
            else
                MainApp_SetChannel(me,AUDIO_CHANNEL_BT);
            MainApp_UpdateProdLedStatus(me,e);
            break;
        }
        case MAINAPP_SET_AUDIO_SIG:
        {
            AudioSettEvt * evt = (AudioSettEvt*)e;
            AudioSrv_SetAudio(evt->aduioSettId, evt->enable, evt->param, evt->param2);
            if(evt->aduioSettId == DSP_VOLUME_SETT_ID)
                Setting_Set(SETID_VOLUME, &( evt->param));
            break;
        }
#ifdef HAS_IWDG
        case TIMER_FEED_WD_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)TIMER_FEED_WD_SIG", e->sig);
            IWDG_ReloadCounter();
            return Q_HANDLED();
        }
#endif

        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_AMP_ERR_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_POWERING_DOWN_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SLEEP_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_POWER_SWITCH_CHK_TIMEROUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SOURCE_CHANGE_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_CUE_CMD_DELAY_TIMEROUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_LOW_BATTERY_NOTIFY_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_BT_BATT_NOTIFY_TIMEOUT].timer = INVALID_VALUE;
#ifdef HAS_SYSTEM_GAIN_CONTROL
            me->tickHandlers[TIMER_ID_SYS_GAIN_ADJUST_TIMEOUT].timer = INVALID_VALUE;
#endif
            RingBuf_Reset(&me->btCueRBufObj);
            me->isCuePlaying = FALSE;
            me->isBTStreaming = FALSE;
            me->CurrBTStatus = BT_CONNECTABLE_STA;
            me->battStatus = BatteryStatus_NO_BATTERY;
            ret = Q_HANDLED();
            break;
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


/* Powering Down state
 * Purpose: show LED; play cue; mute amp;
 */
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
			//printf("MainApp_PoweringDown(Q_ENTRY_SIG)\n");
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);
            me->systemStatus = SYSTEM_STA_POWERING_DOWN;

            Mainapp_SaveData(me);

            //start led timer to show batt led off
            me->currBattLed = LED_BATTERY_BAR_LV;
            me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = MAINAPP_BATTERY_LED_CHANGE_TIMEOUT_IN_MS;
            MainApp_SendLedReq(me, LED_IND_ID_PROD_TRANS_OFF);

            //BluetoothSrv_SendBtCmd((QActive*)me, BT_DISCONNECT_CMD);

            //wait for play powering off cue
            MainApp_Mute(me,TRUE);

            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if(POWERING_DOWN_MODE == evt->modeId)
            {
                me->tickHandlers[TIMER_ID_POWERING_OFF_TIMEOUT].timer = MAINAPP_POWERING_OFF_TIMEOUT_IN_MS;
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
			//printf("MainApp_PoweringDown(Q_EXIT_SIG)\n");
			TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_POWERING_OFF_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            if(pIdleDlg)
            {
                IdleDlg_Xtor(pIdleDlg);
                pIdleDlg = NULL;
            }
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);

}

/* Sleep state
 * Purpose: let user think system off, and save battery power.
 */
QState MainApp_Sleep(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
			//printf("MainApp_Sleep(Q_ENTRY_SIG)\n");
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, SLEEP_MODE);

            QTimeEvt_disarm(TIME_EVT_OF(me));
            IWDG_ReloadCounter();
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RES", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if(SLEEP_MODE == evt->modeId)
            {
                me->systemStatus = SYSTEM_STA_SLEEP;
                QEvt* pEvt = Q_NEW(QEvt, POWER_MCU_SLEEP_SIG);
                SendToServer(POWER_SRV_ID, pEvt);
                IWDG_ReloadCounter();
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            return Q_HANDLED();
        }
        case DC_IN_INT_SIG:
        case BT_KEY_INT_SIG:
        case POWER_SWITCH_INT_SIG:
        {
            me->currBattLed = 0;
            me->isCriticalTemp = FALSE;
            me->isChargeStopByNTC = FALSE;
            uint8 value = FALSE;
            Setting_Set(SETID_IS_OVER_TEMP,&value);
            Setting_Reset(SETID_IS_PWR_SWITH_ON);
            me->battStatus = BatteryStatus_NO_BATTERY;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)DC_IN_INT_SIG Or etc", e->sig);
            return Q_TRAN(&MainApp_Off);
        }
#ifdef HAS_IWDG
        case TIMER_FEED_WD_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)TIMER_FEED_WD_SIG", e->sig);
            if(me->systemStatus != SYSTEM_STA_SLEEP)
            {
                IWDG_ReloadCounter();
                return Q_HANDLED();
            }
            IWDG_ReloadCounter();
            QEvt* pEvt = Q_NEW(QEvt, POWER_MCU_SLEEP_SIG);
            SendToServer(POWER_SRV_ID, pEvt);
            return Q_HANDLED();
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



/* Soft Reset state*/
QState MainApp_FactoryReset(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SetChannel(me, AUDIO_CHANNEL_BT);
            //BluetoothSrv_SendBtCmd((QActive*)me, BT_DISCONNECT_CMD);
            BluetoothSrv_SendBtCmd((QActive*)me, BT_RESET_PAIR_LIST_CMD);
            MainApp_Mute(me,TRUE);
            //so here should mute it.
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
            me->battStatus = BatteryStatus_NO_BATTERY;

            MainApp_SendLedReq(me, LED_IND_ID_FACTORY_RESET_TRIGGERED);
            me->tickHandlers[TIMER_ID_POWERING_DOWN_TIMEOUT].timer = MAINAPP_FACTORY_POWER_DOWN_TIMEOUT_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);


            return Q_HANDLED();
        }
        case BT_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)BT_STATE_SIG", e->sig);
            // To do :add BT_RESET_PAIR_LIST_CMD response from Bluetooth
            // Need confirm correct response of BT_RESET_PAIR_LIST_CMD
            // If reset fail may need to retry reset BT.
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_POWERING_DOWN_TIMEOUT].timer = INVALID_VALUE;
            return Q_HANDLED();
        }

        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);
}

/*Off charging state,   for power off charging*/
QState MainApp_OffCharging(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
			//printf("MainApp_OffCharging(Q_ENTRY_SIG)\n");
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, OFF_CHARGING_MODE);
            me->systemStatus = SYSTEM_STA_OFF_CHARGING;
            me->battChgTemp_bk =TL_NORMAL;
            me->battDischgTemp_bk =TL_NORMAL;
            PowerSrv_Get((QActive*)me, POWER_GET_ID_CHARGER_STAT);
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TICK_IN_OFF_CHG_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RES", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if(OFF_CHARGING_MODE == evt->modeId)
            {
                if(TRUE == *(bool*)Setting_Get(SETID_ALLOW_POWERUP) &&
                   PWR_SW_ON == *(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON))
                {
                    TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)BATTERY ALREADY ENOUGH", e->sig);
                    return Q_TRAN(&MainApp_PoweringUp);   //battery enough Powering up}
                }
                MainApp_ChargingEnable(me);
                MainApp_UpdateBattLedStatus(me,e);
                PowerSrv_Get((QActive*)me, POWER_GET_ID_CHARGER_STAT);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (PWR_SW_ON == *(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON))
            {
                if(MainApp_IsBatteryRemoved(me))
                {
                    TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)NO BATTERY", e->sig);
                    MainApp_SendLedReq(me, LED_IND_ID_BATT_LV_10);
                    return Q_TRAN(&MainApp_PoweringUp);   //nobattery Powering up
                }
                if(TRUE == *(bool*)Setting_Get(SETID_ALLOW_CHG_PWRUP))
                {
                    TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)BATTERY ENOUGH", e->sig);
                    return Q_TRAN(&MainApp_PoweringUp);   //battery enough Powering up}
                }
            }
            // if remove dc then tran to off
            if (FALSE == *(bool*)Setting_Get(SETID_IS_DC_PLUG_IN))
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)NO DC", e->sig);
                //Save current battery capacity, wait in off state
                Mainapp_SaveData(me);
                //stop charging led display
                me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
                MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);// close led when exit off-charging.
                return Q_TRAN(&MainApp_Off);   //DC plug In change to Off charging state
            }
            break;
        }
        case POWER_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_STATE_SIG", e->sig);
            if(TRUE == *(bool*)Setting_Get(SETID_ALLOW_POWERUP) &&
               PWR_SW_ON == *(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON))
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)BATTERY ALREADY ENOUGH", e->sig);
                return Q_TRAN(&MainApp_PoweringUp);   //battery enough Powering up}
            }
            return Q_HANDLED();
        }
        case POWER_BATT_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG", e->sig);
            //dc in & battery adc to 0 & charger sta error -> no battery
            if(MainApp_IsBatteryRemoved(me))
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)NO BATTERY", e->sig);
                return Q_HANDLED();
            }
            QState trans_state = MainApp_BatteryEvtHandler(me,e);
            MainApp_UpdateBattLedStatus(me,e);
            if(trans_state!=Q_UNHANDLED())
            {
                return trans_state;
            }
            else
            {
                return Q_HANDLED();
            }
        }
        case MAINAPP_SLEEP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_SLEEP_SIG", e->sig);
            me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
            MainApp_SendLedReq(me,LED_IND_ID_ALL_OFF);
            //wait MAINAPP_BATTERY_LED_CHANGE_TIMEOUT_IN_MS ms to let led turn off
            me->tickHandlers[TIMER_ID_SLEEP_TIMEOUT].timer = MAINAPP_SLEEP_TIMEOUT_IN_MS;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_SLEEP_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->isCriticalTemp = FALSE;
            me->isChargeStopByNTC = FALSE;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }

        default:
            break;
    }
    return Q_SUPER(&MainApp_Base);

}





/* For ShopMode
 */
QState MainApp_Shop(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->systemStatus = SYSTEM_STA_ON;

            MainApp_SwitchMode(me, SHOP_MODE);

            me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer= MAINAPP_LED_DIM_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_AMP_ERR_CHECK_TIMEOUT].timer = MAINAPP_AMP_ERR_CHECK_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = MAINAPP_STACK_CHECK_TIMEOUT_IN_MS;

            ret = Q_HANDLED();
            break;
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if(SHOP_MODE == evt->modeId)
            {

                AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, *(uint8*)Setting_Get(SETID_BASS), 0);
                AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, *(uint8*)Setting_Get(SETID_TREBLE), 0);

                MainApp_SendLedReq(me, LED_IND_ID_AUXIN);
                MainApp_SendLedReq(me, LED_IND_ID_BATT_LV_10);

                MainApp_SetVolume(me,me->absoluteVol);
                MainApp_SetChannel(me,AUDIO_CHANNEL_AUXIN);
                MainApp_Mute(me,FALSE);

                MainApp_ChargingDisable(me);
                PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            }
            ret = Q_HANDLED();
            break;
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            ret = Q_HANDLED();
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if(BT_KEY == evt->keyId)
            {
                //HW reset pop noise
                if(evt->keyEvent == KEY_EVT_VERY_LONG_HOLD)
                {
                    AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
                    MainApp_Mute(me,TRUE);
                    me->tickHandlers[TIMER_ID_DELAYED_ERROR_REBOOT].timer = MAINAPP_SHOW_ERR_SLEEP_TIMEROUT_IN_MS;
                }
                break;
            }
            if(me->systemStatus == SYSTEM_STA_STANDBY)
            {
                MainApp_SendToMainApp(me,MAINAPP_ACTIVE_SIG);
            }
            else
            {
                me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer= MAINAPP_LED_DIM_TIMEOUT_IN_MS;
                MainApp_KnobHandler(me,evt);
            }
            break;
        }
        case MAINAPP_ACTIVE_SIG:
        {
            me->systemStatus = SYSTEM_STA_ON;
            me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer = MAINAPP_LED_DIM_TIMEOUT_IN_MS;
            LedSrv_SetPatt((QActive*)me, LED_MASK_BAT_LEDS, TRANS_PREV_2_ON_PAT_RED);
            ret= Q_HANDLED();
            break;
        }
        case MAINAPP_STANDBY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_STANDBY_SIG", e->sig);
            me->systemStatus = SYSTEM_STA_STANDBY;
            //because POWER_BATT_STATE_SIG will not comes immediately, so need to update batt led.
            LedSrv_SetPatt((QActive*)me, LED_MASK_BAT_LEDS, TRANS_PREV_2_ON_PAT_DIM_RED);
            ret = Q_HANDLED();
            break;
        }
        case POWER_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_STATE_SIG", e->sig);
            if(!*(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON))
                MainApp_SetVolume(me,0);
            if(me->systemStatus == SYSTEM_STA_STANDBY)
            {
                MainApp_SendToMainApp(me,MAINAPP_ACTIVE_SIG);
            }
            ret = Q_HANDLED();
            break;
        }
        case MAINAPP_RESTART_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_MCU_RESET_SIG", e->sig);
            uint32 shopModeWord = *(uint32*)Setting_Get(SETID_SHOP_MODE_WORD);
            TP_PRINTF("\r\n MainApp ShopModeWord : %x \r\n",shopModeWord);
            ret = Q_TRAN(MainApp_PoweringDown);
            break;
        }
        case MAINAPP_SET_AUDIO_SIG:
        {
            ret = Q_HANDLED();
            AudioSettEvt * evt = (AudioSettEvt*)e;
            AudioSrv_SetAudio(evt->aduioSettId, evt->enable, evt->param, evt->param2);
            if(evt->aduioSettId == DSP_VOLUME_SETT_ID)
                Setting_Set(SETID_VOLUME, &( evt->param));
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            // To Do: disable timer handler here
            me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer= INVALID_VALUE;
            me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_AMP_ERR_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = INVALID_VALUE;
            ret = Q_HANDLED();
            break;
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


/* LS sample state
 * Purpose: normal state, BTshould work now, no battery
 */
QState MainApp_LS_Sample(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->systemStatus = SYSTEM_STA_ON;
            MainApp_SwitchMode(me, NORMAL_MODE);

            BluetoothSrv_SendBtCmd((QActive*)me, BT_ANSWER_CMD);

            me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer= MAINAPP_LED_DIM_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_AMP_ERR_CHECK_TIMEOUT].timer = MAINAPP_AMP_ERR_CHECK_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = MAINAPP_STACK_CHECK_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_SOURCE_CHANGE_TIMEOUT].timer = MAINAPP_START_AUIDO_SWTH_TIMEOUT_IN_MS;
#ifdef HAS_SYSTEM_GAIN_CONTROL
            me->tickHandlers[TIMER_ID_SYS_GAIN_ADJUST_TIMEOUT].timer = MAINAPP_SYS_GAIN_ADJUST_IN_MS;
#endif

            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            ret = Q_HANDLED();
            break;
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if(NORMAL_MODE == evt->modeId)
            {
                if(!RingBuf_IsEmpty(&me->btCueRBufObj))
                {
                    uint8 btcmd;
                    RingBuf_Pop(&me->btCueRBufObj, (uint8*)&btcmd, sizeof(uint8));
                    MainApp_SendBTCueCmd(me, (eBtCmd)btcmd);
                }
                MainApp_Mute(me,FALSE);
                MainApp_ChargingDisable(me);
                ret = Q_HANDLED();
            }
            break;
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            if(me->systemStatus == SYSTEM_STA_STANDBY)
            {
                MainApp_SendToMainApp(me,MAINAPP_ACTIVE_SIG);
                ret = Q_HANDLED();
            }
            else
            {
                me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer = MAINAPP_LED_DIM_TIMEOUT_IN_MS;
                ret = MainApp_KeyHandler(me,e);
                if(ret != Q_UNHANDLED())
                    MainApp_UpdateProdLedStatus(me,e);
            }
            break;
        }
        case POWER_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_STATE_SIG", e->sig);
            ret = Q_HANDLED();
            if(!*(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON))
            {
                MainApp_SetVolume(me,0);
            }
            if(me->systemStatus == SYSTEM_STA_STANDBY)
            {
                MainApp_SendToMainApp(me,MAINAPP_ACTIVE_SIG);
            }
            break;
        }
        case BT_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)BT_STATE_SIG", e->sig);
            ret = MainApp_BluetoothEvtHandler(me,e);
            MainApp_UpdateProdLedStatus(me,e);
            break;
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            ret = Q_HANDLED();
            AudioMusicDetectStateEvt* pAudioMusicStateEvt = (AudioMusicDetectStateEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "AUDIO_MUSIC_STREAM_STATE_SIG:%d", pAudioMusicStateEvt->jackId);
            if(me->isCuePlaying ||
               me->CurrBTStatus == BT_DISCOVERABLE_STA ||
               //workaround for issue [IN:013756]
               me->tickHandlers[TIMER_ID_SOURCE_CHANGE_TIMEOUT].timer > 0||me->isCueChanel)
            {
                me->isCueChanel = FALSE;
                break;
            }
            MainApp_UpdateAudioChannel(me,(uint8)pAudioMusicStateEvt->jackId);
            MainApp_UpdateProdLedStatus(me,e);
            break;
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            ret = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            break;
        }
        case MAINAPP_ACTIVE_SIG:
        {
            me->systemStatus = SYSTEM_STA_ON;

            me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer= MAINAPP_LED_DIM_TIMEOUT_IN_MS;

            me->currBattLed = 0;

            LedSrv_SetPatt((QActive*)me, LED_MASK_BAT_LEDS, TRANS_PREV_2_ON_PAT_RED);
            MainApp_UpdateProdLedStatus(me,e);
            ret= Q_HANDLED();
            break;
        }
        case MAINAPP_STANDBY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_STANDBY_SIG", e->sig);
            me->systemStatus = SYSTEM_STA_STANDBY;
            me->currBattLed = 0;

            //because POWER_BATT_STATE_SIG will not comes immediately, so need to update batt led.
            LedSrv_SetPatt((QActive*)me, LED_MASK_BAT_LEDS, TRANS_PREV_2_ON_PAT_DIM_RED);
            MainApp_UpdateProdLedStatus(me,e);
            ret = Q_HANDLED();
            break;
        }
        case MAINAPP_FACTORY_RESET_SIG:
        {
            ret = Q_TRAN(&MainApp_FactoryReset);
            break;
        }
        case MAINAPP_SET_AUDIO_SIG:
        {
            ret = Q_HANDLED();
            AudioSettEvt * evt = (AudioSettEvt*)e;
            AudioSrv_SetAudio(evt->aduioSettId, evt->enable, evt->param, evt->param2);
            if(evt->aduioSettId == DSP_VOLUME_SETT_ID)
                Setting_Set(SETID_VOLUME, &( evt->param));
            break;
        }
        case MAINAPP_SWITCH_CHANNEL_SIG:
        {
            AudioChannelSwitchReqEvt *pReq = (AudioChannelSwitchReqEvt*)e;
            if(pReq->channel == AUDIO_CHANNEL_AUXIN)
                MainApp_SetChannel(me,AUDIO_CHANNEL_AUXIN);
            else
                MainApp_SetChannel(me,AUDIO_CHANNEL_BT);
            MainApp_UpdateProdLedStatus(me,e);
            break;
        }
#ifdef HAS_IWDG
        case TIMER_FEED_WD_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)TIMER_FEED_WD_SIG", e->sig);
            IWDG_ReloadCounter();
            return Q_HANDLED();
        }
#endif
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_AMP_ERR_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SLEEP_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_POWER_SWITCH_CHK_TIMEROUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SOURCE_CHANGE_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_CUE_CMD_DELAY_TIMEROUT].timer = INVALID_VALUE;
#ifdef HAS_SYSTEM_GAIN_CONTROL
            me->tickHandlers[TIMER_ID_SYS_GAIN_ADJUST_TIMEOUT].timer = INVALID_VALUE;
#endif
            RingBuf_Reset(&me->btCueRBufObj);
            me->isCuePlaying = FALSE;
            ret = Q_HANDLED();
            break;
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


