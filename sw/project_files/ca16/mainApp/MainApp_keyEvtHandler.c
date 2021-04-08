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
#include "AseTkSrv.h"
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
typedef enum
{
    VOL_ROTATION_FREE = 0,
    VOL_ROTATION_START_TURNING,
    VOL_ROTATION_IN_TURNING,
    VOL_ROTATION_UP_HOLD,
    VOL_ROTATION_UP_RELEASE,
    VOL_ROTATION_DOWN_HOLD,
    VOL_ROTATION_DOWN_RELEASE,
}eVolRotationState;
static eVolRotationState volRotationSta= VOL_ROTATION_FREE;

const uint8 VolTable[CUST_MAX_VOL+1] =
{
    0, 20, 30, 35, 40, 43, 46, 49, 52, 55,
    59, 61, 63, 65, 67, 69, 71, 73, 75, 77,
    78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
    88, 89, 90
};

/*****************************************************************
 * Function Implemenation
 *****************************************************************/

static uint8 MainApp_AseTkVolToCustVol(uint8 aseTkVol)
{
    uint8 i;
    for (i = MIN_VOLUME; i < CUST_MAX_VOL; i++)
    {
        if ((aseTkVol >= VolTable[i]) && (aseTkVol < VolTable[i+1]))
        {
            break;
        }
    }
    return i;
}

static uint8 MainApp_VolumeDownOneStep(cMainApp * const me)
{
    uint8 custVol= 0;

    if (me->absoluteVol == INVALID_VALUE)
    {
        me->absoluteVol = *(uint8*)Setting_Get(SETID_VOLUME);
    }

    if(me->absoluteVol <= MIN_VOLUME)
    {
        TP_PRINTF("VOLUME_DOWN_KEY, already reached minimum volume absoluteVol= %d\r\n", me->absoluteVol);
    }
    else
    {
        custVol = MainApp_AseTkVolToCustVol(me->absoluteVol);
        if (me->absoluteVol <= VolTable[custVol])
        {
            me->absoluteVol = VolTable[custVol-1];
        }
        else
        {
            me->absoluteVol = VolTable[custVol];
        }
        me->isVolChanged = TRUE;
        TP_PRINTF("VOLUME_DOWN_KEY, absoluteVol= %d\r\n", me->absoluteVol);
        MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
    }
}

static uint8 MainApp_VolumeUpOneStep(cMainApp * const me)
{
    uint8 custVol= 0;

    if (me->absoluteVol == INVALID_VALUE)
    {
        me->absoluteVol = *(uint8*)Setting_Get(SETID_VOLUME);
    }

    if(me->absoluteVol >= MAX_VOLUME)
    {
        TP_PRINTF("VOLUME_UP_KEY, already reached maximun volume absoluteVol= %d\r\n", me->absoluteVol);
    }
    else
    {
        custVol = MainApp_AseTkVolToCustVol(me->absoluteVol);
        me->absoluteVol = VolTable[custVol+1];
        me->isVolChanged = TRUE;
        TP_PRINTF("VOLUME_UP_KEY, absoluteVol= %d\r\n", me->absoluteVol);
        MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
    }
}

static QState MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
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
#ifdef SUPPORT_NEW_TOP_PANEL
            if((volRotationSta == VOL_ROTATION_START_TURNING) && (me->combinedKey == COM_KEY_IN_TURNING))
            {
                volRotationSta = VOL_ROTATION_UP_HOLD;
            }
            else if ((me->combinedKey == VOL_DOWN_KEY_IN_HOLD) || (me->combinedKey == COM_KEY_VOL_DOWN_HOLD))
            {
                volRotationSta = VOL_ROTATION_FREE;
            }
#endif
            TYM_CLR_BIT(me->combinedKey, VOL_DOWN_KEY_IN_HOLD);
            TYM_CLR_BIT(me->combinedKey, VOL_DOWN_KEY_IN_L_HOLD);

            break;
        }
        case VOLUME_UP_KEY:
        {
#ifdef SUPPORT_NEW_TOP_PANEL
            if((volRotationSta == VOL_ROTATION_START_TURNING) && (me->combinedKey == COM_KEY_IN_TURNING))
            {
                volRotationSta = VOL_ROTATION_DOWN_HOLD;
            }
#endif
            TYM_CLR_BIT(me->combinedKey, VOL_UP_KEY_IN_HOLD);
            TYM_CLR_BIT(me->combinedKey, VOL_UP_KEY_IN_L_HOLD);
            break;
        }
        case CONNECT_KEY:
        {
            TYM_CLR_BIT(me->combinedKey, CONNECT_IN_HOLD);
            TYM_CLR_BIT(me->combinedKey, CONNECT_IN_VL_HOLD);
            break;
        }
        case CONFIG_KEY:
        {
            TYM_CLR_BIT(me->combinedKey, CONFIG_IN_HOLD);
            TYM_CLR_BIT(me->combinedKey, CONFIG_IN_VL_HOLD);
            break;
        }        /*if necessary, add the handler for more keys here */
        default:break;
    }
    return ret;
}
static QState MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
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
#ifdef SUPPORT_NEW_TOP_PANEL
            if((volRotationSta == VOL_ROTATION_FREE) && (me->combinedKey == COM_KEY_IN_TURNING))
            {
                volRotationSta = VOL_ROTATION_START_TURNING;
            }
            else if((volRotationSta == VOL_ROTATION_UP_HOLD) && (me->combinedKey == COM_KEY_IN_TURNING))
            {
                volRotationSta = VOL_ROTATION_UP_RELEASE;
            }
#endif
            break;
        }
        case VOLUME_UP_KEY:
        {
            TYM_SET_BIT(me->combinedKey, VOL_UP_KEY_IN_HOLD);
#ifdef SUPPORT_NEW_TOP_PANEL
            if((volRotationSta == VOL_ROTATION_FREE) && (me->combinedKey == COM_KEY_IN_TURNING))
            {
                volRotationSta = VOL_ROTATION_START_TURNING;
            }
            else if((volRotationSta == VOL_ROTATION_DOWN_HOLD) && (me->combinedKey == COM_KEY_IN_TURNING))
            {
                volRotationSta = VOL_ROTATION_DOWN_RELEASE;
            }
#endif
            break;
        }
        case CONNECT_KEY:
        {
            TYM_SET_BIT(me->combinedKey, CONNECT_IN_HOLD);
            break;
        }
        case CONFIG_KEY:
        {
            TYM_SET_BIT(me->combinedKey, CONFIG_IN_HOLD);
            break;
        }
        /*if necessary, add the handler for keys here */
        default:break;
    }
    return ret;
}

static QState MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
#ifdef SUPPORT_NEW_TOP_PANEL
            /* TODO: need further tuning.  */
            if ((me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
             && (volRotationSta == VOL_ROTATION_DOWN_RELEASE)
            )
            {
                volRotationSta = VOL_ROTATION_FREE;
                MainApp_VolumeDownOneStep(me);
            }
#else
            if (me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
            {
                MainApp_VolumeDownOneStep(me);
            }
#endif
            break;
        }
        case VOLUME_UP_KEY:
        {
#ifdef SUPPORT_NEW_TOP_PANEL
            /* TODO: need further tuning.  */
            if ((me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
            &&(volRotationSta == VOL_ROTATION_UP_RELEASE)
            )
            {
                volRotationSta = VOL_ROTATION_FREE;
                MainApp_VolumeUpOneStep(me);
            }
#else
            if (me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
            {
                MainApp_VolumeUpOneStep(me);
            }
#endif
            break;
        }
        case CONNECT_KEY:
        {
            /* Note: Below is testing code for ES1 HW bring up, please remove when make formal release */
#ifdef ENABLE_CA16_ES_TESTING_CODE
            me->audioSource++;
            MainApp_SwitchAudioSource(me, me->audioSource);
            MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
            AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, FALSE, /* NO USED*/0);
#endif //#ifdef ENABLE_CA16_PRE_ES_TESTING_CODE
            if (me->combinedKey == CONNECT_IN_HOLD)
            {
                //MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                if (me->tickHandlers[TIMER_ID_DOUBLE_PRESS_TIMEOUT].timer <= 0)
                {
                    me->tickHandlers[TIMER_ID_DOUBLE_PRESS_TIMEOUT].timer = MAINAPP_DOUBLE_PRESS_TIMEOUT_IN_MS;
                }
                else /* Doudble press */
                {
                    me->tickHandlers[TIMER_ID_DOUBLE_PRESS_TIMEOUT].timer = INVALID_VALUE;
                    TP_PRINTF("FepAseCommand_Command_NEXT_SOURCE\r\n");
                    AseTkSrv_SendFepAseCmd(FepAseCommand_Command_NEXT_SOURCE);
                }
            }
            break;
        }
        case STANDBY_KEY:
        {
            if(me->combinedKey == POWER_IN_HOLD)
            {
                TYM_CLR_BIT(me->combinedKey, POWER_IN_HOLD);
                //ret = Q_TRAN(&MainApp_NetworkStandby);
            }
            AseTkSrv_SendFepAseCmd(FepAseCommand_Command_STANDBY);
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
            break;
        }
        case CONFIG_KEY:
        {
            TP_PRINTF("FepAseCommand_Command_NETWORK_SETUP\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);

            AsetkNetworkInfo* pNetInfo= AseTkSrv_GetDecodedNetworkInfo();
            if (pNetInfo->ethernet_state == NetworkInfo_State_SCANNING ||
             pNetInfo->ethernet_state == NetworkInfo_State_CONNECTING ||
             pNetInfo->ethernet_state == NetworkInfo_State_AUTHENTICATING ||
             pNetInfo->ethernet_state == NetworkInfo_State_ACQUIRING ||
             pNetInfo->ethernet_state == NetworkInfo_State_CONNECTED )
            {
                TP_PRINTF(" If current network is Ethernet, then should not trigger network setup. \r\n");
            }
            else
            {
                AseTkSrv_SendFepAseCmd(FepAseCommand_Command_NETWORK_SETUP);
                MainApp_SendLedReq(me, LED_IND_ID_WIFI_SETUP_IN_PROGRESS);
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }

    return ret;
}

static QState MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
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
        case CONFIG_KEY:
        {
            AsetkNetworkInfo* pNetInfo= AseTkSrv_GetDecodedNetworkInfo();
            if (pNetInfo->ethernet_state == NetworkInfo_State_SCANNING ||
             pNetInfo->ethernet_state == NetworkInfo_State_CONNECTING ||
             pNetInfo->ethernet_state == NetworkInfo_State_AUTHENTICATING ||
             pNetInfo->ethernet_state == NetworkInfo_State_ACQUIRING ||
             pNetInfo->ethernet_state == NetworkInfo_State_CONNECTED )
            {
                TP_PRINTF(" If current network is Ethernet, then should not trigger network setup. \r\n");
            }
            else if(me->combinedKey == CONFIG_IN_HOLD)
            {
                AseTkSrv_SendFepAseCmd(FepAseCommand_Command_NETWORK_SETUP);
                MainApp_SendLedReq(me, LED_IND_ID_WIFI_SETUP_IN_PROGRESS);
            }
            break;
        }
        /*if necessary, add the handler for keys here */
        default:break;
    }

    return ret;
}
static QState MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {
        /*if necessary, add the handler for keys here */
        default:break;
    }
    return ret;
}
static QState MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            TYM_SET_BIT(me->combinedKey, VOL_DOWN_KEY_IN_L_HOLD);
            volRotationSta = VOL_ROTATION_FREE;
            if ((me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
                &&(me->combinedKey == COM_KEY_VOL_DOWN_HOLD))
            {
                MainApp_VolumeDownOneStep(me);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            TYM_SET_BIT(me->combinedKey, VOL_UP_KEY_IN_L_HOLD);
            volRotationSta = VOL_ROTATION_FREE;
            if ((me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
                &&(me->combinedKey == COM_KEY_VOL_UP_HOLD))
            {
                MainApp_VolumeUpOneStep(me);
            }
            break;
        }
        case STANDBY_KEY:
        {
            MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
            AseTkSrv_SendFepAseCmd(FepAseCommand_Command_OFF);
            break;
        }
        case CONFIG_KEY:
        {
            MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
            break;
        }
        case CONNECT_KEY:
        {
            if (me->combinedKey == CONNECT_IN_HOLD)
            {
                MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
                AseTkSrv_SendFepAseCmd(FepAseCommand_Command_ALL_STANDBY);
            }
            break;
        }
        /*if necessary, add the handler for keys here */
        default:break;
    }
    return ret;
}

static QState MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            if ((me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
                &&(me->combinedKey == COM_KEY_VOL_DOWN_HOLD))
            {
                MainApp_VolumeDownOneStep(me);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            if ((me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
                &&(me->combinedKey == COM_KEY_VOL_UP_HOLD))
            {
                MainApp_VolumeUpOneStep(me);
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
    return ret;
}

static QState MainApp_KeyVLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    switch(e->keyId)
    {
        case CONNECT_KEY:
        {
            TYM_SET_BIT(me->combinedKey, CONNECT_IN_VL_HOLD);
            break;
        }
        case CONFIG_KEY:
        {
            TYM_SET_BIT(me->combinedKey, CONFIG_IN_VL_HOLD);
            if (me->combinedKey == (CONFIG_IN_HOLD | CONFIG_IN_VL_HOLD))
            {
                MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
                AseTkSrv_SendFepAseCmd(FepAseCommand_Command_FACTORY_RESET);
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }

    return ret;
}

static void MainApp_CombineKeyHandler(cMainApp * const me, QEvt const * const e, eCombinedKey comKey)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(comKey)
    {
#ifdef HAS_DSP_TUNING_MODE
        case COM_KEY_DSP_TUNING_MODE:
        {
            MainApp_SwitchAudioSource(me, AUDIO_SOURCE_DSP_TUNING);
            me->combinedKey = COM_KEY_INVALID;
            break;
        }
#endif
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

