
/**
*  @file      MainApp_BluetoothEvtHandler.c
*  @brief     Bluetooth event handler of mainApp
*  @author    Colin Chen
*  @date      16-Nov.-2017
*  @copyright Tymphany Ltd.
*  @history:
*     v0.1    Colin Chen  28-Nov-2017      draft
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

#define BT_EVT_HANDLER_DEBUGx
#ifdef BT_EVT_HANDLER_DEBUG
#undef  TP_PRINTF
#define TP_PRINTF printf
#else
#undef  TP_PRINTF
#define TP_PRINTF(...)
#endif

static QState MainApp_BtConnectableEvtAction(cMainApp * const me, BtStatusEvt const * const e)
{
    TYMQP_DUMP_QUEUE_WITH_LOG(me,": BT Connectable");
    QState ret = Q_UNHANDLED();
    BtStatusEvt *evt = (BtStatusEvt*)e;
    if(//me->CurrBTStatus == BT_DISCOVERABLE_STA ||
       me->CurrBTStatus == BT_STREAMING_A2DP_STA ||
       me->CurrBTStatus == BT_CONNECTED_STA)
    {
        /* paring fail go into Conectable status & connected to connectable*/
        MainApp_SendBTCueCmd(me, BT_PAIRING_FAIL_CUE_CMD);      //edmond_20210711
    }
    me->isBTStreaming = FALSE;

    me->CurrBTStatus = evt->btStatus;
    // Play Cue first, then change to AUX source if aux avaliable
    if(me->audioSource == AUDIO_SOURCE_BT)
        me->tickHandlers[TIMER_ID_SOURCE_CHANGE_TIMEOUT].timer = MAINAPP_WAIT_FOR_CUE_TIMEOUT_IN_MS;

    return ret;
}

static QState MainApp_BtDiscoverableEvtAction(cMainApp * const me, BtStatusEvt const * const e)
{
    TYMQP_DUMP_QUEUE_WITH_LOG(me,": BT Discoverable");
    QState ret = Q_UNHANDLED();
    BtStatusEvt *evt = (BtStatusEvt*)e;

#ifdef HAS_BT_PAIRING_FILTER
    if(me->tickHandlers[TIMER_ID_PAIRING_FILTER_TIMEOUT].timer <= 0)
        return ret;
#endif

    /* paring start */
    //AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME); //Nick++ to prevent audio cue volume not set
    MainApp_SendBTCueCmd(me, BT_PAIRING_START_CUE_CMD);
    // cue is gonna playing, change this for source stay in BT when cue stop.
    me->audioSource = AUDIO_SOURCE_BT;


    me->tickHandlers[TIMER_ID_SOURCE_CHANGE_TIMEOUT].timer = INVALID_VALUE;

    me->CurrBTStatus = evt->btStatus;
    return ret;
}

static QState MainApp_BtConnectedEvtAction(cMainApp * const me, BtStatusEvt const * const e)
{
    TYMQP_DUMP_QUEUE_WITH_LOG(me,": BT Connected");
    QState ret = Q_UNHANDLED();
    BtStatusEvt *evt = (BtStatusEvt*)e;
    me->isBTStreaming = FALSE;

    /* paring success is connected */
    if(me->CurrBTStatus != BT_STREAMING_A2DP_STA)
    {
        AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME); //Nick++ to prevent audio cue volume not set
        MainApp_SendBTCueCmd(me, BT_PAIRING_SUCCESS_CUE_CMD);      //edmond_20210711
        me->tickHandlers[TIMER_ID_BT_BATT_NOTIFY_TIMEOUT].timer = MAINAPP_BT_BATT_NOTIFY_TIMEOUT_IN_MS;
        if(!(me->ConnectedCue))
        {
            me->tickHandlers[TIMER_ID_Connected_cue_TIMEOUT].timer = MAINAPP_Connected_cue_IN_MS;
            me->ConnectedCue = TRUE;
        }
    }
    else
    {
        // Play Cue first, then change to AUX source if aux avaliable
        if(me->audioSource == AUDIO_SOURCE_BT)
            me->tickHandlers[TIMER_ID_SOURCE_CHANGE_TIMEOUT].timer = MAINAPP_WAIT_FOR_CUE_TIMEOUT_IN_MS;

    }
    me->CurrBTStatus = evt->btStatus;
    return ret;
}

static QState MainApp_BtStreamingA2DPEvtAction(cMainApp * const me, BtStatusEvt const * const e)
{
    TYMQP_DUMP_QUEUE_WITH_LOG(me,": BT Streaming");
    QState ret = Q_UNHANDLED();
    BtStatusEvt *evt = (BtStatusEvt*)e;
    /* A2DP play */
    me->CurrBTStatus = evt->btStatus;
    me->isBTStreaming = TRUE;
    me->tickHandlers[TIMER_ID_SOURCE_CHANGE_TIMEOUT].timer = INVALID_VALUE;

    //if(me->audioSource == AUDIO_SOURCE_AUXIN)
    {
        MainApp_SetChannel(me,AUDIO_SOURCE_BT);
        uint8 audiostatus;
#ifdef DSP_BT_CHANNEL_DETECTION
        audiostatus = *(uint8_t*)Setting_Get(SETID_MUSIC_STATUS);
        audiostatus |= (1<<BLUETOOTH_JACK);
        Setting_Set(SETID_MUSIC_STATUS, &(audiostatus));
        audiostatus = TRUE;
        Setting_Set(SETID_MUSIC_DET,&audiostatus);
#endif

    }
    return ret;
}

static QState MainApp_BtLinklostEvtAction(cMainApp * const me, BtStatusEvt const * const e)
{
    TYMQP_DUMP_QUEUE_WITH_LOG(me,": second dev disconnect");

    QState ret = Q_UNHANDLED();
    MainApp_SendBTCueCmd(me, BT_PAIRING_FAIL_CUE_CMD);

    return ret;
}

static QState MainApp_BtLinklostOffEvtAction(cMainApp * const me, BtStatusEvt const * const e)
{
    TYMQP_DUMP_QUEUE_WITH_LOG(me,": second dev connect");

    QState ret = Q_UNHANDLED();
    
    if(me->CurrBTStatus != BT_STREAMING_A2DP_STA)
      AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME);//Nick++ to prevent audio cue volume not set        
    
    MainApp_SendBTCueCmd(me, BT_PAIRING_SUCCESS_CUE_CMD);
    me->tickHandlers[TIMER_ID_BT_BATT_NOTIFY_TIMEOUT].timer = MAINAPP_BT_BATT_NOTIFY_TIMEOUT_IN_MS;

    if(me->CurrBTStatus == BT_CONNECTABLE_STA ||
       me->CurrBTStatus == BT_DISCOVERABLE_STA )
        me->CurrBTStatus == BT_CONNECTED_STA;
    return ret;
}


static QState MainApp_BtAudioCueStartEvtAction(cMainApp * const me, BtStatusEvt const * const e)
{
    TYMQP_DUMP_QUEUE_WITH_LOG(me,"BTHandler Cue Start");
    QState ret = Q_UNHANDLED();

    //volume slew takes time, change volume first.
    AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME);
    MainApp_Mute(me,TRUE);

    //CurrBTStatus will be covered
    //delay to unmute change source
    if(me->isBTStreaming == TRUE)
    {
        me->tickHandlers[TIMER_ID_UNMUTE_DELAY_TIMEOUT].timer = MAINAPP_BT_CUE_START_DELAY_TIMEOUT_IN_MS;
    }
    else
    {
        me->tickHandlers[TIMER_ID_UNMUTE_DELAY_TIMEOUT].timer = MAINAPP_BTCUE_NOCONN_DELAY_TIMEOUT_IN_MS;
    }
    AudioSrv_SetChannel((QActive*)me,AUDIO_SOURCE_BT);
    return ret;
}

static QState MainApp_BtAudioCueStopEvtAction(cMainApp * const me, BtStatusEvt const * const e)
{
    TYMQP_DUMP_QUEUE_WITH_LOG(me,"BTHandler Cue Stop");
    QState ret = Q_UNHANDLED();

    if(!RingBuf_IsEmpty(&me->btCueRBufObj))
    {
        MainApp_Mute(me,TRUE);

        //delay to get another cmd
        me->tickHandlers[TIMER_ID_CUE_CMD_DELAY_TIMEROUT].timer = MAINAPP_CUE_CMD_DELAY_TIMEOUT_IN_MS;
        return ret;
    }
    TYMQP_DUMP_QUEUE_WITH_LOG(me,"not got cue cmd");
    me->isCuePlaying = FALSE;
    me->isCueChanel = TRUE;
    if(me->audioSource == AUDIO_SOURCE_AUXIN)
    {
        AudioSrv_SetChannel((QActive*)me,AUDIO_SOURCE_AUXIN);
    }
    Setting_Set(SETID_VOLUME, &(me->absoluteVol));
    AudioSrv_SetVolume(me->absoluteVol);
    return ret;
}

static QState MainApp_BtAvrcpPlayEvtAction(cMainApp * const me, BtStatusEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    return ret;
}

static QState MainApp_BtAvrcpPauseEvtAction(cMainApp * const me, BtStatusEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    return ret;
}

QState MainApp_BluetoothEvtHandler(cMainApp * const me, QEvt const * const e)
{
    // To Do:  Handle bluetooth event

    QState ret = Q_UNHANDLED();
    BtStatusEvt *evt = (BtStatusEvt*)e;
    evt->btStatus = *(uint8_t*)Setting_Get(SETID_BT_STATUS);
    TP_PRINTF("\r\n MainApp_BTHandler status:%d \r\n", evt->btStatus);
    if(me->CurrBTStatus == evt->btStatus)
        return ret;

    switch(evt->btStatus)
    {
        case BT_CONNECTABLE_STA:
            ret = MainApp_BtConnectableEvtAction(me, evt);
            break;
        case BT_DISCOVERABLE_STA:
            ret = MainApp_BtDiscoverableEvtAction(me, evt);
            break;
        case BT_CONNECTED_STA:
            ret = MainApp_BtConnectedEvtAction(me, evt);
            break;
        case BT_STREAMING_A2DP_STA:
            ret = MainApp_BtStreamingA2DPEvtAction(me, evt);
            break;
        case BT_LINKLOST_STA:
            ret = MainApp_BtLinklostEvtAction(me, evt);
            break;
        case BT_LINKLOST_OFF_STA:
            ret = MainApp_BtLinklostOffEvtAction(me, evt);
            break;
        case BT_AUDIO_CUE_START_STA:
            ret = MainApp_BtAudioCueStartEvtAction(me, evt);
            break;
        case BT_AUDIO_CUE_STOP_STA:
            ret = MainApp_BtAudioCueStopEvtAction(me, evt);
            break;
        case BT_AVRCP_PLAY_STA:
            ret = MainApp_BtAvrcpPlayEvtAction(me, evt);
            break;
        case BT_AVRCP_PAUSE_STA:
            ret = MainApp_BtAvrcpPauseEvtAction(me, evt);
            break;
        default:
            break;
    }
    return ret;
}

