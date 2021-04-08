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
#include "MainApp.h"
#include "DspDrv1451.h"


/*****************************************************************
 * Global Variable
 *****************************************************************/

/*****************************************************************
 * Definition
 *****************************************************************/
/* To avoid send too many log to ASE-TK, this file print to UART only
 */
//#undef TP_PRINTF
//#if !defined(NDEBUG)
//    #define TP_PRINTF  printf
//#else    
//    #define TP_PRINTF(...)
//#endif  // NDEBUG



/*****************************************************************
 * Function Implemenation
 *****************************************************************/
static QState MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    return ret;
}

static QState MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    return ret;
}

//Short press
static QState MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    bool keyHandled= TRUE;
    
    switch(e->keyId)
    {
#ifdef DEBUG_DEMO_FEATURE
        case TOUCH_NOISE_KEY:
        {
            MainApp_SendLedReq(me, LED_IND_ID_TOUCH_NOISE, /*force:*/FALSE);
            break;
        }
#endif
        
        case TOUCH_TAP_KEY:
        {
            MainApp_SendLedReq(me, LED_IND_ID_KEY_SP, /*force:*/FALSE);
            TP_PRINTF("FepAseCommand_Command_SOUND_SILENCE_TOGGLE\r\n");
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_SOUND_SILENCE_TOGGLE);
            break;
        }
        case TOUCH_SWIPE_LEFT_KEY:
        {
            TP_PRINTF("FepAseCommand_Command_NEXT\r\n");
#ifdef  DEBUG_DEMO_FEATURE 
            MainApp_SendLedReq(me, LED_IND_ID_TOUCH_KEY_FLASH_RED_TWICE, /*force:*/FALSE);
#else
            MainApp_SendLedReq(me, LED_IND_ID_KEY_SWIPE, /*force:*/FALSE);
#endif
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_NEXT);
            break;
        }
        case TOUCH_SWIPE_RIGHT_KEY:
        {
            TP_PRINTF("FepAseCommand_Command_PREV \r\n");
#ifdef  DEBUG_DEMO_FEATURE 
            MainApp_SendLedReq(me, LED_IND_ID_TOUCH_KEY_FLASH_WHT_TWICE, /*force:*/FALSE);
#else
            MainApp_SendLedReq(me, LED_IND_ID_KEY_SWIPE, /*force:*/FALSE);
#endif
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_PREV);
            break;
        }
        case FACTORY_RESET_KEY:
        {
#ifdef  DEBUG_DEMO_FEATURE 
            if (me->tickHandlers[TIMER_ID_DOUBLE_PRESS_FACTORY_RESET_TIMEOUT].timer <= 0)
            {
                me->tickHandlers[TIMER_ID_DOUBLE_PRESS_FACTORY_RESET_TIMEOUT].timer = MAINAPP_DOUBLE_PRESS_TIMEOUT_IN_MS;
            }
            else /* Doudble press */
            {
                me->tickHandlers[TIMER_ID_DOUBLE_PRESS_FACTORY_RESET_TIMEOUT].timer = INVALID_VALUE;
                MainApp_SendLedReq(me, LED_IND_ID_KEY_SP, /*force:*/FALSE);
                MainApp_SetAudioMode(me, (eAudioMode)(me->audioMode+1) );
            }
#endif
            break;
        }
        case SOFT_AP_KEY:
        {
#if defined(DEBUG_DEMO_FEATURE ) && !defined(BnO_fs1)
            static int32 pressTimes= 0;
            uint32 currTime= getSysTime();
            uint32 timeLastSoftApKey= me->timeLastSoftApKey;
            me->timeLastSoftApKey= currTime;
            if( (currTime - timeLastSoftApKey) < 300 )
            {
                pressTimes++;
                TP_PRINTF("SoftAP key press times = %d\r\n", pressTimes);
                if(pressTimes>=4) //press 5 times to trigger channel change
                {
                    pressTimes= 0;                    
                    uint32 dspCh= 0;
                    dspCh= *(uint32*)Setting_GetEx(SETID_DSP_CHANNEL, &dspCh);
                    dspCh= (dspCh+1>=DSP_CH_MAX)?(0):(dspCh+1); 
                    Setting_Set(SETID_DSP_CHANNEL, &dspCh);
                    AudioSrv_SetAudio(AUDIO_DSP_CHENNEL_L_R, TRUE, dspCh, /* NO USED*/0);

                    if(dspCh==DSP_CH_NORMAL)
                        MainApp_SendLedReq(me, LED_IND_ID_CHANNEL_NORMAL, /*force:*/FALSE);
                    else if(dspCh==DSP_CH_LEFT_ONLY)
                        MainApp_SendLedReq(me, LED_IND_ID_CHANNEL_LEFT, /*force:*/FALSE);
                    else //right
                        MainApp_SendLedReq(me, LED_IND_ID_CHANNEL_RIGHT, /*force:*/FALSE);                   

                }
            }
            else
            {
                pressTimes= 0; 
#endif         
                TP_PRINTF("FepAseCommand_Command_NETWORK_SETUP\r\n");
                AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_NETWORK_SETUP);
                MainApp_SendLedReq(me, LED_IND_ID_KEY_SP, /*force:*/FALSE);
#if defined(DEBUG_DEMO_FEATURE ) && !defined(BnO_fs1)
            }
#endif            
            break;
        }
        case POWER_KEY:
        {
            TP_PRINTF("FepAseCommand_Command_OFF\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_KEY_SP, /*force:*/FALSE);
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_OFF);
            break;
        }
        case VOLUME_UP_KEY:
        {
            MainApp_VolumeButtonHandler(me, VOLUME_STEP);
            break;
        }
        case VOLUME_DOWN_KEY:
        {
            MainApp_VolumeButtonHandler(me, -VOLUME_STEP);
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:          
          keyHandled= FALSE;
          break;
    }
    
    if(keyHandled)
    {
        MainApp_WakeupFromIdle(me);
    }
    
    return ret;
}


//Long press
static QState MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();    
    return ret;
}


//Very Long press, 8s
static QState MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    return ret;
}


//Double press
static QState MainApp_KeyDPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    switch(e->keyId)
    {
        case TOUCH_TAP_KEY:
        {
            TP_PRINTF("FepAseCommand_Command_NEXT_SOURCE\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_KEY_DP, /*force:*/FALSE);
            MainApp_WakeupFromIdle(me);
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_NEXT_SOURCE);
            break;
        }        
            
        /*if necessary, add the handler for more keys here */
        default:
        {
            ASSERT(0);
        }
    }
    
    return ret;
}

static QState MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    switch(e->keyId)
    {
        case TOUCH_TAP_KEY:
        {  
            TP_PRINTF("FepAseCommand_Command_ALL_STANDBY \r\n");
            MainApp_SendLedReq(me, LED_IND_ID_KEY_SP, /*force:*/FALSE);
            MainApp_WakeupFromIdle(me);
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_ALL_STANDBY);
        }
        break;
        case SOFT_AP_KEY:
        {
#ifdef  DEBUG_DEMO_FEATURE   
            MainApp_SendLedReq(me, LED_IND_ID_KEY_SP, /*force:*/FALSE);
            me->audioPassEnable= !me->audioPassEnable;
            MainApp_WakeupFromIdle(me);
            MainApp_DspBypassEnable(me, me->audioPassEnable);
#endif            
        }
        break;
        default:
          break;
    }
    
    return ret;
}

static QState MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();    
    return ret;
}

static QState MainApp_KeyVLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            if( me->powerEvt.dcInStatus  )
            {   //DC mode
                TP_PRINTF("Ignore Long press POWER KEY, becuase DC=1\r\n");
            }
            else
            {   //Battery mode
                TP_PRINTF("FepAseCommand_Command_STORAGE \r\n");
                MainApp_SendLedReq(me, LED_IND_ID_ENTER_STORAGE, /*force:*/TRUE);
                MainApp_WakeupFromIdle(me);
                AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_STORAGE);

                //ASE-TK v1.6.31 do not reply STORAGE mode, thus we add this checking
                 me->tickHandlers[TIMER_ID_ENTER_STORAGE_MODE_TIMEOUT].timer = MAINAPP_ENTER_STORAGE_MODE_TIMEOUT_MS;
            }
            break;
        }
        case FACTORY_RESET_KEY:
        {
            //On bring up stage, use factory reset key for volume control
            //thus disable factory reset function temporary
            TP_PRINTF("FepAseCommand_Command_FACTORY_RESET \r\n");

            //When send event to ASETK, it will reply very soon, thus we just flash RED when get event.
            //MainApp_SendLedReq(me, LED_IND_ID_FACTORY_RESET_TRIGGER, /*force:*/TRUE);
            MainApp_WakeupFromIdle(me);

            /* When reset command is done, ASE-TK will reboot automatically */
            AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_FACTORY_RESET);       
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
    
    return ret;
}


QState MainApp_KeyHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    KeyStateEvt *evt = (KeyStateEvt*)e;
    //TP_PRINTF("MainApp_KeyHandler: keyId=%d, keyEvent=%d\r\n", evt->keyId, evt->keyEvent);
    
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
        case KEY_EVT_DOUBLE_PRESS:
            ret= MainApp_KeyDPressEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_HOLD:
            ret= MainApp_KeyVLHoldEvtAction(me, evt);
            break;
        default:break;
    }

    //MainApp_CombineKeyHandler(me, e, me->combinedKey);
    return ret;
}

