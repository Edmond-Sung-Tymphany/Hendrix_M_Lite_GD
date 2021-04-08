/**
*  @file      MainApp_util.c
*  @brief     util function for BnO mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "trace.h"
#include "controller.h"
#include "Setting_id.h"
#include "LedSrv.h"
#include "SettingSrv.h"
#include "MainApp_priv.h"

/*****************************************************************
 * Definition
 *****************************************************************/
#define AUX_IN_STATUS_BITMASK       AUXIN_JACK

#ifdef HAS_BATTERY
#ifdef HAS_SYSTEM_GAIN_CONTROL

const uint8 BattCapacityToSysGainTable[]=
{
    BATT_LEVEL_OF_SYS_GAIN_1,
    BATT_LEVEL_OF_SYS_GAIN_2,
    BATT_LEVEL_OF_SYS_GAIN_3,
    BATT_LEVEL_OF_SYS_GAIN_4,
};
const uint8 MaxNumOfBattCapacityToSysGain = ArraySize(BattCapacityToSysGainTable);
#endif
#endif

/*****************************************************************
 * Function Implemenation
 *****************************************************************/

void MainApp_InitStatusVariables(cMainApp * const me)
{
    SetSender((QActive *)me);
    me->audioSource = AUDIO_SOURCE_BT;

    me->systemStatus = SYSTEM_STA_OFF;

    me->isAmpFault = FALSE;
    me->isCuePlaying = FALSE;
    //impact connective LED
    me->isCriticalTemp = FALSE;
    me->isBTenabled = FALSE;
    me->btReBootCnt = 0;

    me->isBTStreaming = FALSE;

#ifdef HAS_BATTERY
    me->currBattLed = 0;
    me->battStatus = BatteryStatus_NO_BATTERY;
    me->btBattStatus = BTBatteryStatus_Lv_6;
    me->isChgEnable = FALSE;
    me->isChgComplete = FALSE;
#ifdef HAS_BATTERY_NTC
    me->battChgTemp_bk = TL_NORMAL;
    me->battDischgTemp_bk = TL_NORMAL;
    me->isChargeStopByNTC = FALSE;
    Setting_Reset(SETID_BATTERY_CHARGE_TEMP_LEVEL);
    Setting_Reset(SETID_BATTERY_DISCHARGE_TEMP_LEVEL);
#endif
#ifdef HAS_SYSTEM_GAIN_CONTROL
    me->currSysGain = SYS_GAIN_DEFAULT;
#endif
#endif
    //to do remove this after temp detect ready
    uint8 initvalue = FALSE;

    Setting_Set(SETID_SW_VER, PRODUCT_VERSION_MCU);
    Setting_Reset(SETID_IS_OVER_TEMP);
    Setting_Reset(SETID_IS_AMP_FAULT);

    Setting_Reset(SETID_DISPLAY_CAPACITY);
    Setting_Reset(SETID_BATTERY_STATUS);
    Setting_Set(SETID_ALLOW_POWERUP,&initvalue);
    Setting_Set(SETID_ALLOW_CHG_PWRUP,&initvalue);
    Setting_Set(SETID_MUSIC_STATUS,&initvalue);
    Setting_Reset(SETID_IS_DC_PLUG_IN);
    Setting_Reset(SETID_BT_STATUS);

    initvalue = BTBatteryStatus_Lv_6;
    Setting_Set(SETID_BT_BATT_STATUS,&initvalue);

    initvalue = INVALID_VALUE;
    Setting_Set(SETID_NTC_TEST_VALUE,&initvalue);
}



/* Note: The Api: MainApp_VolumeDown() and MainApp_VolumeUp() is just for HW bring-up in ES stage,
 * they can be removed if not need in later stages.
 */
void MainApp_VolumeDown(cMainApp * const me, uint8 step)
{
#ifndef DEBUG_QUICK_BOOT_NO_AUDIO
    // FIX-ME: avoid un-init setting server
    if (!Setting_IsReady(SETID_VOLUME))
        return;

    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
    // To Do: setting volume step down here
    AudioSrv_SetVolume(vol); //pass to DSPDrv1451_SetVol()
    TP_PRINTF("[%d] v = %d\r\n", __LINE__, vol);
#endif
}

void MainApp_VolumeUp(cMainApp * const me, uint8 step)
{
#ifndef DEBUG_QUICK_BOOT_NO_AUDIO
    // FIX-ME: avoid un-init setting server
    if (!Setting_IsReady(SETID_VOLUME))
        return;

    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);

    // To Do: setting volume step up here

    AudioSrv_SetVolume(vol); //pass to DSPDrv1451_SetVol()
    TP_PRINTF("[%d] v = %d\r\n", __LINE__, vol);
#endif
}

void MainApp_SetVolume(cMainApp * const me, uint8 vol)
{
    me->absoluteVol = vol;
    Setting_Set(SETID_VOLUME, &vol);
    AudioSrv_SetVolume(vol);
}

void MainApp_SendLedReq(cMainApp* me, eLedIndID ledIndID)
{
    ASSERT(ledIndID < LED_IND_ID_MAX);
    //TP_PRINTF("MainApp_SendLedReq: ledIndID=%d\r\n", ledIndID);
    bool enableDspTuning = FALSE;
#ifdef HAS_DSP_TUNING_MODE
    enableDspTuning = *(bool *)Setting_Get(SETID_IS_DSP_TUNING);
#endif
    /* Do NOT update LED indication if in DSP tuning mode or temperature level is lager than TL_NORMAL. */
    if ((ledIndID == LED_IND_ID_PROD_FW_ERROR || ledIndID == LED_IND_ID_HW_OVER_HEAT)||
        ((!me->isCriticalTemp) && (enableDspTuning == FALSE)) )
    {
        LedSrv_SetPatt((QActive*)me, me->ledInds[ledIndID].leds, me->ledInds[ledIndID].patternId);
    }
}

void MainApp_SendBattLedReq(cMainApp* me,ledMask baseled,uint8 count, ePattern patternId)
{
    uint8 i = 1;
    ledMask leds = baseled;
    for(; i<count; i++)
    {
        leds  |= (baseled << i);
    }
    LedSrv_SetPatt((QActive*)me,leds, patternId);
}

bool IsBluetoothPowerOn(uint8_t status)
{
    bool ret = FALSE;
    switch(status)
    {
        case BT_CONNECTABLE_STA:
        case BT_DISCOVERABLE_STA:
        case BT_CONNECTED_STA:
        case BT_STREAMING_A2DP_STA:
        case BT_AVRCP_PLAY_STA:
        case BT_AVRCP_PAUSE_STA:
            ret = TRUE;
            break;
    }
    return ret;
}

void MainApp_DelayedErrorReboot(cMainApp * const me)
{
    //delayed reboot for release biuld
    MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);

    /* wait 5 second(for finished LED indication), then reboot system. */
    me->tickHandlers[TIMER_ID_DELAYED_ERROR_REBOOT].timer = MAINAPP_DELAYED_ERROR_REBOOT_TIMEOUT_IN_MS;
}

void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}

void MainApp_Mute(cMainApp* me, bool muteEnable2)
{
    AudioSrv_SendMuteReq((QActive *)me, AUDIO_SOURCE_MUTE, muteEnable2);
}

void MainApp_ShowVersion(void)
{
    char * version_mcu = ((char *)Setting_Get(SETID_SW_VER));
    char * version_hw = ((char *)Setting_Get(SETID_HW_VER));
    char * version_bt = "null";// update when ready
    char * version_dsp =((char *)Setting_Get(SETID_DSP_VER));
    char msg[40];
    sprintf(msg,"HendrixM MCU:V%s,", version_mcu);
    DebugSSrv_Printf(msg);
    sprintf(msg," HW:V_%s,BT:V%s,", version_hw,version_bt);
    DebugSSrv_Printf(msg);
    sprintf(msg,"DSP:V%s .\n\r",version_dsp);
    DebugSSrv_Printf(msg);
}



QState MainApp_UpdateBattLedStatus(cMainApp * const me, QEvt const * const e)
{

    if (!Setting_IsReady(SETID_BATTERY_STATUS))
        return Q_UNHANDLED();
    if (MainApp_IsBatteryRemoved(me))
    {
        me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
        MainApp_SendLedReq(me, LED_IND_ID_BATT_LV_10);
        return Q_UNHANDLED();
    }

    uint8 batteryLevel = *((uint8*)Setting_Get(SETID_DISPLAY_CAPACITY));
    batteryLevel = MainaApp_BatteryCapcityToLedLv(batteryLevel);

    ledMask eLedMask = 0;
    ePattern ledPattern = TRANS_PREV_2_ON_PAT_RED;
    if (me->systemStatus == SYSTEM_STA_STANDBY ||
        me->systemStatus == SYSTEM_STA_OFF_CHARGING)
        ledPattern = TRANS_PREV_2_ON_PAT_DIM_RED;

    if(TRUE == me->isCriticalTemp)
    {
        //This is only for over temp indication at 5s before powering off.
        TYMQP_DUMP_QUEUE_WITH_LOG(me,"show_temp_error_led");
        // Close all battery led first
        LedSrv_SetPatt((QActive*)me, LED_MASK_BAT_LEDS, OFF_PATT);
        LedSrv_SetPatt((QActive*)me, LED_MASK_BAT_LEDS, FLASH_PAT_RED);
        me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
        return Q_UNHANDLED();
    }
    if(Setting_IsReady(SETID_IS_AMP_FAULT) && (TRUE == *(bool*)Setting_Get(SETID_IS_AMP_FAULT)))
    {
        me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;
        return Q_UNHANDLED();
    }
    if(me->isChgComplete)
    {
        LedSrv_SetPatt((QActive*)me,LED_MASK_BAT_LEDS,ledPattern);
        return Q_UNHANDLED();
    }

    //Update Charging led & status
    if(me->isChgEnable)
    {
        if(me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer <= 0)
        {
            LedSrv_SetPatt((QActive*)me, LED_MASK_BAT_LEDS,TRANS_PREV_2_OFF_PAT);
            me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = MAINAPP_BATTERY_LED_FADE_TIMEOUT_IN_MS;
            me->currBattLed = 0;
        }
        return Q_UNHANDLED();
    }
    else if(me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer > 0)
    {
        return Q_UNHANDLED();
    }
    else
        me->tickHandlers[TIMER_ID_SHOW_BATTERY_LED_TIMEOUT].timer = INVALID_VALUE;

    //Update Battery lv leds
    uint8 i;
    for(i = 0; i<batteryLevel; i++)
    {
        eLedMask |= LED_MASK_BAT_1<<i;
    }


    //close other batt led first
    LedSrv_SetPatt((QActive*)me, (LED_MASK_BAT_LEDS ^ eLedMask),OFF_PATT);

    //light lv batt led
    LedSrv_SetPatt((QActive*)me, eLedMask, ledPattern);
    //Update Battery lv leds
    me->currBattLed = batteryLevel;

    return Q_UNHANDLED();
}

void MainApp_SendRespOfMainAppSig(cMainApp * const me, eEvtReturn e )
{
    CommonRespEvt* pResp = Q_NEW(CommonRespEvt,DEBUG_RESP_SIG);
    pResp->evtReturn = e;
    SendToServer(DEBUG_SRV_ID, (QEvt*)pResp);
    //or use this to send response back
    //CommonEvtResp((QActive*)me, pMuteReqEvt->sender, RET_SUCCESS, AUDIO_MUTE_RESP_SIG);
}

void MainApp_SendToMainApp(cMainApp * const me, eSignal e)
{
    QEvt* pEvt = Q_NEW(QEvt, e);
    QACTIVE_POST((QActive *)me, pEvt, 0);
}

void MainApp_SetChannel(cMainApp * const me,eAudioChannel channel)
{

    me->audioSource = channel;
    AudioSrv_SetChannel((QActive*)me,channel);
}


void MainApp_UpdateProdLedStatus(cMainApp * const me, QEvt const * const e)
{
    eLedIndID ledind = LED_IND_ID_BLUETOOTH;
    eBtStatus btStatus = me->CurrBTStatus;

    switch(btStatus)
    {
        case BT_CONNECTABLE_STA:
        case BT_LINKLOST_STA:
            ledind = LED_IND_ID_BLUETOOTH_CONNECTABLE;
            break;
        case BT_DISCOVERABLE_STA:
            ledind = LED_IND_ID_BLUETOOTH_PAIRING;
            break;
        case BT_CONNECTED_STA:
        case BT_STREAMING_A2DP_STA:
            ledind = LED_IND_ID_BLUETOOTH;
            break;
        default:
            return;
    }

    if(me->audioSource == AUDIO_SOURCE_BT)
    {
    }
    else
    {
        ledind = LED_IND_ID_AUXIN;
    }

    //IDLE ledInd is next value of ACTIVE ledInd
    if(me->systemStatus == SYSTEM_STA_STANDBY)
    {
        ledind += 1;
    }

    MainApp_SendLedReq(me, ledind);
}

void MainApp_UpdateAudioChannel(cMainApp * const me,uint8 audiostatus)
{
    switch(me->audioSource)
    {
        case AUDIO_SOURCE_BT:
            if((audiostatus & (1<<AUXIN_JACK))&&me->isNoButtonChanel)
            {
                MainApp_SetChannel(me,AUDIO_SOURCE_AUXIN);

                // not stable now, need be debugging pause mucsic function.
                // state will not update immediately while stop bt playback.
                // this will encounter bt playback can't hijack aux music in short time.
                //if(me->CurrBTStatus == BT_STREAMING_A2DP_STA)
                //BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PAUSE_CMD);

            }
            else
            {
                me->isNoButtonChanel = TRUE;
            }
            break;
        case AUDIO_SOURCE_AUXIN:
            if(
#ifdef DSP_BT_CHANNEL_DETECTION
                (audiostatus & (1<<BLUETOOTH_JACK) && me->CurrBTStatus == BT_STREAMING_A2DP_STA) ||
#endif
                // aux stop streaming, change source to BT.
                (!(audiostatus & (1<<AUXIN_JACK))) )
            {
                MainApp_SetChannel(me,AUDIO_SOURCE_BT);
            }
            break;
        default:
            break;
    }
}
void MainApp_SendBTCueCmd(cMainApp * const me,  eBtCmd cmd)
{
    if(!me->isBTenabled)
        return;
    if(!me->isCuePlaying)
    {
        me->isCuePlaying = TRUE;
        BluetoothSrv_SendBtCmd((QActive*)me, cmd);
    }
    else if(RingBuf_GetFreeSize(&me->btCueRBufObj) > 0)
    {
        uint8 btcmd = (uint8)cmd;
        RingBuf_Push(&me->btCueRBufObj, &btcmd, sizeof(uint8));
    }
}


void Mainapp_SaveData(cMainApp * const me)
{

    /*shop mode or LS mode don't need saving data*/
    uint32 shopword = *(uint32*)Setting_Get(SETID_SHOP_MODE_WORD);
    if((shopword == SHOP_MODE_VALUE) ||
       (shopword == LS_SAMPLE_VALUE))
    {
        return;
    }

    if(me->systemStatus == SYSTEM_STA_POWERING_DOWN)
    {
        if(me->audioSource != *((eAudioSource*)Setting_Get(SETID_AUDIO_SOURCE)))
            Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
    }
    uint32 capacity = (uint32)(*(uint8*)Setting_Get(SETID_DISPLAY_CAPACITY));
    if(capacity != *(uint8*)Setting_Get(SETID_STORE_CAPACITY))
    {
        Setting_Set(SETID_STORE_CAPACITY, &capacity);
        TYMQP_DUMP_QUEUE_WITH_LOG(me, "STORE BATTERY CAP %d", capacity);
    }

}



#ifdef HAS_BATTERY

void MainApp_UpdateBTBattIndicator(cMainApp * const me)
{
#ifdef HAS_BT_BATT_CMD
    TYMQP_DUMP_QUEUE_WITH_LOG(me, "UPDATE %d", me->btBattStatus);
    if(me->CurrBTStatus == BT_CONNECTABLE_STA)
        return;
    switch(me->btBattStatus)
    {
        case BTBatteryStatus_Lv_1:
            BluetoothSrv_SendBtCmd((QActive*)me, BT_BATT_1);
            break;
        case BTBatteryStatus_Lv_2:
            BluetoothSrv_SendBtCmd((QActive*)me, BT_BATT_2);
            break;
        case BTBatteryStatus_Lv_3:
            BluetoothSrv_SendBtCmd((QActive*)me, BT_BATT_3);
            break;
        case BTBatteryStatus_Lv_4:
            BluetoothSrv_SendBtCmd((QActive*)me, BT_BATT_4);
            break;
        case BTBatteryStatus_Lv_5:
            BluetoothSrv_SendBtCmd((QActive*)me, BT_BATT_5);
            break;
        case BTBatteryStatus_Lv_6:
            BluetoothSrv_SendBtCmd((QActive*)me, BT_BATT_6);
            break;
        default:
            break;
    }
#endif
}

uint8 MainaApp_BatteryCapcityToLedLv(uint8 batteryCa)
{
    uint8 mod = batteryCa % LED_BATTERY_BAR_LV;
    batteryCa /= LED_BATTERY_BAR_LV;
    if(mod)
    {
        batteryCa++;
    }
    return batteryCa;
}

bool MainApp_IsBatteryAllowCharge(cMainApp * const me)
{
    bool ret = TRUE;
#ifdef HAS_BATTERY_NTC
    if(me->isChargeStopByNTC)
    {
        ret = FALSE;
    }
#endif
    return ret;
}
void MainApp_ChargingEnable(cMainApp * const me)
{
    if(MainApp_IsBatteryAllowCharge(me))
    {
        me->isChgEnable = TRUE;
        PowerSrv_Set((QActive *)me, POWER_SET_ID_CHARGER_ON, TRUE); //turn on charging
    }
}

void MainApp_ChargingDisable(cMainApp * const me)
{
    me->isChgEnable = FALSE;
    PowerSrv_Set((QActive *)me, POWER_SET_ID_CHARGER_ON, FALSE); //turn off charging
}

bool MainApp_IsBatteryRemoved(cMainApp * const me)
{
    if((*(uint8*)Setting_Get(SETID_IS_BATT_VOL_ZERO) == TRUE) &&
       (*(uint8*)Setting_Get(SETID_IS_BATT_NTC_REMOVED) == TRUE) &&
       (*(uint8*)Setting_Get(SETID_CHARGING_STATUS) == CHARGER_STA_ERROR))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#ifdef HAS_SYSTEM_GAIN_CONTROL
void MainApp_AdjustSysGainForDcIn(cMainApp * const me)
{
    uint8 nextSysGain = SYS_GAIN_DEFAULT;
    if(nextSysGain!= me->currSysGain)
    {
        me->currSysGain = nextSysGain;
        AudioSrv_SetAudio(SYSTEM_GAIN_SETT_ID, TRUE, (uint32)me->currSysGain, /*not-used*/0);
    }
}


void MainApp_AdjustSysGainForDcOut(cMainApp * const me)
{
    uint8 i;
    uint8 batteryLevel = *((uint8*)Setting_Get(SETID_BATTERY_CAPACITY));
    uint8 nextSysGain = SYS_GAIN_MAX;
    for(i=0; i<MaxNumOfBattCapacityToSysGain; i++)
    {
        if(batteryLevel > BattCapacityToSysGainTable[i])
        {
            nextSysGain = i+SYS_GAIN_LEVEL1;
            break;
        }
    }

    if(nextSysGain!= me->currSysGain)
    {
        me->currSysGain = nextSysGain;
        AudioSrv_SetAudio(SYSTEM_GAIN_SETT_ID, TRUE, (uint32)me->currSysGain, /*not-used*/0);
    }
}
#endif  /*end of HAS_SYSTEM_GAIN_CONTROL*/



#endif /* end of  HAS_BATTERY */




