/**
*  @file      MainApp_tickEvtHandler.c
*  @brief     tick event handler of mainApp
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
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "IdleDlg.h"
#include "tym_qp_lib.h"
#include "MainApp_priv.h"

static QState MainApp_BtBootingTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DelayedErrorRebootTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_FactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_PowerDownTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_VolumeChangeTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_StackCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_AmpErrCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DoublePressTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_SemiActiveTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_SourceSwitchTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_UnmuteSysTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_BtCombKeyTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_CommPingTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_AudioCueTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_CommWdgTimeourtHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_VolumeSyncTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_BtSyncTimerHandler(cMainApp * const me, QEvt const * const e);

#define CRITICAL_TEMP   (90)

/*****************************************************************
 * Global Variable
 *****************************************************************/
/*  Default value of tick handler list */
static tTickHandlerList tickHandlerlist[TIMER_ID_MAX] =
{
    {0, (TickHandler)MainApp_BtBootingTimeoutHandler},    //TIMER_ID_ASE_TK_BOOTING_TIMEOUT
    {0, (TickHandler)MainApp_DelayedErrorRebootTimeoutHandler},    //TIMER_ID_DELAYED_ERROR_REBOOT
    {0, (TickHandler)MainApp_UpgradeTimeoutTimerHandler},    //TIMER_ID_UPGRADE_TIMEOUT
    {0, (TickHandler)MainApp_FactoryResetTimeoutHandler},    //TIMER_ID_FACTORY_RESET_TIMEOUT
    {0, (TickHandler)MainApp_PowerDownTimeoutHandler},       //TIMER_ID_POWER_DOWN_TIMEOUT
    {0, (TickHandler)MainApp_VolumeChangeTimeoutHandler},    //TIMER_ID_VOLUME_CHANGE_TIMEOUT
    {0, (TickHandler)MainApp_StackCheckTimeoutHandler},      //TIMER_ID_STACK_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_AmpErrCheckTimeoutHandler},     //TIMER_ID_AMP_ERR_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_DoublePressTimeoutHandler},     //TIMER_ID_DOUDBLE_PRESS_TIMEOUT
    {0, (TickHandler)MainApp_SemiActiveTimeoutHandler},      //TIMER_ID_SEMI_ACTIVE_TIMEOUT
    {0, (TickHandler)MainApp_SourceSwitchTimerHandler},      //TIMER_ID_SOURCE_SWITCH_TIMEOUT
    {0, (TickHandler)MainApp_UnmuteSysTimerHandler},         //TIMER_ID_UNMUTE_SYS_TIMEOUT
    {0, (TickHandler)MainApp_BtCombKeyTimerHandler},         //TIMER_ID_BT_COMB_KEY_TIMEOUT
    {0, (TickHandler)MainApp_CommPingTimerHandler},          //TIMER_ID_COMM_PING_INTERVAL
    {0, (TickHandler)MainApp_AudioCueTimerHandler},          //TIMER_ID_AUDIO_CUE_TIMEOUT
    {0, (TickHandler)MainApp_CommWdgTimeourtHandler},        //TIMER_ID_COMM_WDG_TIMEOUT
    {0, (TickHandler)MainApp_VolumeSyncTimerHandler},        //TIMER_ID_VOLUME_SYNC_TIMEOUT
    {0, (TickHandler)MainApp_BtSyncTimerHandler},            //TIMER_ID_BT_SYNC_TIMEOUT
};

/*****************************************************************
 * Function Implemenation
 *****************************************************************/
tTickHandlerList* MainApp_GetTickHandlerList(void)
{
    return tickHandlerlist;
}

static QState MainApp_BtBootingTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if sam is booting time out. */
    //Nvm_StoreExceptionCode(REBOOT_CODE_BT_BOOTING_TIMEOUT);
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_DelayedErrorRebootTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    BSP_SoftReboot(); /* for release build */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if system is upgrading time out. */
    //Nvm_StoreExceptionCode(REBOOT_CODE_UPGRADE_TIMEOUT);
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_FactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_PowerDownTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_VolumeChangeTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_VOLUME_CHANGE_TIMEOUT].timer = MAINAPP_VOLUME_CHANGE_TIMEOUT_IN_MS;
    if (me->isVolChanged == TRUE)
    {
        // To Do: set volume here
        me->isVolChanged = FALSE;
    }
    return Q_UNHANDLED(); //do not transit state
}

//void stack_overflow_test(int level)
//{
//    char ch[100];
//    ch[0]= level;
//    TP_PRINTF("stack_overflow_test(i=%d)\r\n", ch[0]);
//    if(level>0)
//    {
//        stack_overflow_test(level-1);
//    }
//}

static QState MainApp_StackCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = MAINAPP_STACK_CHECK_TIMEOUT_IN_MS;
    bool stackOverflow= stackOverflowCheck();
    ASSERT(!stackOverflow);
    if( stackOverflow )
    {
        TP_PRINTF("\r\n\r\n\r\n *** ERROR: stack overflow ***\r\n\r\n\r\n\r\n");
        MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    }

    uint32 stack_max_usage= stackMaxUsage();
    const uint32 stack_size= stackSize();
    uint32 stack_usage_percent= stack_max_usage*100/stack_size;
    // To Do: Set MAX STACK USAGE here
    TP_PRINTF("\r\nmax stack usage: %d bytes / %d bytes (%d%%)\r\n\r\n", stack_max_usage, stack_size, stack_usage_percent);
    if(stack_usage_percent>=70)
    {
        //debug build
        TP_PRINTF("\r\n\r\n\r\n*** WARNING: stack usage trigger 70%, please increase stack size\r\n\r\n\r\n");
        ASSERT(0);
    }

//    //test stack overflow
//    static int level=20;
//    stack_overflow_test(level++);
//    stack_max_usage= stackMaxUsage();
//    stack_usage_percent= stack_max_usage*100/stack_size;
//    TP_PRINTF("new stack usage: %d bytes / %d bytes (%d%%)\r\n\r\n", stack_max_usage, stack_size, stack_usage_percent);

    /* print QP queue status */
    DynamicAnalysis();
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_AmpErrCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    //Amplifier health checking

    bool isAmpFault= FALSE;

    me->tickHandlers[TIMER_ID_AMP_ERR_CHECK_TIMEOUT].timer = MAINAPP_AMP_ERR_CHECK_TIMEOUT_IN_MS;

    if(Setting_IsReady(SETID_IS_AMP_FAULT))
        isAmpFault  = *((bool*)Setting_Get(SETID_IS_AMP_FAULT));
    else
        return Q_UNHANDLED();


    if (me->isAmpFault != isAmpFault)
    {
        if (isAmpFault)
        {
            TP_PRINTF("\r\n\r\n\r\n*** Amplifier failture shutdown!!! ***\r\n\r\n\r\n");
            AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, TRUE, NULL, NULL); //shutdown amplifier
        }
        else
        {
            TP_PRINTF("\r\n\r\n\r\n*** Wake up Amplifier ***\r\n\r\n\r\n");
            AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, FALSE, NULL, NULL); //wakeup amplifier
        }
        me->isAmpFault = isAmpFault;
    }

    if (me->isAmpFault)
    {
        MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);
    }
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_DoublePressTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /* Note: The function will be performed if double press is NOT being triggered within 0.5second after first press. */
    // To Do: operation of double press time out
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_SemiActiveTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    if (me->connState == Proto_BtState_ConnState_PAIRING)
    {
        MainApp_ResetSemiActiveTimer(me);
    }
    else
    {
        MainApp_UpdateSystemStatus(me, SYSTEM_STA_SEMI_ACTIVE);
    }
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_SourceSwitchTimerHandler(cMainApp * const me, QEvt const * const e)
{
    eAudioChannel audioChannel = me->sourceHandler[me->audioSource].audioChannel;
    AudioSrv_SetChannel((QActive *)me, audioChannel);
    Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
    me->tickHandlers[TIMER_ID_UNMUTE_SYS_TIMEOUT].timer = MAINAPP_UNMUTE_SYS_TIMEOUT;
    if (me->audioSource == AUDIO_SOURCE_BT &&
        (me->connState != Proto_BtState_ConnState_CONNECTED &&
         me->connState != Proto_BtState_ConnState_A2DPSTREAMING))
    {
        bool param = TRUE;
        BluetoothSrv_SendBtCmdWithParam((QActive*)me, BT_ENTER_RECONNECT_REQ, sizeof(bool),&param);
    }
    else if (me->audioSource != AUDIO_SOURCE_BT)
    {
        bool param = FALSE;
        BluetoothSrv_SendBtCmdWithParam((QActive*)me, BT_ENTER_PAIRING_REQ,sizeof(param) ,&param);
        BluetoothSrv_SendBtCmdWithParam((QActive*)me, BT_ENTER_RECONNECT_REQ, sizeof(bool),&param);
    }

    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_UnmuteSysTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* Unmute system after switch audio channel.*/
    AudioSrv_SendMuteReq((QActive *)me, AUDIO_SOURCE_MUTE, FALSE);
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_BtCombKeyTimerHandler(cMainApp * const me, QEvt const * const e)
{
    if (me->combKey & COM_KEY_ON_HOLD)
    {
        return Q_UNHANDLED(); //do not transit state
    }

    if (me->audioSource == AUDIO_SOURCE_BT )
    {
        switch(me->combKey)
        {
            case PLAY_PAUSE_COMB:
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PLAY_PAUSE_REQ);
                break;
            }
            case PREV_TRACK_COMB:
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PREV_REQ);
                break;
            }
            case NEXT_TRACK_COMB:
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_NEXT_REQ);
                break;
            }
            default:
                break;
        }

        me->combKey = COM_KEY_INVALID;

    }
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_CommPingTimerHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_COMM_PING_INTERVAL].timer = MAINAPP_COMM_PING_INTERVAL_IN_MS;
    /* Ping BT module: system is booted. */
    BluetoothSrv_SendBtCmd((QActive*)me, BT_COMM_PING_REQ);
    if (me->tickHandlers[TIMER_ID_COMM_WDG_TIMEOUT].timer == 0)
    {
        me->tickHandlers[TIMER_ID_COMM_WDG_TIMEOUT].timer = MAINAPP_COMM_WGD_TIMEOUT_IN_MS;
    }
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_CommWdgTimeourtHandler(cMainApp * const me, QEvt const * const e)
{
#ifdef HAS_COMM_WATCHDOG
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
#endif
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_AudioCueTimerHandler(cMainApp * const me, QEvt const * const e)
{
    if (me->isCuePlaying)
    {
        if (me->systemStatus == SYSTEM_STA_POWERING_DOWN)
        {
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_SOURCE_MUTE, TRUE);
        }
        else if (me->audioSource != AUDIO_SOURCE_BT && me->isCuePlaying == TRUE)
        {
            MainApp_SwitchAudioSource(me, me->audioSource, TRUE);
        }
        me->isCuePlaying = FALSE;
        AudioSrv_SetVolume(me->vol/CLICK_NUM_PER_VOLUME_STEP);
    }
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_VolumeSyncTimerHandler(cMainApp * const me, QEvt const * const e)
{
    static uint32 vol;
    if (!(me->vol%CLICK_NUM_PER_VOLUME_STEP))
    {
        vol = me->vol/CLICK_NUM_PER_VOLUME_STEP;
        BluetoothSrv_SendBtCmdWithParam((QActive*)me, MCU_VOLUME_CHANGE_EVENT, sizeof(vol), (uint8 *)&vol);
    }
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_BtSyncTimerHandler(cMainApp * const me, QEvt const * const e)
{
    if (me->isCuePlaying)
    {
        me->tickHandlers[TIMER_ID_BT_SYNC_TIMEOUT].timer = MAINAPP_BT_SYNC_TIMEOUT_IN_MS;
    }
    else
    {
        /* sync BT info here */
        BluetoothSrv_SendBtCmdWithParam((QActive*)me, MCU_SOURCE_CHANGE_EVENT, sizeof(eAudioSource), (uint8 *)&me->audioSource);
    }
    return Q_UNHANDLED(); //do not transit state
}


QState MainApp_ActiveTickEvtHandler(cMainApp * const me, QEvt const * const e)
{
    uint8 i = 0;
    int32 *pTimer = NULL;
    for(i = 0; i < TIMER_ID_MAX; i++)
    {
        pTimer = &(me->tickHandlers[i].timer);
        if(*pTimer <= 0)
        {
            continue;
        }
        *pTimer -= MAINAPP_TIMEOUT_IN_MS;
        if(0 >= (*pTimer))
        {
            if(me->tickHandlers[i].timerHandler)
            {
                /* If any tick want to transit state, we handle state transit first,
                 * and pospone next tick handling
                 */
                QState trans_state= me->tickHandlers[i].timerHandler(me, e);
                if(trans_state!=Q_UNHANDLED())
                {
                    return trans_state;
                }
            }
            else
            {
                ASSERT(0);
            }
        }
    }

    return Q_UNHANDLED();
}

