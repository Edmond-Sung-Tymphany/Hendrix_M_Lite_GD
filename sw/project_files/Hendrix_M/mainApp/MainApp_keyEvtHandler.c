/**
*  @file      MainApp_KeyEvtHandler.c
*  @brief     Key event handler of mainApp
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
#include "BluetoothSrv.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "tym_qp_lib.h"
#include "MainApp_priv.h"

/*****************************************************************
 * Global Variable
 *****************************************************************/
#define FACTORY_RESET_COMB                  COMB_KEY_ID_0


/*****************************************************************
 * Function Implemenation
 *****************************************************************/

static QState MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {
        case BT_KEY:

            break;
        case PLAY_PAUSE_KEY:
            break;
        /*if necessary, add the handler for more keys here */
        default:
            break;
    }
    return ret;
}
static QState MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {
        case BT_KEY:

            break;
        case PLAY_PAUSE_KEY:
            break;
        /*if necessary, add the handler for keys here */
        default:
            break;
    }
    return ret;
}

static QState MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_HANDLED();

    uint8 audiostatus = *(uint8_t*)Setting_Get(SETID_MUSIC_STATUS);

    switch(e->keyId)
    {
        case BT_KEY:
            TP_PRINTF("MainApp BT Key SP \r\n");

            switch(me->CurrBTStatus)
            {
                case BT_DISCOVERABLE_STA: //stop pair while pairing
                    //TP_PRINTF("MainApp BT Key SP Stop BT Pairing \r\n");
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                    break;
                case BT_CONNECTABLE_STA:  //Aux source to BT source, or source in BT & no Aux playback & reconnecting.
                    if (me->audioSource == AUDIO_SOURCE_AUXIN ||
                        !(audiostatus & (1<<AUXIN_JACK)))
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_RECONNECT_CMD);
                        me->isNoButtonChanel = FALSE;
                case BT_CONNECTED_STA:    //stay connected, change source to aux
                case BT_STREAMING_A2DP_STA:
                    if (me->audioSource == AUDIO_SOURCE_BT)
                    {
                        if(audiostatus & (1<<AUXIN_JACK))
                        {
                            MainApp_SetChannel(me,AUDIO_SOURCE_AUXIN);
                        }
                    }
                    else
                    {
                        MainApp_SetChannel(me,AUDIO_SOURCE_BT);
                        me->ConnectedCue = FALSE;
                    }
                    break;
                default:
                    break;
            }
            break;
        case PLAY_PAUSE_KEY:

            //To do
            //play pause during bt streaming
            //if sp 2 times & release
            //do next track
            //if sp 3 times & release
            //do previous track
            break;

        /*if necessary, add the handler for more keys here */
        default:
            break;
    }
    return ret;
}

static QState MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_HANDLED();
    TP_PRINTF("MainApp_LPress BTstatus:%d \n", me->CurrBTStatus);
    switch(e->keyId)
    {
        case BT_KEY:
            /*switch(me->CurrBTStatus)
            {
                case BT_STREAMING_A2DP_STA:
                //BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PAUSE_CMD);
                case BT_CONNECTED_STA:      //disconnect & start pairing
                case BT_CONNECTABLE_STA:    //start pairing from connectable
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                    break;
                case BT_DISCOVERABLE_STA:   //stop pairing from pairing mode
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                    break;
                default:
                    break;
            }*/
            break;
        case PLAY_PAUSE_KEY:
            break;
        /*if necessary, add the handler for keys here */
        default:
            break;
    }

    return ret;
}
static QState MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_HANDLED();
    switch(e->keyId)
    {
        case BT_KEY:
            //MainApp_SendToMainApp(me,MAINAPP_FACTORY_RESET_SIG);
            break;
        /*if necessary, add the handler for keys here */
        default:
            break;
    }
    return ret;
}
static QState MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_HANDLED();
    switch(e->keyId)
    {
        case BT_KEY:
            switch(me->CurrBTStatus)
            {
                case BT_STREAMING_A2DP_STA:
                //BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PAUSE_CMD);
                case BT_CONNECTED_STA:      //disconnect & start pairing
                case BT_CONNECTABLE_STA:    //start pairing from connectable
                    if(me->BQBTestPlay)
                    {
                        //just for BQB test
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PLAY_CMD);
                        me->BQBTestPlay = FALSE;
                    }
                    else if(me->BQBTestVol)
                    {
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PAUSE_CMD);
                        me->BQBTestVol = FALSE;
                    }
                    else
                    {
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                    }
#ifdef HAS_BT_PAIRING_FILTER
                    me->tickHandlers[TIMER_ID_PAIRING_FILTER_TIMEOUT].timer = MAINAPP_PAIRING_FILTER_TIMEOUT_IN_MS;
#endif
                    break;
                case BT_DISCOVERABLE_STA:   //stop pairing from pairing mode
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return ret;
}

static QState MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {
        case BT_KEY:
            break;
        case PLAY_PAUSE_KEY:
            //To do
            //if sp 2 times already,
            //do fast forward
            //if sp 3 times
            //do fast rewind
            break;
        /*if necessary, add the handler for more keys here */
        default:
            break;
    }
    return ret;
}

static QState MainApp_KeyVLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_HANDLED();
    switch(e->keyId)
    {
        case BT_KEY:
            MainApp_SendToMainApp(me,MAINAPP_FACTORY_RESET_SIG);
            break;
        case PLAY_PAUSE_KEY:
            break;
        /*if necessary, add the handler for more keys here */
        default:
            break;
    }
    return ret;
}

static QState MainApp_CombineKeyHandler(cMainApp * const me, KeyStateEvt const * const e)
{

    QState ret= Q_UNHANDLED();
    switch(e->keyId)
    {
        case FACTORY_RESET_COMB:
            break;
        /*if necessary, add the handler for more keys here */
        default:
            break;
    }

    return ret;

}

static QState MainApp_VolumeKnobHander(cMainApp * const me, KeyStateEvt const * const e)
{
#ifdef KNOB_KEY_INVERT_VALUE
    e->index = MAX_VOLUME_STEPS - 1 - e->index;
#endif
    me->absoluteVol = e->index;         /* backup current volume whenever changed volume knob */
    //TYMQP_DUMP_QUEUE_WITH_LOG(me,"%d,%d",e->index,e->adcRawValue);
    if (FALSE == me->isCuePlaying)
    {
        // we should postpone volume adjust after cue playing.

        Setting_Set(SETID_VOLUME, &(e->index));
        AudioSrv_SetVolume(e->index);
    }

    return Q_HANDLED();
}

static QState MainApp_BASSKnobHander(cMainApp * const me, KeyStateEvt const * const e)
{
    static int step = 0;
#ifdef KNOB_KEY_INVERT_VALUE
    evt->index = MAX_BASS_STEPS - 1 - e->index;
#endif
    TYMQP_DUMP_QUEUE_WITH_LOG(me,"%d",e->index);
    Setting_Set(SETID_BASS, &(e->index));
    AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, e->index, 0);
     //JUST FOR BQB TEST
    if((e->index == 5)&&(step == 0))
    {
       step = 1;
    }
    else if((e->index == 6)&&(step == 1))
    {
       step = 2;
    }
    else if((e->index == 5)&&(step == 2))
    {
       step = 3;
    }
    else if((e->index == 6)&&(step == 3))
    {
       me->BQBTestPlay = TRUE;
    }
    else
    {
       step =0;
    }
    return Q_HANDLED();
}

static QState MainApp_TREBLEKnobHander(cMainApp * const me, KeyStateEvt const * const e)
{
    static int step = 0;
#ifdef KNOB_KEY_INVERT_VALUE
    evt->index = MAX_TREBLE_STEPS - 1 - e->index;
#endif
    TYMQP_DUMP_QUEUE_WITH_LOG(me,"%d",e->index);
    Setting_Set(SETID_TREBLE, &(e->index));
    AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, e->index, 0);
     //JUST FOR BQB TEST
    if((e->index == 5)&&(step == 0))
    {
       step = 1;
    }
    else if((e->index == 6)&&(step == 1))
    {
       step = 2;
    }
    else if((e->index == 5)&&(step == 2))
    {
       step = 3;
    }
    else if((e->index == 6)&&(step == 3))
    {
       me->BQBTestVol = TRUE;
    }
    else
    {
       step =0;
    }

    return Q_HANDLED();
}



QState MainApp_KnobHandler(cMainApp * const me, KeyStateEvt const * const evt)
{
    QState ret = Q_UNHANDLED();
    switch(evt->keyId)
    {
        case VOLUME_KNOB_BASE_KEY_ID:
            ret = MainApp_VolumeKnobHander(me, evt);
            break;
        case BASS_KNOB_BASE_KEY_ID:
            ret = MainApp_BASSKnobHander(me, evt);
            break;
        case TREBLE_KNOB_BASE_KEY_ID:
            ret = MainApp_TREBLEKnobHander(me, evt);
            break;
        default:
            break;
    }
    return ret;

}

QState MainApp_KeyHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    KeyStateEvt *evt = (KeyStateEvt*)e;

    if( MainApp_KnobHandler(me,evt) == Q_HANDLED() )
        return ret;

    switch(evt->keyEvent)
    {
        case KEY_EVT_UP:
            ret = MainApp_KeyUpEvtAction(me, evt);
            break;
        case KEY_EVT_DOWN:
            ret = MainApp_KeyDownEvtAction(me, evt);
            break;
        case KEY_EVT_SHORT_PRESS:
            ret = MainApp_KeySPressEvtAction(me, evt);
            break;
        case KEY_EVT_LONG_PRESS:
            ret= MainApp_KeyLPressEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_PRESS:
            ret= MainApp_KeyVLPressEvtAction(me, evt);
            break;
        case KEY_EVT_HOLD:
            ret= MainApp_KeyHoldEvtAction(me, evt);
            break;
        case KEY_EVT_REPEAT:
            ret= MainApp_KeyRepeatEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_HOLD:
            ret= MainApp_KeyVLHoldEvtAction(me, evt);
            break;
        case COMB_KEY_EVT:
            ret =  MainApp_CombineKeyHandler(me,evt);
        default:
            break;
    }


    return ret;

}

