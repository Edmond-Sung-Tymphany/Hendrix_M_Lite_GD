
/**
*  @file      MainApp_batteryEvtHandler.c
*  @brief     battery event handler of mainApp
*  @author    Colin Chen
*  @date      11-Dec.-2017
*  @copyright Tymphany Ltd.
*  @history:
*     v0.1    Colin Chen  11-Dec-2017      draft
*/

#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "trace.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "tym_qp_lib.h"
#include "MainApp_priv.h"

typedef QState (*BATTERY_HANDLE_FUN)(cMainApp * const me, PowerSrvInfoEvt const * const e);
#ifdef HAS_BATTERY
#ifdef HAS_BATTERY_NTC
static void MainApp_BatteryTempCriticalOp(cMainApp * const me)
{
    me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = INVALID_VALUE;
    me->isCriticalTemp = TRUE;
    me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
    //wait somtime to show critical led pattern
    me->tickHandlers[TIMER_ID_SLEEP_TIMEOUT].timer = MAINAPP_SHOW_ERR_SLEEP_TIMEROUT_IN_MS;
    MainApp_Mute(me, TRUE);
}

static QState MainApp_BatteryNtcDischargeEvtHandle(cMainApp * const me, PowerSrvInfoEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    bool isOverTemp;
    if(Setting_IsReady(SETID_BATTERY_DISCHARGE_TEMP_LEVEL))
    {
        uint8 battDischgTemp = *(uint8*)Setting_Get(SETID_BATTERY_DISCHARGE_TEMP_LEVEL);
        if(battDischgTemp != me->battDischgTemp_bk)
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me,"MainApp BattNtcDisChg Temp Lv : %d \r\n",battDischgTemp);
            switch(battDischgTemp)
            {
                case TL_CRITICAL:
                    MainApp_BatteryTempCriticalOp(me);
                    break;
                case TL_SERIOUS:
                    if(me->battDischgTemp_bk == TL_CRITICAL)
                    {
                        me->isCriticalTemp = FALSE;
                        isOverTemp = TRUE;
                        Setting_Set(SETID_IS_OVER_TEMP,&isOverTemp);
                    }
                    if(me->battDischgTemp_bk == TL_NORMAL)
                    {
                        isOverTemp = TRUE;
                        Setting_Set(SETID_IS_OVER_TEMP,&isOverTemp);
                        me->isChargeStopByNTC = TRUE;
                        MainApp_ChargingDisable(me);
                    }
                    break;
                case TL_NORMAL:
                    if(me->battDischgTemp_bk <= TL_SERIOUS || me->battDischgTemp_bk >= TL_SUBNORMAL)
                    {
                        me->isCriticalTemp = FALSE;
                        me->isChargeStopByNTC = FALSE;
                        isOverTemp = FALSE;
                        Setting_Set(SETID_IS_OVER_TEMP,&isOverTemp);
                    }
                    break;
                case TL_SUBNORMAL:
                    if(me->battDischgTemp_bk == TL_NORMAL)
                    {
                        isOverTemp = TRUE;
                        Setting_Set(SETID_IS_OVER_TEMP,&isOverTemp);
                        me->isChargeStopByNTC = TRUE;
                        MainApp_ChargingDisable(me);
                    }
                    if(me->battDischgTemp_bk == TL_CRITICAL_COLD)
                    {
                        me->isCriticalTemp = FALSE;
                        isOverTemp = TRUE;
                        Setting_Set(SETID_IS_OVER_TEMP,&isOverTemp);
                    }
                    break;
                case TL_CRITICAL_COLD:
                    MainApp_BatteryTempCriticalOp(me);
                    break;
            }
            me->battDischgTemp_bk = battDischgTemp;
        }

    }
    return ret;
}

static QState MainApp_BatteryNtcChargeEvtHandle(cMainApp * const me, PowerSrvInfoEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    bool isOverTemp;
    if(Setting_IsReady(SETID_BATTERY_CHARGE_TEMP_LEVEL))
    {
        uint8 battChgTemp = *(uint8*)Setting_Get(SETID_BATTERY_CHARGE_TEMP_LEVEL);
        if(battChgTemp != me->battChgTemp_bk)
        {
            switch(battChgTemp)
            {
                case TL_NORMAL:
                    if(me->battChgTemp_bk == TL_SERIOUS ||me->battChgTemp_bk == TL_CRITICAL_COLD )
                    {
                        TYMQP_DUMP_QUEUE_WITH_LOG(me,"Temp return,enable");
                        me->isChargeStopByNTC = FALSE;
                        //MainApp_ChargingEnable(me);
                        me->isCriticalTemp = FALSE;
                        isOverTemp = FALSE;
                        Setting_Set(SETID_IS_OVER_TEMP,&isOverTemp);
                    }
                    break;
                case TL_SERIOUS:
                case TL_CRITICAL_COLD:
                    //Stop charging and flash led but not shutting down
                    isOverTemp = TRUE;
                    Setting_Set(SETID_IS_OVER_TEMP,&isOverTemp);
                    me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
                    MainApp_ChargingDisable(me);
                    me->isChargeStopByNTC = TRUE;
                    break;
            }
            me->battChgTemp_bk = battChgTemp;
        }
    }
    return ret;
}
#endif


static QState MainApp_BatteryCommonEvtHandle(cMainApp * const me, PowerSrvInfoEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    if(Setting_IsReady(SETID_IS_DC_PLUG_IN))
    {
        if(*(bool*)Setting_Get(SETID_IS_DC_PLUG_IN))
        {
            //Keep isChgEnable reseted by ChargingDisable function & system re-init
            //Hot restart shall not trigger charing cue again.
            if((!me->isChgEnable)&&(me->isChargeStopByNTC == FALSE))
            {
                if(me->CurrBTStatus != BT_STREAMING_A2DP_STA)
                    AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME); //Nick++ to prevent audio cue volume not set
                MainApp_SendBTCueCmd(me, BT_CHARGING_CUE_CMD);
            }
            MainApp_ChargingEnable(me);
        }
        else
        {
            MainApp_ChargingDisable(me);
        }
    }
    return ret;
}

static QState MainApp_BatteryCapcityEvtHandle(cMainApp * const me, PowerSrvInfoEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->batteryInfo.chargerState)
    {
        case  CHARGER_STA_CHARGING_DONE:
            if(FALSE == me->isChgComplete)
            {
                me->tickHandlers[TIMER_ID_BAT_FULL_CUE_TIMEOUT_TIMEOUT].timer = MAINAPP_BatFullCue_TIMEOUT_IN_MS;
                Mainapp_SaveData(me);
            }
            me->isChgComplete = TRUE;
            break;
        case  CHARGER_STA_CHARGING:
            me->isChgComplete = FALSE;
            me->tickHandlers[TIMER_ID_LOW_BATTERY_NOTIFY_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_POWERING_DOWN_TIMEOUT].timer = INVALID_VALUE;
            //reset batt status also while wakeup from sleep
            me->battStatus = BatteryStatus_NO_BATTERY;
            break;
        case  CHARGER_STA_ERROR:
            me->isChgComplete = FALSE;
            break;
        default:
            break;
    }
    if(!Setting_IsReady(SETID_BATTERY_STATUS) && !Setting_IsReady(SETID_IS_DC_PLUG_IN))
    {
        return ret;
    }
    BatteryStatus currBattStatus = (BatteryStatus)*(uint8*)Setting_Get(SETID_BATTERY_STATUS);
    if(currBattStatus != me->battStatus &&
       FALSE == *(bool*)Setting_Get(SETID_IS_DC_PLUG_IN))
    {
        switch(currBattStatus)
        {
            case BatteryStatus_NO_BATTERY:
                break;
            case BatteryStatus_LEVEL_CRITICAL:
                if(!me->isChgEnable)
                {
                    //waiting for battery cue play
                    me->tickHandlers[TIMER_ID_POWERING_DOWN_TIMEOUT].timer = MAINAPP_BATT_CUE_PLAY_TIMEOUT_IN_MS;
                    me->tickHandlers[TIMER_ID_LOW_BATTERY_NOTIFY_TIMEOUT].timer = INVALID_VALUE;
                    MainApp_SendBTCueCmd(me, BT_BAT_LOW_CUE_CMD);
                }
                break;
            case BatteryStatus_LEVEL_LOW:
            case BatteryStatus_LEVEL_MID:
                if(!me->isChgEnable &&
                   me->tickHandlers[TIMER_ID_LOW_BATTERY_NOTIFY_TIMEOUT].timer <= 0)
                {
                    //first cue is comes 30s later to avoid emit cue while just start up
                    me->tickHandlers[TIMER_ID_LOW_BATTERY_NOTIFY_TIMEOUT].timer = MAINAPP_DELAY_BATT_NOTIFY_TIMEOUT_IN_MS;
                }
                break;
            case BatteryStatus_LEVEL_EXTRA:
                break;
            case BatteryStatus_LEVEL_HIGH:
                break;
            default:
                break;
        }
        me->battStatus = currBattStatus;
		//Setting_Set(SETID_BATTERY_STATUS, &currBattStatus);		//edmond_20210508
    }
#ifdef HAS_BT_BATT_CMD
    uint8 BTBattStatus = *(uint8*)Setting_Get(SETID_BT_BATT_STATUS);
    if(me->btBattStatus != BTBattStatus)
    {
        me->btBattStatus = BTBattStatus;
        me->tickHandlers[TIMER_ID_BT_BATT_NOTIFY_TIMEOUT].timer = MAINAPP_BT_BATT_NOTIFY_TIMEOUT_IN_MS;
    }
#endif
    return ret;
}


BATTERY_HANDLE_FUN BatteryEvtHandleMap[]=
{
#ifdef HAS_BATTERY_NTC
    &MainApp_BatteryNtcDischargeEvtHandle, // run dischage evt process first it have a higher priority
    &MainApp_BatteryNtcChargeEvtHandle,
#endif
    &MainApp_BatteryCommonEvtHandle,
    &MainApp_BatteryCapcityEvtHandle,
};

uint8 BatteryEvtHandleMaxNum = ArraySize(BatteryEvtHandleMap);
#endif

QState MainApp_BatteryEvtHandler(cMainApp * const me, QEvt const * const e)
{

    QState ret = Q_UNHANDLED();
#ifdef HAS_BATTERY
    PowerSrvInfoEvt *evt = (PowerSrvInfoEvt*)e;
    uint8 i;
    for(i=0; i<BatteryEvtHandleMaxNum; i++)
    {
        ret = BatteryEvtHandleMap[i](me,evt);
        if(ret != Q_UNHANDLED())
            break;
    }
#endif
    return ret;
}

