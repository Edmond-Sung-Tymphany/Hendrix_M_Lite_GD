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
#include "AseNgSrv.h"
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
const uint8 VolTable[CUST_MAX_VOL+1] =
{
/* DSP index | dB value | volume step    */
    0,  //    -Infinity     0
    11, //    -79           1
    21, //    -69           2
    26, //    -64           3
    30, //    -60           4
    33, //    -57           5
    35, //    -55           6
    37, //    -53           7
    39, //    -51           8
    41, //    -49           9
    43, //    -47           10
    45, //    -45           11
    47, //    -43           12
    49, //    -41           13
    51, //    -39           14
    53, //    -37           15
    55, //    -35           16
    57, //    -33           17
    59, //    -31           18
    61, //    -29           19
    63, //    -27           20
    65, //    -25           21
    67, //    -23           22
    69, //    -21           23
    71, //    -19           24
    73, //    -17           25
    75, //    -15           26
    77, //    -13           27
    79, //    -11           28
    81, //    -9            29
    83, //    -7            30
    86, //    -4            31
    90, //    0             32
};


/*****************************************************************
 * Function Implemenation
 *****************************************************************/
static uint8 MainApp_AseNgVolToCustVol(uint8 aseVol)
{
    uint8 i;
    for (i = MIN_VOLUME; i < CUST_MAX_VOL; i++)
    {
        if ((aseVol >= VolTable[i]) && (aseVol < VolTable[i+1]))
        {
            break;
        }
    }
    return i;
}
                    
                    
static void MainApp_VolumeHandlerOnPause(cMainApp * const me)
{
    /* When music not play or ASE just boot, ASE may limit volume to 60 or do not handle volume change.
     * It let FEP/ASE volume un-synchronize and cause many issues.
     * Solution: set the same volume when music pause. It let ASE can unmute aux-in mute when press VOL+/-.
     */
    TP_PRINTF("VOLUME_KEY while music pause, send the same vol= %d\r\n", me->absoluteVol);
    AseNgSrv_SendFepAseCmdAbsoluteVol(me->absoluteVol);
    MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
}

                    
static void MainApp_VolumeDownOneStep(cMainApp * const me)
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
        custVol = MainApp_AseNgVolToCustVol(me->absoluteVol);
        if (me->absoluteVol <= VolTable[custVol])
        {
            me->absoluteVol = VolTable[custVol-1];
        }
        else
        {
            me->absoluteVol = VolTable[custVol];
        }
        TP_PRINTF("VOLUME_DOWN_KEY, vol= %d\r\n", me->absoluteVol);
        AseNgSrv_SendFepAseCmdAbsoluteVol(me->absoluteVol);

        //for debug
        if( me->audioMode==AUDIO_MODE_EXT_SOURCE )
        {
            AudioSrv_SetVolume(me->absoluteVol);
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
        }
    }
}

static void MainApp_VolumeUpOneStep(cMainApp * const me)
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
        custVol = MainApp_AseNgVolToCustVol(me->absoluteVol);
        me->absoluteVol = VolTable[custVol+1];
        TP_PRINTF("VOLUME_UP_KEY, vol= %d\r\n", me->absoluteVol);
        AseNgSrv_SendFepAseCmdAbsoluteVol(me->absoluteVol);

        if( me->audioMode==AUDIO_MODE_EXT_SOURCE )
        {
            AudioSrv_SetVolume(me->absoluteVol);
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
        }
    }
}

static QState MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            TYM_CLR_BIT(me->combinedKey, VOL_DOWN_COMB_DOWN | VOL_DOWN_IN_DOWN);
            break;
        }
        case VOLUME_UP_KEY:
        {
            TYM_CLR_BIT(me->combinedKey, VOL_UP_COMB_DOWN | VOL_UP_IN_DOWN);
            //TP_PRINTF("******************************************* VOLUME_UP_KEY up, %d, c=0x%02x\r\n\r\n", vol_up, me->combinedKey);
            break;
        }
        case CONNECT_KEY:
        {
            TYM_CLR_BIT(me->combinedKey, CONNECT_COMB_DOWN | CONNECT_IN_DOWN);
            //TP_PRINTF("******************************************* CONNECT_KEY up, %d, c=0x%02x\r\n\r\n", connect, me->combinedKey);
            break;
        }
        case STANDBY_KEY:
        {
            TYM_CLR_BIT(me->combinedKey, STANDBY_COMB_DOWN | STANDBY_IN_DOWN);
            break;
        }
        case RESET_KEY:
        {
            TYM_CLR_BIT(me->combinedKey, RESET_COMB_DOWN | RESET_IN_DOWN);
            break;
        }
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
            TYM_SET_BIT(me->combinedKey, STANDBY_IN_DOWN);
            break;
        }
        case RESET_KEY:
        {
            TYM_SET_BIT(me->combinedKey, RESET_IN_DOWN);
            break;
        }
        case VOLUME_DOWN_KEY:
        {
            TYM_SET_BIT(me->combinedKey, VOL_DOWN_IN_DOWN);
            break;
        }
        case VOLUME_UP_KEY:
        {
            TYM_SET_BIT(me->combinedKey, VOL_UP_IN_DOWN);
            //TP_PRINTF("******************************************* VOLUME_UP_KEY down, %d, c=0x%02x\r\n\r\n", vol_up, me->combinedKey);
            break;
        }
        case CONNECT_KEY:
        {
            TYM_SET_BIT(me->combinedKey, CONNECT_IN_DOWN);
            //TP_PRINTF("******************************************* CONNECT_KEY down, %d, c=0x%02x\r\n\r\n", connect, me->combinedKey);
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
            if ( me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE )
            {
                if( me->musicPlaying )
                {
                    MainApp_VolumeDownOneStep(me);
                }
                else
                {
                    MainApp_VolumeHandlerOnPause(me);          
                }
            }
            else if (me->audioMode==AUDIO_MODE_EXT_SOURCE)
            {
                MainApp_VolumeDownOneStep(me);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            if ( me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE )
            {
                if( me->musicPlaying )
                {
                    MainApp_VolumeUpOneStep(me);
                }
                else
                {
                    MainApp_VolumeHandlerOnPause(me);           
                }
            }
            else if (me->audioMode==AUDIO_MODE_EXT_SOURCE)
            {
                MainApp_VolumeUpOneStep(me);
            }
            break;
        }
        case CONNECT_KEY:
        {
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
            if (me->tickHandlers[TIMER_ID_DOUBLE_PRESS_CONNECT_TIMEOUT].timer <= 0)
            {
                me->tickHandlers[TIMER_ID_DOUBLE_PRESS_CONNECT_TIMEOUT].timer = MAINAPP_DOUBLE_PRESS_TIMEOUT_IN_MS;
            }
            else /* Doudble press */
            {
                me->tickHandlers[TIMER_ID_DOUBLE_PRESS_CONNECT_TIMEOUT].timer = INVALID_VALUE;
                TP_PRINTF("Proto_FepAse_ReqResp_NEXT_SOURCE\r\n");
                MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_NEXT_SOURCE);
            }
            break;
        }
        case STANDBY_KEY:
        { 
            TP_PRINTF("Proto_FepAse_ReqResp_STANDBY\r\n");
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_STANDBY);
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
            break;
        }
        case RESET_KEY:
        {
            if (me->tickHandlers[TIMER_ID_DOUBLE_PRESS_RESET_TIMEOUT].timer <= 0)
            {
                me->tickHandlers[TIMER_ID_DOUBLE_PRESS_RESET_TIMEOUT].timer = MAINAPP_DOUBLE_PRESS_TIMEOUT_IN_MS;
            }
            else /* Doudble press */
            {
                me->tickHandlers[TIMER_ID_DOUBLE_PRESS_RESET_TIMEOUT].timer = INVALID_VALUE;         
#ifdef CA17_LINEIN_TEST_MODE
                MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
                MainApp_SetAudioMode(me, me->audioMode+1);
#endif     
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
        case RESET_KEY:
        {
            AseNgNetworkInfo* pNetInfo= AseNgSrv_GetDecodedNetworkInfo();
//            if (pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_SCANNING ||
//             pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_CONNECTING ||
//             pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_AUTHENTICATING ||
//             pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_ACQUIRING ||
//             pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_CONNECTED )
//            {
//                MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
//                TP_PRINTF(" If current network is Ethernet, then should not trigger network setup. \r\n");
//            }
//            else if(me->combinedKey == CONFIG_IN_HOLD)
//            {
//                TP_PRINTF("Proto_FepAse_ReqResp_NETWORK_SETUP\r\n");
//                AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_NETWORK_SETUP);
//                MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
//            }
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
        case STANDBY_KEY:
        {
            TP_PRINTF("Proto_FepAse_ReqResp_OFF\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_OFF);
            break;
        }
        case CONNECT_KEY:
        {
            TP_PRINTF("Proto_FepAse_ReqResp_ALL_STANDBY\r\n");
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_ALL_STANDBY);
            MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
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
            if ( me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE )
            {
                if( me->musicPlaying )
                {
                    MainApp_VolumeDownOneStep(me);
                }
                else
                {
                    MainApp_VolumeHandlerOnPause(me);         
                }
            }
            else if (me->audioMode==AUDIO_MODE_EXT_SOURCE)
            {
                MainApp_VolumeDownOneStep(me);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {   
            if ( me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE )
            {
                if( me->musicPlaying )
                {
                    MainApp_VolumeUpOneStep(me);
                }
                else
                {
                    MainApp_VolumeHandlerOnPause(me);           
                }
            }
            else if (me->audioMode==AUDIO_MODE_EXT_SOURCE)
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
            break;
        }
        case RESET_KEY:
        {
            TP_PRINTF("FepAseCommand_Command_FACTORY_RESET\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_FACTORY_RESET);
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
    return ret;
}
static QState MainApp_KeyCombEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    switch(e->keyId)
    {
        case NEXT_KEY:
        {
            TP_PRINTF("Proto_FepAse_ReqResp_NEXT\r\n");
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_NEXT);
            MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
            TYM_SET_BIT(me->combinedKey, COM_KEY_NEXT);
            break;
        }
        case PREV_KEY:
        {
            TP_PRINTF("Proto_FepAse_ReqResp_PREV\r\n");
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_PREV);
            MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
            TYM_SET_BIT(me->combinedKey, COM_KEY_PREV);
            break;
        }
        case CONFIG_KEY:
        {
            TP_PRINTF("Proto_FepAse_ReqResp_NETWORK_SETUP\r\n");
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_NETWORK_SETUP);
            MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
            TYM_SET_BIT(me->combinedKey, COM_KEY_CONFIG);   
            break;
        }
#ifdef HAS_SHOP_MODE
        case SHOP_MODE_KEY:
        {
            TP_PRINTF("Transit to shop mode\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);

            /* wait 2s to LED flashing, then transit to shopd mode. */
            MainApp_SendLedReq(me, LED_IND_ID_SHOP_MODE_POWERING_UP_PROD);
            MainApp_SendLedReq(me, LED_IND_ID_SHOP_MODE_POWERING_UP_CONN);
            me->tickHandlers[TIMER_ID_DELAYED_ENTER_SHOP_MODE].timer = MAINAPP_DELAYED_ENTER_SHOP_MODE_TIMEOUT_IN_MS;
            
            /* Because we wait 2s for LED, ASE have enough time to play audio cue
             */
            AseNgSrv_PlayComfortTone("action_interaction_a.wav");
            me->combinedKey = COM_KEY_INVALID; 

            break;
        }
#endif        
        /*if necessary, add the handler for more keys here */
        default:break;
    }

    return ret;
}


QState MainApp_KeyHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    KeyStateEvt *evt = (KeyStateEvt*)e;
    
    //TP_PRINTF("key=%d, evt=%d, c=0x%02x\r\n", evt->keyId, evt->keyEvent, me->combinedKey );
    if( evt->keyEvent != KEY_EVT_UP && evt->keyEvent != KEY_EVT_DOWN )
    {
        /* When combine key press, before releaes all keys, ignore new key events
         */
        if( me->combinedKey & (CONNECT_COMB_DOWN|VOL_UP_COMB_DOWN|VOL_DOWN_COMB_DOWN|STANDBY_COMB_DOWN|RESET_COMB_DOWN) ) 
        {
            //TP_PRINTF("ignore 1 \r\n");
            return ret;
        }
        
        /* For combile key [CONNECT + RESET], standby hold event is earilier than Combile hold,
         * thus we need to ignore hold event.
         */
        if( (evt->keyId==STANDBY_KEY     &&  (me->combinedKey & (                CONNECT_IN_DOWN|RESET_IN_DOWN|VOL_UP_IN_DOWN|VOL_DOWN_IN_DOWN)) ) ||
            (evt->keyId==CONNECT_KEY     &&  (me->combinedKey & (STANDBY_IN_DOWN|                RESET_IN_DOWN|VOL_UP_IN_DOWN|VOL_DOWN_IN_DOWN)) ) ||
            (evt->keyId==RESET_KEY       &&  (me->combinedKey & (STANDBY_IN_DOWN|CONNECT_IN_DOWN|              VOL_UP_IN_DOWN|VOL_DOWN_IN_DOWN)) ) ||
            (evt->keyId==VOLUME_UP_KEY   &&  (me->combinedKey & (STANDBY_IN_DOWN|CONNECT_IN_DOWN|RESET_IN_DOWN|               VOL_DOWN_IN_DOWN)) ) ||
            (evt->keyId==VOLUME_DOWN_KEY &&  (me->combinedKey & (STANDBY_IN_DOWN|CONNECT_IN_DOWN|RESET_IN_DOWN|VOL_UP_IN_DOWN                 )) ) )
        {
            //TP_PRINTF("ignore 2 \r\n");
            return ret;
        }
    }
    
            
    /* CA17 EVT2 button have a problem. When press VOL+ very hard, it trigger VOL+ then (after 30ms) VOL- event.
     * To workaround it, we ignore VOL- if VOL+ come first. The opposite behavior (VOL- then VOL+) is the same.
     */
    if( evt->keyEvent != KEY_EVT_UP )
    {
        if( (evt->keyId==VOLUME_DOWN_KEY &&  ((me->combinedKey & (VOL_UP_IN_DOWN)) != 0 )) ||
             evt->keyId==VOLUME_UP_KEY &&  ((me->combinedKey & (VOL_DOWN_IN_DOWN)) != 0 )) 
        {
            //TP_PRINTF("ignore [vol-%s]\r\n", ((evt->keyId==VOLUME_DOWN_KEY)?"-":"+"));
            return ret;
        }
    }
    
    
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
            ret = MainApp_KeyCombEvtAction(me, evt);
            break;
        default:break;
    }
    return ret;

}

