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
static QState MainApp_UnmuteDelayTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_LowBatteryNotifyTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_WaitingInOFFTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_SourceChangeTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_StackCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_SysTempCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_AmpErrCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_BTPairingFilterTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_ShowBatteryLedHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_StandByTimeoutHander(cMainApp * const me, QEvt const * const e);
static QState MainApp_SleepTimeoutHander(cMainApp * const me, QEvt const * const e);
static QState MainApp_PoweringOffTimeoutHander(cMainApp * const me, QEvt const * const e);
static QState MainApp_BTToActiveTimeoutHander(cMainApp * const me, QEvt const * const e);

static QState MainApp_PoweringDownTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_PowerSwitchChkTimeoutHander(cMainApp * const me, QEvt const * const e);
static QState MainApp_CueCmdDelayTimeoutHander(cMainApp * const me, QEvt const * const e);
static QState MainApp_BTBattNotifyTimeoutHander(cMainApp * const me, QEvt const * const e);
static QState MainApp_ConnectedCueTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_BatFullCueTimeoutHandler(cMainApp * const me, QEvt const * const e);
#ifdef HAS_SYSTEM_GAIN_CONTROL
static QState MainApp_SystemGainAdjustTimeoutHandler(cMainApp * const me, QEvt const * const e);
#endif


/*****************************************************************
 * Global Variable
 *****************************************************************/
/*  Default value of tick handler list */
static tTickHandlerList tickHandlerlist[TIMER_ID_MAX] =
{
    {0, (TickHandler)MainApp_BtBootingTimeoutHandler},             //TIMER_ID_ASE_TK_BOOTING_TIMEOUT
    {0, (TickHandler)MainApp_DelayedErrorRebootTimeoutHandler},    //TIMER_ID_DELAYED_ERROR_REBOOT
    {0, (TickHandler)MainApp_UnmuteDelayTimeoutHandler},           //TIMER_ID_UNMUTE_DELAY_TIMEOUT
    {0, (TickHandler)MainApp_LowBatteryNotifyTimeoutHandler},      //TIMER_ID_LOW_BATTERY_NOTIFY_TIMEOUT
    {0, (TickHandler)MainApp_WaitingInOFFTimeoutHandler},          //TIMER_ID_WAIT_IN_OFF_TIMEOUT
    {0, (TickHandler)MainApp_SourceChangeTimeoutHandler},          //TIMER_ID_VOLUME_CHANGE_TIMEOUT
    {0, (TickHandler)MainApp_StackCheckTimeoutHandler},            //TIMER_ID_STACK_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_SysTempCheckTimeoutHandler},          //TIMER_ID_SYS_TEMP_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_AmpErrCheckTimeoutHandler},           //TIMER_ID_AMP_ERR_CHECK_TIMEOUT
#ifdef HAS_BT_PAIRING_FILTER
    {0, (TickHandler)MainApp_BTPairingFilterTimeoutHandler},       //TIMER_ID_PAIRING_FILTER_TIMEOUT
#endif
    {0, (TickHandler)MainApp_ShowBatteryLedHandler},               //TIMER_ID_SHOW_BATTERY_LED_TIMEOUT
    {0, (TickHandler)MainApp_StandByTimeoutHander},                //TIMER_ID_STANDBY_TIMEOUT
    {0, (TickHandler)MainApp_SleepTimeoutHander},                  //TIMER_ID_SLEEP_TIMEOUT
    {0, (TickHandler)MainApp_PoweringOffTimeoutHander},            //TIMER_ID_POWERING_OFF_TIMEOUT
    {0, (TickHandler)MainApp_BTToActiveTimeoutHander},             //TIMER_ID_BT_TO_ACTIVE_TIMEOUT
    {0, (TickHandler)MainApp_PoweringDownTimeoutHandler},          //TIMER_ID_POWERING_DOWN_TIMEOUT
    {0, (TickHandler)MainApp_PowerSwitchChkTimeoutHander},         //TIMER_ID_POWER_SWITCH_CHK_TIMEROUT
    {0, (TickHandler)MainApp_CueCmdDelayTimeoutHander},            //TIMER_ID_CUE_CMD_DELAY_TIMEROUT
    {0, (TickHandler)MainApp_BTBattNotifyTimeoutHander},           //TIMER_ID_BT_BATT_NOTIFY_TIMEOUT
    {0, (TickHandler)MainApp_ConnectedCueTimeoutHandler},          //TIMER_ID_Connected_Cue_NOTIFY_TIMEOUT
    {0, (TickHandler)MainApp_BatFullCueTimeoutHandler},          //TIMER_ID_Connected_Cue_NOTIFY_TIMEOUT

#ifdef HAS_SYSTEM_GAIN_CONTROL
    {0, (TickHandler)MainApp_SystemGainAdjustTimeoutHandler},      //TIMER_ID_SYS_GAIN_ADJUST_TIMEOUT
#endif
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

    TP_PRINTF("Tick BTbooting timeout ! cnt:%d \r\n ",me->btReBootCnt);
    if((uint8)me->nextState > SYSTEM_STA_OFF_CHARGING) //Nick++ for not reboot if device going to powering down
    {
      if(me->btReBootCnt < MAX_BT_REBOOT_COUNT)
      {
          me->btReBootCnt++;
          return Q_TRAN(MainApp_PoweringUp);
      }
    }
    me->btReBootCnt = 0;
#ifdef IGNORE_BT_BOOTING_ERROR
    return Q_TRAN(MainApp_Active); //temp supress bt error status
#endif
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_DelayedErrorRebootTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    BSP_SoftReboot(); /* for release build */
    return Q_UNHANDLED(); //do not transit state
}


static QState MainApp_UnmuteDelayTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    TP_PRINTF("MainApp_BTCueStartDelayTimerHandler \r\n");
    MainApp_Mute(me,FALSE);
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_LowBatteryNotifyTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    if(!(*(uint8*)Setting_Get(SETID_IS_DC_PLUG_IN)))
    {
        me->tickHandlers[TIMER_ID_LOW_BATTERY_NOTIFY_TIMEOUT].timer = MAINAPP_LOW_BATTERY_NOTIFY_TIMEOUT_IN_MS;
        MainApp_SendBTCueCmd(me, BT_BAT_LOW_CUE_CMD);
    }
    else
    {
        me->tickHandlers[TIMER_ID_LOW_BATTERY_NOTIFY_TIMEOUT].timer = INVALID_VALUE;
    }
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_WaitingInOFFTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    TYMQP_DUMP_QUEUE_WITH_LOG(me, "OFF");
    PowerSrv_Set((QActive *)me, POWER_SET_ID_SHUT_DOWN, TRUE);
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_PoweringDownTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    return Q_TRAN(&MainApp_PoweringDown);
}
static QState MainApp_PoweringOffTimeoutHander(cMainApp * const me, QEvt const * const e)
{
    //MainApp_Mute(me,TRUE);
    return Q_TRAN(&MainApp_Off);
}

static QState MainApp_BTToActiveTimeoutHander(cMainApp * const me, QEvt const * const e)
{
    TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)BT_STATE_SIG ACTIVE", e->sig);
    if(*(uint32*)Setting_Get(SETID_SHOP_MODE_WORD) == LS_SAMPLE_VALUE)
        return Q_TRAN(&MainApp_LS_Sample);
    else
        return Q_TRAN(&MainApp_Active);
}


static QState MainApp_SourceChangeTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    // postpone source change to aux if cue is playing
    if(me->isCuePlaying)
    {
        me->tickHandlers[TIMER_ID_SOURCE_CHANGE_TIMEOUT].timer = MAINAPP_SOURCE_CHANGE_TIMEOUT_IN_MS;
    }
    else
    {
        TYMQP_DUMP_QUEUE_WITH_LOG(me,"Reset MusicStatus");
        //reset status to trigger detection again.
        Setting_Reset(SETID_MUSIC_STATUS);
    }
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_ConnectedCueTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /*DSP1761 dectect channel is timeout about 6s,so MCU can't switch channel immediately.
      Connected cue will trigger hijack_channel*/
    TYMQP_DUMP_QUEUE_WITH_LOG(me,"Connected Cue is over");
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_BatFullCueTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    if(TRUE == me->isChgComplete)
    {
        MainApp_SendBTCueCmd(me, BT_BAT_FULL_CUE_CMD);
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

static QState MainApp_SysTempCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{

    me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
    //Get information for error
    if(Setting_IsReady(SETID_IS_OVER_TEMP) && *((bool*)Setting_Get(SETID_IS_OVER_TEMP)))
    {
        MainApp_SendLedReq(me, LED_IND_ID_HW_OVER_HEAT);
    }

    return Q_UNHANDLED();
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
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "AMP Fault");
            MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);
            PowerSrv_Set((QActive *)me, POWER_SET_ID_AMP_SHUTDOWN, TRUE); //shutdown amplifier
            me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
        }
        else
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "AMP OK");
            MainApp_UpdateBattLedStatus(me,e);
            PowerSrv_Set((QActive *)me, POWER_SET_ID_AMP_SHUTDOWN, FALSE); //wakeup amplifier
        }
        me->isAmpFault = isAmpFault;
    }
    return Q_UNHANDLED();
}

#ifdef HAS_BT_PAIRING_FILTER
//workaround method to filter BT module extra discoverable
static QState MainApp_BTPairingFilterTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    return Q_UNHANDLED(); //do not transit state
}
#endif


//Only for powering up & down BattLed Showing.
static QState MainApp_ShowBatteryLedHandler(cMainApp * const me, QEvt const * const e)
{

    uint8 batteryLevel = *((uint8*)Setting_Get(SETID_DISPLAY_CAPACITY));
    batteryLevel = MainaApp_BatteryCapcityToLedLv(batteryLevel);

    ledMask eLedMask = LED_MASK_BAT_1;
    ePattern ledPattern = TRANS_OFF_2_ON_PAT_RED;

    switch(me->systemStatus)
    {
        case SYSTEM_STA_POWERING_DOWN:
            eLedMask = eLedMask << --(me->currBattLed);
            ledPattern = TRANS_PREV_2_OFF_PAT;
            if (me->currBattLed < 0)
            {
                me->currBattLed = 0;
                return Q_UNHANDLED();
            }
            break;
        case SYSTEM_STA_POWERING_UP:
        case SYSTEM_STA_WAIT_BT_UP:
            eLedMask = eLedMask << me->currBattLed++;
            if(me->currBattLed > batteryLevel)
            {
                me->currBattLed = batteryLevel;
                return Q_UNHANDLED();
            }
            break;
        case SYSTEM_STA_ON:
            if(!(me->isChgEnable))
            {
                eLedMask = eLedMask << me->currBattLed++;
                if(me->currBattLed > batteryLevel)
                {
                    me->currBattLed = batteryLevel;
                    MainApp_UpdateBattLedStatus(me,NULL);
                    return Q_UNHANDLED();
                }
                break;
            }//if ischarging let it go through
        default:
            if(me->isChgEnable)
            {
                if( me->systemStatus == SYSTEM_STA_STANDBY ||
                    me->systemStatus == SYSTEM_STA_OFF_CHARGING)
                    ledPattern = TRANS_OFF_2_ON_PAT_DIM_RED;
                else
                    ledPattern = TRANS_OFF_2_ON_PAT_RED;

                eLedMask = LED_MASK_BAT_1 << me->currBattLed++;

                if(eLedMask == LED_MASK_BAT_10)
                {
                    me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = MAINAPP_WAITING_FOR_LED_OPER_IN_MS;
                }
                else
                {
                    me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = MAINAPP_BATTERY_LED_CHANGE_TIMEOUT_IN_MS;
                }

                if(eLedMask > LED_MASK_BAT_10)
                {
                    me->currBattLed = 0;
                    eLedMask = LED_MASK_BAT_LEDS ^ LED_MASK_BAT_1;
                    ledPattern = TRANS_PREV_2_OFF_PAT;
                    me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = MAINAPP_WAITING_FOR_LED_OPER_IN_MS;
                }
                //solid led
                if(eLedMask <= LED_MASK_BAT_1)
                {
                    (me->systemStatus == SYSTEM_STA_STANDBY || me->systemStatus == SYSTEM_STA_OFF_CHARGING)?(ledPattern = DIM_PATT_RED):(ledPattern = SOLID_PAT_RED);
                }
                if(me->isChgComplete)
                {
                    eLedMask = LED_MASK_BAT_LEDS;
                    if(me->systemStatus == SYSTEM_STA_OFF_CHARGING ||
                       me->systemStatus == SYSTEM_STA_STANDBY )
                        ledPattern = DIM_PATT_WHITE;
                    else
                        ledPattern = SOLID_PAT_WHITE;
                    me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
                }
                LedSrv_SetPatt((QActive*)me, eLedMask, ledPattern);

                return Q_UNHANDLED();
            }
            else
            {
                MainApp_UpdateBattLedStatus(me,NULL);
                return Q_UNHANDLED();
            }
    }
    me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = MAINAPP_BATTERY_LED_CHANGE_TIMEOUT_IN_MS;

    LedSrv_SetPatt((QActive*)me, eLedMask, ledPattern);
    return Q_UNHANDLED();
}


static QState MainApp_StandByTimeoutHander(cMainApp * const me, QEvt const * const e)
{
    if(me->systemStatus == SYSTEM_STA_ON)
    {
        if(me->CurrBTStatus == BT_DISCOVERABLE_STA)
            me->tickHandlers[TIMER_ID_STANDBY_TIMEOUT].timer= MAINAPP_LED_DIM_TIMEOUT_IN_MS;
        else
            MainApp_SendToMainApp(me,MAINAPP_STANDBY_SIG);
    }
    return Q_UNHANDLED();
}

static QState MainApp_SleepTimeoutHander(cMainApp * const me, QEvt const * const e)
{
    LedSrv_SetPatt((QActive*)me, LED_MASK_ALL_LEDS, OFF_PATT);
    return Q_TRAN(&MainApp_Sleep);
}

static QState MainApp_PowerSwitchChkTimeoutHander(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_POWER_SWITCH_CHK_TIMEROUT].timer = MAINAPP_TIMEOUT_IN_MS;
    if(PWR_SW_OFF == *(bool*)Setting_Get(SETID_IS_PWR_SWITH_ON))
    {
        //waiting for cue finish
        if(me->isCuePlaying)
        {
            //if powering up cue is playing, then set 2s timer tran to powering down.
            if(me->tickHandlers[TIMER_ID_POWERING_DOWN_TIMEOUT].timer <= 0)
            {
                //reset cue buffer to avoid extra cue
                RingBuf_Reset(&me->btCueRBufObj);
                me->tickHandlers[TIMER_ID_POWERING_DOWN_TIMEOUT].timer = MAINAPP_WAITING_FOR_CUE_STOP_IN_MS;
            }
        }
        else
        {
            return Q_TRAN(&MainApp_PoweringDown);   // Powering down
        }
    }
    return Q_UNHANDLED();
}

static QState MainApp_CueCmdDelayTimeoutHander(cMainApp * const me, QEvt const * const e)
{
    uint8 btcmd;
    RingBuf_Pop(&me->btCueRBufObj, (uint8*)&btcmd, sizeof(uint8));
    TYMQP_DUMP_QUEUE_WITH_LOG(me,"getCUEcmd:%d",btcmd);
    BluetoothSrv_SendBtCmd((QActive*)me, (eBtCmd)btcmd);

    return Q_UNHANDLED();
}

static QState MainApp_BTBattNotifyTimeoutHander(cMainApp * const me, QEvt const * const e)
{
#ifdef HAS_BT_BATT_CMD
    MainApp_UpdateBTBattIndicator(me);
#endif
    return Q_UNHANDLED();
}


#ifdef HAS_SYSTEM_GAIN_CONTROL
static QState MainApp_SystemGainAdjustTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_SYS_GAIN_ADJUST_TIMEOUT].timer = MAINAPP_SYS_GAIN_ADJUST_IN_MS;

    if(Setting_IsReady(SETID_IS_DC_PLUG_IN))
    {
        bool fDcPlugIn = *(bool*)Setting_Get(SETID_IS_DC_PLUG_IN);
        if(fDcPlugIn)
        {
            MainApp_AdjustSysGainForDcIn(me);
        }
        else
        {

            MainApp_AdjustSysGainForDcOut(me);
        }
    }
    return Q_UNHANDLED();
}
#endif


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



void MainApp_CleanAllTickHandlerTimer(cMainApp * const me)
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
        *pTimer = INVALID_VALUE;
    }
}
