/**
*  @file      MainApp_KeyEvtHandler.c
*  @brief     Key event handler of mainApp
*  @author    Viking Wang
*  @date      26-May-2016
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
#include "CommSrv.h"
#include "tym_qp_lib.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "GroupPattern.Config"
#include "DspDrv21584.h"

extern cDSPDrv21584 dspAdsp21584Drv;
#define TEST_LEDGROUP    0
#define  SOURCE_TO_LED_ID(_x)  ((_x) - AUDIO_SOURCE_AUXIN + LED_IND_ID_INPUT_AUXIN)
/*****************************************************************
 * Global Variable
 *****************************************************************/


/*****************************************************************
 * Function Implemenation
 *****************************************************************/

static QState MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_HANDLED();
    switch(e->keyId)
    {
        case STANDBY_KEY:
        {
            TYM_CLR_BIT(me->combinedKey, POWER_IN_HOLD);
            TYM_CLR_BIT(me->combinedKey, POWER_IN_L_HOLD);
            break;
        }
        case VOLUME_DOWN_KEY:
        {
            TYM_CLR_BIT(me->combinedKey, VOL_DOWN_KEY_IN_HOLD);
            TYM_CLR_BIT(me->combinedKey, VOL_DOWN_KEY_IN_L_HOLD);
            break;
        }
        case VOLUME_UP_KEY:
        {
            TYM_CLR_BIT(me->combinedKey, VOL_UP_KEY_IN_HOLD);
            TYM_CLR_BIT(me->combinedKey, VOL_UP_KEY_IN_L_HOLD);
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:
            ret = Q_UNHANDLED();
            break;
    }
    return ret;
}
static QState MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_HANDLED();
    switch(e->keyId)
    {
        case STANDBY_KEY:
        {
            TYM_SET_BIT(me->combinedKey, POWER_IN_HOLD);
            break;
        }
        case VOLUME_DOWN_KEY:
        {
            TYM_SET_BIT(me->combinedKey, VOL_DOWN_KEY_IN_HOLD);
            break;
        }
        case VOLUME_UP_KEY:
        {
            TYM_SET_BIT(me->combinedKey, VOL_UP_KEY_IN_HOLD);
            break;
        }
        /*if necessary, add the handler for keys here */
        default:
            ret = Q_UNHANDLED();
            break;
    }
    return ret;
}

static QState MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    eAudioSource channel;
    QState ret = Q_HANDLED();
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            if ( me->systemStatus == SYSTEM_STA_ON )
            {
                uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                uint8 volStep;
                volStep = (vol > MAINAPP_SWITCH_STEP_VOL)? MAINAPP_VOL_STEP1 : MAINAPP_VOL_STEP2;
                TP_PRINTF("VOLUME_DOWN_KEY, relativeVol= %d\r\n", -volStep);
                ////// [ATOMOS] : add controller here
                MainApp_VolumeDown(me,volStep);
                //if(vol != MIN_VOLUME)
                //{
                    //MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                //}
#if (TEST_LEDGROUP==1)
                MainApp_SendLedReq2(me, LED_IND_ID_GROUP, knightRiderPattern, 30, volToOneLed_tbl);
#endif
            }
            break;
        }
        case VOLUME_UP_KEY:
        {

            if ( me->systemStatus == SYSTEM_STA_ON )
            {
                uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                uint8 volStep;
                volStep = (vol >=MAINAPP_SWITCH_STEP_VOL)? MAINAPP_VOL_STEP1 : MAINAPP_VOL_STEP2;
                TP_PRINTF("VOLUME_UP_KEY, relativeVol= %d\r\n", volStep);
                ////// [ATOMOS] : add controller here
                MainApp_VolumeUp(me,volStep);
                //if(vol != MAX_VOLUME)
                //{
                    //MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                //}
#if (TEST_LEDGROUP==1)
                MainApp_SendLedReq2(me, LED_IND_ID_GROUP, breathePattern, 30, volToOneLed_tbl);
#endif
            }
            break;
        }
        case STANDBY_KEY:
        {
#if 1         	
            channel = *((eAudioSource*)Setting_Get(SETID_AUDIO_SOURCE));
            if (channel == AUDIO_SOURCE_AUXIN)
            	channel = AUDIO_SOURCE_HDMI;
            else if (channel == AUDIO_SOURCE_HDMI)
            {
            	channel = AUDIO_SOURCE_BLUETOOTH;
            	
            }
            else if (channel == AUDIO_SOURCE_BLUETOOTH)
            	channel = AUDIO_SOURCE_OPTICAL;
            else
            	channel = AUDIO_SOURCE_AUXIN;            	
            	
            if (channel >= AUDIO_SOURCE_MAX)
               channel = AUDIO_SOURCE_AUXIN;
               
            if (channel == AUDIO_SOURCE_BLUETOOTH)
            	CommSrv_SendCmd((QActive*)me, COMM_BT_PAIRING_CMD);
            else
            	CommSrv_SendCmd((QActive*)me, COMM_BT_DISCONNECT_CMD);
            	 
            MainApp_SwitchAudioSource(me, channel);
            channel = channel - AUDIO_SOURCE_AUXIN + LED_IND_ID_INPUT_AUXIN;
            MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
            MainApp_SendLedReq(me,channel);
#else
            if(me->combinedKey == POWER_IN_HOLD)
            {
                TYM_CLR_BIT(me->combinedKey, POWER_IN_HOLD);
            }
            ////// [ATOMOS] : add controller here
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
            CommSrv_SendCmd((QActive*)me, COMM_BT_DISCONNECT_CMD);
#endif            
                       
           
#if (TEST_LEDGROUP==1)
            MainApp_SendLedReq2(me, LED_IND_ID_GROUP, volumePattern, 30, volToOneLed_tbl);
#endif
            break;
        }
        case SOURCE_SWITCH_IR_KEY :
        {
            DSPDrv21584_StartPlay(&dspAdsp21584Drv);    //just for temporary testing
            break;
            
        }
        case BT_KEY :
        {
            DSPDrv21584_StopPlay(&dspAdsp21584Drv);     //just for temporary testing
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:
            ret = Q_UNHANDLED();
            break;
    }

    return ret;
}

static QState MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_HANDLED();
    switch(e->keyId)
    {
        case STANDBY_KEY:
        {
            if(me->combinedKey == POWER_IN_L_HOLD)
            {
                TYM_CLR_BIT(me->combinedKey, POWER_IN_L_HOLD);
            }
            break;
        }
        /*if necessary, add the handler for keys here */
        default:
            ret = Q_UNHANDLED();
            break;
    }

    return ret;
}

static QState MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_HANDLED();
    switch(e->keyId)
    {
        /*if necessary, add the handler for keys here */
        default:
            ret = Q_UNHANDLED();
            break;
    }
    return ret;
}

static QState MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_HANDLED();
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            TYM_SET_BIT(me->combinedKey, VOL_DOWN_KEY_IN_L_HOLD);
            if ((me->systemStatus == SYSTEM_STA_ON) &&(me->combinedKey == COM_KEY_VOL_DOWN_HOLD))
            {
                uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                uint8 volStep;
                volStep = (vol > MAINAPP_SWITCH_STEP_VOL)? MAINAPP_VOL_STEP1 : MAINAPP_VOL_STEP2;
                TP_PRINTF("VOLUME_DOWN_KEY, relativeVol= %d\r\n", -volStep);
                ////// [ATOMOS] : add controller here
                if(vol != MIN_VOLUME)
                {
                    MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                }
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            TYM_SET_BIT(me->combinedKey, VOL_UP_KEY_IN_L_HOLD);
            if ((me->systemStatus == SYSTEM_STA_ON) &&(me->combinedKey == COM_KEY_VOL_UP_HOLD))
            {
                uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                uint8 volStep;
                volStep = (vol >=MAINAPP_SWITCH_STEP_VOL)? MAINAPP_VOL_STEP1 : MAINAPP_VOL_STEP2;
                TP_PRINTF("VOLUME_UP_KEY, relativeVol= %d\r\n", volStep);
                ////// [ATOMOS] : add controller here
                if(vol != MAX_VOLUME)
                {
                    MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                }
            }
            break;
        }
        case STANDBY_KEY:
        {
            MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
            ////// [ATOMOS] : add controller here
            break;
        }
        /*if necessary, add the handler for keys here */
        default:
            ret = Q_UNHANDLED();
            break;
    }

    return ret;
}

static QState MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_HANDLED();
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            if ((me->systemStatus == SYSTEM_STA_ON) &&(me->combinedKey == COM_KEY_VOL_DOWN_HOLD))
            {
                uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                uint8 volStep;
                volStep = (vol > MAINAPP_SWITCH_STEP_VOL)? MAINAPP_VOL_STEP1 : MAINAPP_VOL_STEP2;
                TP_PRINTF("VOLUME_DOWN_KEY, relativeVol= %d\r\n", -volStep);
                ////// [ATOMOS] : add controller here
                if(vol != MIN_VOLUME)
                {
                    MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                }
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            if ((me->systemStatus == SYSTEM_STA_ON) &&(me->combinedKey == COM_KEY_VOL_UP_HOLD))
            {
                uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                uint8 volStep;
                volStep = (vol >=MAINAPP_SWITCH_STEP_VOL)? MAINAPP_VOL_STEP1 : MAINAPP_VOL_STEP2;
                TP_PRINTF("VOLUME_UP_KEY, relativeVol= %d\r\n", volStep);
                ////// [ATOMOS] : add controller here
                if(vol != MAX_VOLUME)
                {
                    MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                }
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:
            ret = Q_UNHANDLED();
            break;
    }
    return ret;
}

static QState MainApp_KeyVLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_HANDLED();
    switch(e->keyId)
    {
        case STANDBY_KEY:
        {
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:
            ret = Q_UNHANDLED();
            break;
    }

    return ret;
}

static void MainApp_CombineKeyHandler(cMainApp * const me, QEvt const * const e, eCombinedKey comKey)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(comKey)
    {
        case COM_KEY_STANDBY:
        {
            break;
        }
        default:break;
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
        default:break;
    }

    MainApp_CombineKeyHandler(me, e, me->combinedKey);
    return ret;

}

