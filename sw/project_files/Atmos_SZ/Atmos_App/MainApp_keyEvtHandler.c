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
#include "tym_qp_lib.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"

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
                if(vol != MIN_VOLUME)
                {
                    MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                }
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
                if(vol != MAX_VOLUME)
                {
                    MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                }
            }
            break;
        }
        case STANDBY_KEY:
        {
            if(me->combinedKey == POWER_IN_HOLD)
            {
                TYM_CLR_BIT(me->combinedKey, POWER_IN_HOLD);
            }
            ////// [ATOMOS] : add controller here
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
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

