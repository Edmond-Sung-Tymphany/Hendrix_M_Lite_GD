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
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "tym_qp_lib.h"
#include "MainApp_priv.h"

/*****************************************************************
 * Global Variable
 *****************************************************************/
const uint8 vol_led_mapping[MAX_VOLUME_STEPS] = 
{
    0,
    1,
    1,
    1,
    2,
    2,
    2,
    3,
    3,
    3,
    4,
    4,
    4,
    5,
    5,
    5,
    6,
    6,
    6,
    6,
    7,
    7,
    7,
    7,
    8,
    8,
    8,
    8,
    9,
    9,
    9,
    9,
    10,
};

/*****************************************************************
 * Function Implemenation
 *****************************************************************/

static QState MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {

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

        /*if necessary, add the handler for keys here */
        default:
            break;
    }
    return ret;
}

static QState MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    uint8 i;
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            //BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_NEXT_CMD);
            break;
        }
        case PLAY_PAUSE_KEY:
        {
            if (me->audioSource == AUDIO_SOURCE_BT)
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PLAY_PAUSE_CMD);
            }
            else
            {
                me->muteStatus =!(me->muteStatus);
                AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, me->muteStatus);
            }
            break;
        }
        case INPUT_KEY:
        {
            eAudioSource nextSrc;
            me->audioSource++;
            nextSrc = MainApp_GetNextAvialableSource(me);
            MainApp_SwitchAudioSource(me, nextSrc);
            break;
        }
        case VOLUME_UP_KEY:
        {
            if((me->vol/2) < (MAX_VOLUME_STEPS-1))
            {
                me->vol++;
                /* Only update if it is multiple of 2 */
                if (!(me->vol%2))
                {
                    AudioSrv_SetVolume(me->vol/2);
                    MainApp_SetRotaterLedOn(me, LED_VOL_0, vol_led_mapping[me->vol/2]);
                }
            }
            break;
        }
        case BASS_UP_KEY:
            if((me->bass/2) < (MAX_BASS_STEPS-1))
            {
                me->bass++;
                /* Only update if it is multiple of 2 */
                if (!(me->bass%2))
                {
                    AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, me->bass/2, NULL);
                    MainApp_SetRotaterLedOn(me, LED_BAS_0, me->bass/2);
                }
            }
            break;
        case TREBLE_UP_KEY:
            if((me->treble/2) < (MAX_TREBLE_STEPS-1))
            {
                me->treble++;
                /* Only update if it is multiple of 2 */
                if (!(me->treble%2))
                {
                    AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, me->treble/2, NULL);
                    MainApp_SetRotaterLedOn(me, LED_TRE_0, me->treble/2);
                }
            }
            break;

        case VOLUME_DOWN_KEY:
        {
            if(me->vol > 0)
            {
                me->vol--;
                /* Only update if it is multiple of 2 */
                if (!(me->vol%2))
                {
                    AudioSrv_SetVolume(me->vol/2);
                    MainApp_SetRotaterLedOff(me, LED_VOL_0, vol_led_mapping[me->vol/2]);
                }
            }
            break;
        }
        case BASS_DOWN_KEY:
            if(me->bass > 0)
            {
                me->bass--;
                /* Only update if it is multiple of 2 */
                if (!(me->bass%2))
                {
                    AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, me->bass/2, NULL);
                    MainApp_SetRotaterLedOff(me, LED_BAS_0, me->bass/2);
                }
            }
            break;

        case TREBLE_DOWN_KEY:
            if(me->treble > 0)
            {
                me->treble--;
                /* Only update if it is multiple of 2 */
                if (!(me->treble%2))
                {
                    AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, me->treble/2, NULL);
                    MainApp_SetRotaterLedOff(me, LED_TRE_0, me->treble/2);
                }
            }
            break;

        /*if necessary, add the handler for more keys here */
        default:
            break;
    }

    return ret;
}

static QState MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    switch(e->keyId)
    {
        /*if necessary, add the handler for keys here */
        default:
            break;
    }

    return ret;
}
static QState MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {
        /*if necessary, add the handler for keys here */
        default:
            break;
    }
    return ret;
}
static QState MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {
        case INPUT_KEY:
        {
            if (me->audioSource == AUDIO_SOURCE_BT)
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_ENABLED);
            }
            break;
        }
        /*if necessary, add the handler for keys here */
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

        /*if necessary, add the handler for more keys here */
        default:
            break;
    }
    return ret;
}

static QState MainApp_KeyVLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    switch(e->keyId)
    {
        case INPUT_KEY:
        {
            /* Very long hold input key to active RCA source for Joplin M model.  */
            bool isRcaActivated = *(bool *)Setting_Get(SETID_IS_RCA_ACTIVATED);
            if (isRcaActivated)
            {
                isRcaActivated = FALSE;
                me->sourceHandler[AUDIO_SOURCE_RCA].bIsValid = FALSE;
            }
            else
            {
                isRcaActivated = TRUE;
                me->sourceHandler[AUDIO_SOURCE_RCA].bIsValid = TRUE;
            }

            Setting_Set(SETID_IS_RCA_ACTIVATED, &isRcaActivated);
            eAudioSource nextSrc;
            me->audioSource = AUDIO_SOURCE_RCA;
            nextSrc = MainApp_GetNextAvialableSource(me);
            MainApp_SwitchAudioSource(me, nextSrc);
            break;
        }

        /*if necessary, add the handler for more keys here */
        default:
            break;
    }

    return ret;
}

static void MainApp_CombineKeyHandler(cMainApp * const me, QEvt const * const e, eCombinedKey comKey)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(comKey)
    {

        default:
            break;
    }
}

QState MainApp_KeyHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    KeyStateEvt *evt = (KeyStateEvt*)e;
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
        default:
            break;
    }

    MainApp_CombineKeyHandler(me, e, me->combinedKey);
    return ret;

}

