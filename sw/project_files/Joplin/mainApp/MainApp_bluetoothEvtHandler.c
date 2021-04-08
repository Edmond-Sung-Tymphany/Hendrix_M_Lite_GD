
/**
*  @file      MainApp_BluetoothEvtHandler.c
*  @brief     Bluetooth event handler of mainApp
*  @author    Colin Chen
*  @date      16-Nov.-2017
*  @copyright Tymphany Ltd.
*/

#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "trace.h"
#include "BluetoothSrv.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "tym_qp_lib.h"
#include "MainApp_priv.h"
#include "BluetoothSrv.h"

#define MAINAPP_BT_DEBUG_ENABLE
#ifndef MAINAPP_BT_DEBUG_ENABLE
#undef  TP_PRINTF
#define TP_PRINTF(...)
#endif

/* EqBand_t should be keep same as the definition in DSP driver. */
typedef enum _EqBands
{
    EQ_BAND1,
    EQ_BAND2,
    EQ_BAND3,
    EQ_BAND4,
    EQ_BAND5,
    EQ_BAND_MAX,
}EqBand_t;

static void MainApp_BtConnStateHandler(cMainApp * const me, Proto_BtState_ConnState connState)
{
    TP_PRINTF("bt connection state: %d \r\n", connState);
    switch (connState)
    {
        case Proto_BtState_ConnState_CONNECTABLE:
        {
            TP_PRINTF("BtMcuEvent: connectable \r\n");
            break;
        }
        case Proto_BtState_ConnState_PAIRING:
        {
            TP_PRINTF("BtMcuEvent: enabled pairing \r\n");
            break;
        }
        case Proto_BtState_ConnState_RECONNECTING:
        {
            TP_PRINTF("BtMcuEvent: reconnecting \r\n");
            break;
        }
        case Proto_BtState_ConnState_CONNECTED:
        {
            TP_PRINTF("BtMcuEvent: connected \r\n");
            break;
        }
        case Proto_BtState_ConnState_A2DPSTREAMING:
        {
            TP_PRINTF("BtMcuEvent: steaming \r\n");
            break;
        }
        default:
            break;
    }
    if (me->audioSource == AUDIO_SOURCE_BT)
    {
        MainApp_UpdateBtLed(me, connState);
    }
    me->connState = connState;

}
static QState MainApp_BtMcuEventHandler(cMainApp * const me, Proto_BtMcu_Event *pBtMcuEvent)
{
    QState ret = Q_UNHANDLED();
    switch (pBtMcuEvent->type)
    {
        case Proto_BtMcu_Event_Type_BOOTED:
        {
            TP_PRINTF("BtMcuEvent: Booted \r\n");
            me->isBtBooted = TRUE;
            if (me->isAudioDrvReady)
            {
                TP_PRINTF("###111 Notify BT module: system is booted.\r\n");
                /* Notify BT module: system is booted. */
                BluetoothSrv_SendBtCmd((QActive*)me, MCU_SYSTEM_BOOTED);
            }
            break;
        }
        case Proto_BtMcu_Event_Type_BT_A2DP_CONNECTED:
        {
            TP_PRINTF("BtMcuEvent: a2dp connected \r\n");
            /* update system satus and timer */
            MainApp_UpdateSystemStatus(me, SYSTEM_STA_ACTIVE);
            break;
        }
        case Proto_BtMcu_Event_Type_BT_A2DP_DISCONNECTED:
        {
            TP_PRINTF("BtMcuEvent: a2dp disconnected \r\n");
            break;
        }
        case Proto_BtMcu_Event_Type_BT_CONNECTION_STATE:
        {
            MainApp_BtConnStateHandler(me, pBtMcuEvent->OneOf.connState);
            break;
        }
        case Proto_BtMcu_Event_Type_BT_AUDIO_CUE_PLAY_START:
        {
            me->isCuePlaying = TRUE;
            me->tickHandlers[TIMER_ID_AUDIO_CUE_TIMEOUT].timer = MAINAPP_AUDIO_CUE_TIMEOUT_IN_MS;
            if (me->audioSource != AUDIO_SOURCE_BT)
            {
                AudioSrv_SetChannel((QActive *)me, AUDIO_CHANNEL_BT);
            }
            if (me->systemStatus == SYSTEM_STA_POWERING_UP)
            {
                AudioSrv_SendMuteReq((QActive *)me, AUDIO_SOURCE_MUTE, FALSE);
            }
            AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME);
            break;
        }
        case Proto_BtMcu_Event_Type_BT_AUDIO_CUE_PLAY_STOP:
        {
            me->tickHandlers[TIMER_ID_AUDIO_CUE_TIMEOUT].timer = 0;
            if (me->systemStatus == SYSTEM_STA_POWERING_DOWN)
            {
                //AudioSrv_SendMuteReq((QActive *)me, AUDIO_SOURCE_MUTE, TRUE);
            }
            else if (me->audioSource != AUDIO_SOURCE_BT && me->isCuePlaying == TRUE)
            {
                MainApp_SwitchAudioSource(me, me->audioSource, TRUE);
            }
            me->isCuePlaying = FALSE;
            AudioSrv_SetVolume(me->vol/CLICK_NUM_PER_VOLUME_STEP);
            break;
        }
        case Proto_BtMcu_Event_Type_VOLUME_CHANGED:
        {
            int8 step = pBtMcuEvent->OneOf.volumeStep;
            TP_PRINTF("BtMcuEvent: vol step %d\r\n", pBtMcuEvent->OneOf.volumeStep);
            if (me->vol/CLICK_NUM_PER_VOLUME_STEP > step)
            {
                MainApp_SetRotaterLedOff(me, LED_VOL_0, vol_led_mapping[step]);
            }
            else
            {
                MainApp_SetRotaterLedOn(me, LED_VOL_0, vol_led_mapping[step]);
            }
            if (me->isCuePlaying == FALSE)
            {
                AudioSrv_SetVolume(step);
            }
            me->vol = CLICK_NUM_PER_VOLUME_STEP * step;
            break;
        }
        case Proto_BtMcu_Event_Type_SOURCE_CHANGED:
        {
            eAudioSource source = pBtMcuEvent->OneOf.source;
            TP_PRINTF("BtMcuEvent: BLE source changed: %d\r\n", pBtMcuEvent->OneOf.source);
            if (me->audioSource != source)
            {
                MainApp_SwitchAudioSource(me, source, TRUE);
                BluetoothSrv_SendBtCmdWithParam((QActive*)me, MCU_SOURCE_CHANGE_EVENT, sizeof(eAudioSource), (uint8 *)&source);
            }
            break;
        }

        case Proto_BtMcu_Event_Type_EQ_SET:
        {
            /* TODO: Below code need to be debuged with BT firmware to verify. */
            uint8 i = 0;
            uint8 eqBands[DSP_EQ_BAND_NUM] = {0};
            memcpy(eqBands, pBtMcuEvent->OneOf.eq.eq.bytes, DSP_EQ_BAND_NUM);

            for (i = 0; i < DSP_EQ_BAND_NUM; i++)
            {
                eqBands[i] = (eqBands[i] < (MAX_EQ_BAND_STEPS-1))? eqBands[i] : (MAX_EQ_BAND_STEPS-1);
                if (eqBands[i] != (*(uint32 *)Setting_Get(SETID_EQ_BAND1+i)))
                {
                    AudioSrv_SetAudio(DSP_EQ_CTRL_TUNING, TRUE, EQ_BAND1+i, eqBands[i]);
                }
            }

            /* Update Bass LEDs */
            if (me->bass/CLICK_NUM_PER_BASS_STEP > eqBands[0])
            {
                MainApp_SetRotaterLedOff(me, LED_BAS_0, eqBands[0]);
            }
            else
            {
                MainApp_SetRotaterLedOn(me, LED_BAS_0, eqBands[0]);
            }
            me->bass = eqBands[0]*CLICK_NUM_PER_BASS_STEP;

            /* Update Treble LEDs */
            if (me->treble/CLICK_NUM_PER_TREBLE_STEP > eqBands[4])
            {
                MainApp_SetRotaterLedOff(me, LED_TRE_0, eqBands[4]);
            }
            else
            {
                MainApp_SetRotaterLedOn(me, LED_TRE_0, eqBands[4]);
            }
            me->treble = eqBands[4]*CLICK_NUM_PER_TREBLE_STEP;
            break;
        }

        case Proto_BtMcu_Event_Type_LIGHT_SET:
        {
            /* TODO: need to change definition in tymproto */
            uint8 steady_brightness = (uint8)pBtMcuEvent->OneOf.light.light.bytes[0];
            if (steady_brightness > STEADY_BRIGHTNESS_MAX)
            {
                steady_brightness = STEADY_BRIGHTNESS_MAX;
            }
            MainApp_UpdateSteadyBrightness(steady_brightness);

            uint8 dim_brightness = (uint8)pBtMcuEvent->OneOf.light.light.bytes[1];
            if (dim_brightness > DIM_BRIGHTNESS_MAX)
            {
                dim_brightness = DIM_BRIGHTNESS_MAX;
            }
            MainApp_UpdateDimBrightness(dim_brightness);

            MainApp_UpdateLed(me, me->systemStatus);
            break;
        }

        default:
            TP_PRINTF("BtMcuEvent: %d=unknown\r\n", pBtMcuEvent->type);
            break;
    }

    return ret;
}

static QState MainApp_BtMcuReqHandler(cMainApp * const me, Proto_BtMcu_Req *pBtMcuReq)
{
    QState ret = Q_UNHANDLED();
    switch (pBtMcuReq->type)
    {
        case Proto_BtMcu_ReqResp_MCU_APPLICATION_IS_RUNNING:
        {
            TP_PRINTF("btMcuReq: communication heartbeat  \r\n");
            break;
        }

        case Proto_BtMcu_ReqResp_MCU_FIRMWARE_UPDATE:
        {
            ret = Q_TRAN(&MainApp_FirmwareUpdate);
            break;
        }

        default:
            TP_PRINTF("btMcuReq: %d=unknown\r\n", pBtMcuReq->type);
            break;
    }
    return ret;
}

static QState MainApp_BtMcuRespHandler(cMainApp * const me, Proto_BtMcu_Resp *pBtMcuResp)
{
    QState ret = Q_UNHANDLED();
    switch (pBtMcuResp->type)
    {
        case Proto_McuBt_ReqResp_BT_PAIRING_ON:
        {
            TP_PRINTF("btMcuReq: BT pairing enabled  \r\n");
            break;
        }
        case Proto_McuBt_ReqResp_BT_SW_VERSION:
        {
            uint32 version = pBtMcuResp->OneOf.swVersion;
            char msg[BT_VERSION_LENGTH] = "0.0.0";
            msg[0] += (version>>24) & 0x0F;
            msg[2] += (version>>20) & 0x0F;
            msg[4] += (version>>16) & 0x0F;
            Setting_Set(SETID_BT_VER_STR, msg);
            break;
        }
        case Proto_McuBt_ReqResp_BT_LOCAL_ADDR:
        {
            uint8 btMac[BT_MAC_ADDR_LENGTH] = {0};
            memcpy(btMac, pBtMcuResp->OneOf.btInfo.btAddr.bytes, BT_MAC_ADDR_LENGTH);
            TP_PRINTF("\r\n bt address:");
            for (int i=0; i<BT_MAC_ADDR_LENGTH; i++)
            {
                TP_PRINTF( "%02x", btMac[i]);
            }
            TP_PRINTF("\r\n");
            Setting_Set(SETID_BT_ADDR,btMac);
            break;
        }
        case Proto_McuBt_ReqResp_PING:
        {
            /* Reset communication watchdog timer */
            me->tickHandlers[TIMER_ID_COMM_WDG_TIMEOUT].timer = 0;
            break;
        }
        default:
            TP_PRINTF("btMcuResp: %d=unknown\r\n", pBtMcuResp->type);
            break;
    }
    return ret;
}

QState MainApp_BluetoothEvtHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    Proto_Tym_BtMcuMessage *pMessage = (Proto_Tym_BtMcuMessage *)((BtRxEvt *)e)->data;
    switch (pMessage->which_OneOf)
    {
        case Proto_Tym_BtMcuMessage_btMcuEvent_tag:
            ret = MainApp_BtMcuEventHandler(me, &(pMessage->OneOf.btMcuEvent));
            break;
        case Proto_Tym_BtMcuMessage_btMcuReq_tag:
            ret = MainApp_BtMcuReqHandler(me, &(pMessage->OneOf.btMcuReq));
            break;
        case Proto_Tym_BtMcuMessage_btMcuResp_tag:
            ret = MainApp_BtMcuRespHandler(me, &(pMessage->OneOf.btMcuResp));
            break;
        default:
            //TP_PRINTF("Unknown BtMcuMessage: which_OneOf=%d\r\n", pMessage->which_OneOf);
            break;
    }

    return ret;
}

QState MainApp_BtEvtFilterInShopMode(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_HANDLED();
    Proto_Tym_BtMcuMessage *pMessage = (Proto_Tym_BtMcuMessage *)((BtRxEvt *)e)->data;
    switch (pMessage->which_OneOf)
    {
        case Proto_Tym_BtMcuMessage_btMcuEvent_tag:
            if ((pMessage->OneOf.btMcuEvent.type == Proto_BtMcu_Event_Type_BOOTED) ||
                (pMessage->OneOf.btMcuEvent.type == Proto_BtMcu_Event_Type_SW_UPDATE_STARTED) ||
                (pMessage->OneOf.btMcuEvent.type == Proto_BtMcu_Event_Type_SW_UPDATE_FINISHED) ||
                (pMessage->OneOf.btMcuEvent.type == Proto_BtMcu_Event_Type_SW_UPDATE_FAILED))
            {
                ret = Q_UNHANDLED();
            }
            break;
        case Proto_Tym_BtMcuMessage_btMcuReq_tag:
            if (pMessage->OneOf.btMcuReq.type == Proto_BtMcu_ReqResp_MCU_FIRMWARE_UPDATE)
            {
                ret = Q_UNHANDLED();
            }
            break;
        case Proto_Tym_BtMcuMessage_btMcuResp_tag:
            ret = Q_UNHANDLED();
            break;
        default:
            //TP_PRINTF("Unknown BtMcuMessage: which_OneOf=%d\r\n", pMessage->which_OneOf);
            break;
    }

    return ret;
}


