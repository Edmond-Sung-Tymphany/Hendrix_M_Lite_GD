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
#include "MainApp_pteCmdHandler.h"
#include "MainApp.Config"
#include "IdleDlg.h"
#include "bl_common.h"
#include "fep_addr.h"


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
            me->isBtBooted = FALSE;
            me->isAudioDrvReady = FALSE;
            /* update system satus and timer */
            MainApp_UpdateSystemStatus(me, SYSTEM_STA_OFF);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
            me->vol = DEFAULT_VOLUME*CLICK_NUM_PER_VOLUME_STEP;

            bool autoBoot = MainApp_IsAutoBoot(me);
            if (autoBoot)
            {
                return Q_TRAN(&MainApp_BtModuleBooting);
            }
            else
            {
                return Q_HANDLED();
            }
        }
        case POWER_STATE_SIG:
        {
            if (Setting_IsReady(SETID_IS_PWR_SWITH_ON))
            {
                bool onOffSta = *(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON);
                if(onOffSta == PWR_SW_ON)
                {
                    return Q_TRAN(&MainApp_BtModuleBooting);
                }
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
            me->connState = Proto_BtState_ConnState_CONNECTABLE;
            MainApp_SwitchMode(me, WAIT_MODE);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( WAIT_MODE != evt->modeId )   //ignore unexpected SYSTEM_MODE_RESP_SIG
            {
                //ASSERT(0);
            }
            else
            {
                MainApp_UpdateSystemStatus(me, SYSTEM_STA_POWERING_UP);
                return Q_TRAN(&MainApp_PoweringUp);
            }
        }
        case BT_STATE_SIG:
        {
            Proto_Tym_BtMcuMessage *pMessage = (Proto_Tym_BtMcuMessage *)((BtRxEvt *)e)->data;
            if (pMessage->which_OneOf == Proto_Tym_BtMcuMessage_btMcuEvent_tag && pMessage->OneOf.btMcuEvent.type == Proto_BtMcu_Event_Type_BOOTED)
            {
                return Q_TRAN(&MainApp_PoweringUp);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
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
            me->isAudioDrvReady = FALSE;
            PowerSrv_Set((QActive *)me, POWER_SET_ID_IOE_POWER, TRUE);
            uint32 workMode = *(uint32 *)Setting_Get(SETID_WORK_MODE);
            if (workMode == WORK_MODE_ID_SHOP)
            {
                PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_POWER_UP_IN_SHOPMODE_TIMEOUT);
            }
            else
            {
                PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_POWER_UP_CUE_TIMEOUT);
            }
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( POWERING_UP_MODE != evt->modeId )   //ignore unexpected SYSTEM_MODE_RESP_SIG
            {
                //ASSERT(0);
            }
            else
            {
                me->isAudioDrvReady = TRUE;
                if (me->isBtBooted)
                {
                    TP_PRINTF("###000 Notify BT module: system is booted.\r\n");
                    /* Notify BT module: system is booted. */
                    BluetoothSrv_SendBtCmd((QActive*)me, MCU_SYSTEM_BOOTED);
                }

                uint32 workMode = *(uint32 *)Setting_Get(SETID_WORK_MODE);
                if (workMode == WORK_MODE_ID_SHOP)
                {
                    /* Do NOT need to power on audio cue in shop mode */
                    MainApp_SwitchAudioSource(me, me->audioSource, TRUE);
                }
            }
            return Q_HANDLED();
        }
        case BT_STATE_SIG:
        {
            QState ret = Q_UNHANDLED();
            uint32 workMode = *(uint32 *)Setting_Get(SETID_WORK_MODE);
            if (workMode == WORK_MODE_ID_SHOP)
            {
                /* Filter some events in Shop mode */
                ret = MainApp_BtEvtFilterInShopMode(me,e);
            }
            if (ret == Q_UNHANDLED())
            {
                MainApp_BluetoothEvtHandler(me,e);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            BluetoothSrv_SendBtCmd((QActive*)me, BT_GET_ADDRESS_REQ);
            MainApp_SwitchAudioSource(me, me->audioSource, TRUE);
            return Q_TRAN(&MainApp_Active);
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
            me->combKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, NORMAL_MODE);
            me->isCuePlaying = FALSE;
            /* update system satus and timer for LED indication powering on. */
            MainApp_UpdateSystemStatus(me, SYSTEM_STA_ACTIVE);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            me->tickHandlers[TIMER_ID_AMP_ERR_CHECK_TIMEOUT].timer = MAINAPP_AMP_ERR_CHECK_TIMEOUT_IN_MS;
            me->tickHandlers[TIMER_ID_COMM_PING_INTERVAL].timer = MAINAPP_COMM_PING_INTERVAL_IN_MS;
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
                pIdleDlg = &IdleDlg;
                IdleDlg_Ctor(pIdleDlg, (QActive *)me);
                /* Set volume, bass and treble */
                AudioSrv_SetVolume(me->vol/CLICK_NUM_PER_VOLUME_STEP);
                AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, me->bass/CLICK_NUM_PER_BASS_STEP, NULL);
                AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, me->treble/CLICK_NUM_PER_BASS_STEP, NULL);
                BluetoothSrv_SendBtCmd((QActive *)me, BT_FIRMWARE_VERSION_REQ);
                me->tickHandlers[TIMER_ID_BT_SYNC_TIMEOUT].timer = MAINAPP_BT_SYNC_TIMEOUT_IN_MS;
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
            /* update system satus and timer */
            MainApp_UpdateSystemStatus(me, SYSTEM_STA_ACTIVE);
            uint32 workMode = *(uint32 *)Setting_Get(SETID_WORK_MODE);
            /* Do NOT response on key event in Shop mode */
            if (workMode != WORK_MODE_ID_SHOP)
            {
                ret = MainApp_KeyHandler(me,e);
            }
            break;
        }
        case POWER_STATE_SIG:
        {
            uint32 workMode = *(uint32 *)Setting_Get(SETID_WORK_MODE);
            /* Do NOT response on key event in Shop mode */
            if (workMode != WORK_MODE_ID_SHOP)
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
            uint32 workMode = *(uint32 *)Setting_Get(SETID_WORK_MODE);
            if (workMode == WORK_MODE_ID_SHOP)
            {
                /* Filter some events in Shop mode */
                ret = MainApp_BtEvtFilterInShopMode(me,e);
            }
            if (ret == Q_UNHANDLED())
            {
                ret = MainApp_BluetoothEvtHandler(me,e);
            }
            break;
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            ret = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            break;
        }
        case IDLE_TIMEOUT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)IDLE_TIMEOUT_SIG", e->sig);
            uint32 workMode = *(uint32 *)Setting_Get(SETID_WORK_MODE);
            /* Do NOT goto standby in Shop mode */
            if (workMode != WORK_MODE_ID_SHOP)
            {
                return Q_TRAN(&MainApp_Standby);
            }
        }
        case MAINAPP_PTE_TEST_CMD_SIG:
        {
            MainApp_PteCmdHandler(me,e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            if(pIdleDlg)
            {
                IdleDlg_Xtor(pIdleDlg);
                pIdleDlg = NULL;
            }
            me->tickHandlers[TIMER_ID_AUDIO_CUE_TIMEOUT].timer = 0;
            me->tickHandlers[TIMER_ID_COMM_PING_INTERVAL].timer = 0;
            /* Reset communication watchdog timer */
            me->tickHandlers[TIMER_ID_COMM_WDG_TIMEOUT].timer = 0;
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


/* Powering Down state
 * Purpose: show LED
 */
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e)
{
    static uint8 powerDownSteps = 0;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);
            MainApp_UpdateSystemStatus(me, SYSTEM_STA_POWERING_DOWN);
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
                powerDownSteps |= POWER_KEY_RELEASE;
            }
            break;
        }
        case BT_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            Proto_Tym_BtMcuMessage *pMessage = (Proto_Tym_BtMcuMessage *)((BtRxEvt *)e)->data;
            if ((pMessage->which_OneOf == Proto_Tym_BtMcuMessage_btMcuEvent_tag &&
                 (pMessage->OneOf.btMcuEvent.type == Proto_BtMcu_Event_Type_BT_AUDIO_CUE_PLAY_START
                  || pMessage->OneOf.btMcuEvent.type == Proto_BtMcu_Event_Type_BT_AUDIO_CUE_PLAY_STOP)) ||
                (pMessage->which_OneOf == Proto_Tym_BtMcuMessage_btMcuReq_tag &&
                 pMessage->OneOf.btMcuReq.type == Proto_BtMcu_ReqResp_BT_AUDIO_CUE_PLAY))
            {
                MainApp_BluetoothEvtHandler(me,e);
            }
            break;
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (powerDownSteps & POWER_OFF_CMD_SENT)
            {
                powerDownSteps |= POWER_OFF_TIMEOUT;
            }
            else if (me->isCuePlaying)
            {
                /* There is another audio cue playing before play power off audio cue, so need to wait more time. */
                PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            }
            else
            {
                BluetoothSrv_SendBtCmd((QActive *)me, BT_PWR_OFF_CMD);
                powerDownSteps |= POWER_OFF_CMD_SENT;
                PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_POWER_DOWN_TIMEOUT_IN_MS);
            }
            break;
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

    if (powerDownSteps == POWER_DOWN_DONE)
    {
        powerDownSteps = 0;
        return Q_TRAN(&MainApp_Off);
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
            MainApp_SwitchMode(me, STANDBY_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RES", e->sig);
            /* update system satus and timer */
            MainApp_UpdateSystemStatus(me, SYSTEM_STA_STANDBY);
            BluetoothSrv_SendBtCmd((QActive *)me, MCU_SYSTEM_STANDBY);
            return Q_HANDLED();
        }
        case POWER_STATE_SIG:
        {
            bool onOffSta = *(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON);
            if(onOffSta == PWR_SW_ON)
            {
                MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                return Q_TRAN(&MainApp_PoweringDown);
            }
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if ((PLAY_PAUSE_KEY == evt->keyId) || (INPUT_KEY == evt->keyId) ||
                (VOLUME_UP_KEY == evt->keyId) || (VOLUME_DOWN_KEY == evt->keyId) ||
                (BASS_UP_KEY == evt->keyId) || (BASS_DOWN_KEY == evt->keyId)||
                (TREBLE_UP_KEY == evt->keyId) || (TREBLE_DOWN_KEY == evt->keyId) ||
                ((POWER_KEY == evt->keyId) && (KEY_EVT_SHORT_PRESS == evt->keyEvent)))
            {
                /* update system satus and timer for LED indication powering on. */
                MainApp_UpdateSystemStatus(me, SYSTEM_STA_POWERING_UP);
                return Q_TRAN(&MainApp_PoweringUp);
            }
            break;
        }
        case BT_STATE_SIG:
        {
            Proto_Tym_BtMcuMessage *pMessage = (Proto_Tym_BtMcuMessage *)((BtRxEvt *)e)->data;
            if (pMessage->which_OneOf == Proto_Tym_BtMcuMessage_btMcuEvent_tag && pMessage->OneOf.btMcuEvent.type == Proto_BtMcu_Event_Type_BT_A2DP_CONNECTED)
            {
                /* update system satus and timer for LED indication powering on. */
                MainApp_UpdateSystemStatus(me, SYSTEM_STA_POWERING_UP);
                return Q_TRAN(&MainApp_PoweringUp);
            }
            break;
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
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
            BSP_SoftReboot();
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

/* Soft Reset state*/
QState MainApp_FactoryReset(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
            BluetoothSrv_SendBtCmd((QActive*)me, BT_FACTORY_RESET_REQ);
            MainApp_FactoryResetSettings(me);
            MainApp_SendLedReq(me, LED_IND_ID_FACTORY_RESET_TRIGGERED);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_DELAY_BEFORE_SOFTRESET_IN_MS);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RES", e->sig);
            return Q_TRAN(&MainApp_SoftReset);
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

/* Firmware update state*/
QState MainApp_FirmwareUpdate(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
            MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
            MainApp_SendLedReq(me, LED_IND_ID_SW_UPDATING);
            BluetoothSrv_SendBtCmd((QActive*)me, BT_MCU_FIRMWARE_UPDATE_REQ_RESP);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_DELAY_BEFORE_JUMP_TO_STBL_IN_MS);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG", e->sig);
            bl_jumpAddr(FEP_ADDR_ISP);
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


