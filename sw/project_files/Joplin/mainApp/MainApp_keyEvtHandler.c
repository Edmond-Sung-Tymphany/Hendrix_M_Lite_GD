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

#define FACTORY_RESET_COMB      COMB_KEY_ID_0

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
        case PLAY_PAUSE_KEY:
            if (me->combKey == SKIP_FAST_FW_COMB)
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_FAST_FORWARD_STOP_REQ);
                me->combKey = COM_KEY_INVALID;
            }
            else if (me->combKey == SKIP_REWIND_COMB)
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_REWIND_STOP_REQ);
                me->combKey = COM_KEY_INVALID;
            }
            else
            {
                TYM_CLR_BIT(me->combKey, COM_KEY_ON_HOLD);
            }
            break;
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
        case PLAY_PAUSE_KEY:
            TYM_SET_BIT(me->combKey, COM_KEY_ON_HOLD);
            break;

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
            //just for test
            //BluetoothSrv_SendBtCmd((QActive*)me, BT_GET_ADDRESS_REQ);
            break;
        }
        case PLAY_PAUSE_KEY:
        {
            TYM_CLR_BIT(me->combKey, COM_KEY_ON_HOLD);
            if (me->audioSource == AUDIO_SOURCE_BT)
            {
                me->combKey++;
                me->tickHandlers[TIMER_ID_BT_COMB_KEY_TIMEOUT].timer = MAINAPP_BT_COMB_KEY_TIMEOUT;
            }
            else
            {
                me->combKey = COM_KEY_INVALID;
                me->tickHandlers[TIMER_ID_BT_COMB_KEY_TIMEOUT].timer = 0;
#ifdef HAS_MUTE_KEY
                /* This is just ES testing code. */
                me->muteStatus =!(me->muteStatus);
                AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, me->muteStatus);
#endif
            }
            break;
        }
        case INPUT_KEY:
        {
            if (!me->isCuePlaying)
            {
                eAudioSource nextSrc;
                nextSrc = MainApp_GetNextAvailableSource(me);
                MainApp_SwitchAudioSource(me, nextSrc, FALSE);
                BluetoothSrv_SendBtCmdWithParam((QActive*)me, MCU_SOURCE_CHANGE_EVENT, sizeof(eAudioSource), (uint8 *)&nextSrc);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            if((me->vol/CLICK_NUM_PER_VOLUME_STEP) < (MAX_VOLUME_STEPS-1))
            {
                me->vol++;
                if (!(me->vol%CLICK_NUM_PER_VOLUME_STEP))
                {
                    uint32 vol;
                    vol = me->vol/CLICK_NUM_PER_VOLUME_STEP;
                    AudioSrv_SetVolume(me->vol/CLICK_NUM_PER_VOLUME_STEP);
                    MainApp_SetRotaterLedOn(me, LED_VOL_0, vol_led_mapping[me->vol/CLICK_NUM_PER_VOLUME_STEP]);
                    MainApp_UpdateVolumeSyncTimer(me);
                    TP_PRINTF("bt->Mcu: volume = %d\r\n", vol);
                }
            }
            break;
        }
        case BASS_UP_KEY:
            if((me->bass/CLICK_NUM_PER_BASS_STEP) < (MAX_BASS_STEPS-1))
            {
                me->bass++;
                if (!(me->bass%CLICK_NUM_PER_BASS_STEP))
                {
                    AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, me->bass/CLICK_NUM_PER_BASS_STEP, NULL);
                    MainApp_SetRotaterLedOn(me, LED_BAS_0, me->bass/CLICK_NUM_PER_BASS_STEP);
                }
            }
            break;
        case TREBLE_UP_KEY:
            if((me->treble/CLICK_NUM_PER_TREBLE_STEP) < (MAX_TREBLE_STEPS-1))
            {
                me->treble++;
                if (!(me->treble%CLICK_NUM_PER_TREBLE_STEP))
                {
                    AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, me->treble/CLICK_NUM_PER_TREBLE_STEP, NULL);
                    MainApp_SetRotaterLedOn(me, LED_TRE_0, me->treble/CLICK_NUM_PER_TREBLE_STEP);
                }
            }
            break;

        case VOLUME_DOWN_KEY:
        {
            if(me->vol > 0)
            {
                me->vol--;
                if (!(me->vol%CLICK_NUM_PER_VOLUME_STEP))
                {
                    uint32 vol;
                    vol = me->vol/CLICK_NUM_PER_VOLUME_STEP;
                    AudioSrv_SetVolume(me->vol/CLICK_NUM_PER_VOLUME_STEP);
                    MainApp_SetRotaterLedOff(me, LED_VOL_0, vol_led_mapping[me->vol/CLICK_NUM_PER_VOLUME_STEP]);
                    MainApp_UpdateVolumeSyncTimer(me);
                    TP_PRINTF("bt->Mcu: volume = %d\r\n", vol);
                }
            }
            break;
        }
        case BASS_DOWN_KEY:
            if(me->bass > 0)
            {
                me->bass--;
                if (me->bass%CLICK_NUM_PER_BASS_STEP)
                {
                    AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, me->bass/CLICK_NUM_PER_BASS_STEP, NULL);
                    MainApp_SetRotaterLedOff(me, LED_BAS_0, me->bass/CLICK_NUM_PER_BASS_STEP);
                }
            }
            break;

        case TREBLE_DOWN_KEY:
            if(me->treble > 0)
            {
                me->treble--;
                if (me->treble%CLICK_NUM_PER_TREBLE_STEP)
                {
                    AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, me->treble/CLICK_NUM_PER_TREBLE_STEP, NULL);
                    MainApp_SetRotaterLedOff(me, LED_TRE_0, me->treble/CLICK_NUM_PER_TREBLE_STEP);
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
                if (me->connState != Proto_BtState_ConnState_PAIRING)
                {
                    bool param = TRUE;
                    BluetoothSrv_SendBtCmdWithParam((QActive*)me, BT_ENTER_PAIRING_REQ, sizeof(param) ,&param);
                    MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_ENABLED);
                }
                else
                {
                    bool param = FALSE;
                    BluetoothSrv_SendBtCmdWithParam((QActive*)me, BT_ENTER_PAIRING_REQ,sizeof(param) ,&param);
                    MainApp_SendLedReq(me, LED_IND_ID_BT_CONNECTABLE);
                }
            }
            break;
        }
        case PLAY_PAUSE_KEY:
        {
            if (me->combKey == (PLAY_PAUSE_COMB | COM_KEY_ON_HOLD))
            {
                me->combKey = SKIP_FAST_FW_COMB;
                BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_FAST_FORWARD_START_REQ);
            }
            else if (me->combKey == (NEXT_TRACK_COMB | COM_KEY_ON_HOLD))
            {
                me->combKey = SKIP_REWIND_COMB;
                BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_REWIND_START_REQ);
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
            break;
        }

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
        {
            ret = Q_TRAN(&MainApp_FactoryReset);
            break;
        }
        default:
            break;
    }

    return ret;
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
        case COMB_KEY_EVT:
            ret =  MainApp_CombineKeyHandler(me,evt);
        default:
            break;
    }

    return ret;

}

