/**
*  @file      MainApp_allplayEvtHandler.c
*  @brief     allplay event handler of mainApp
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
#include "AudioDrv.h"
#include "tym_qp_lib.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "pb_encode.h"


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
 * Function Prototype
 *****************************************************************/


/*****************************************************************
 * Function Implemenation
 *****************************************************************/
static void MainApp_AseTkVolChanged(cMainApp * const me, AseFepVolumeChangedEvent volEvt)
{
    if (volEvt.has_volume)
    {
        //TP_PRINTF("ASE-TK Set Volume: %d\r\n", volEvt.volume);
        MainApp_SetAbsoluteVolume(me, volEvt.volume);
    }
}


static void MainApp_AseTkSetLineInSensitivity(cMainApp * const me, AseFepSetLineInSensitivity sens)
{
#ifndef BnO_fs1
    if (sens.has_sensitivity)
    {
        TP_PRINTF("ASE-TK Set Linein Sensitivity: %d (not implement currently)\r\n", sens.sensitivity);        
        uint8 sensitivityLineIn = (uint8)sens.sensitivity;
        Setting_Set(SETID_SENSITIVITY_LINEIN, &sensitivityLineIn);
        AudioSrv_SetAudio(AUDIO_LINEIN_SENSITIVITY_SETT_ID, TRUE, (uint32)sens.sensitivity, /* NO USED*/0);
    }
#endif
}


static void MainApp_AseTkSetInput(cMainApp * const me, AseFepSetAudioInputReq volEvt)
{
    if (volEvt.has_input)
    {
        TP_PRINTF("ASE-TK Set Input: %d\r\n", volEvt.input);
        //if(AseFepSetAudioInputReq_AudioInput_LINE==volEvt.input)
        //{
        //    MainApp_SwitchAudioSource(me, AUDIO_SOURCE_EXT_SOURCE);
        //}
        //else //all other source set to ASE-TK, include ASE, TOS_LINK, POWER_LINK, HDMI, WIRELESS_MULTICHANNEL
        //{
        //    MainApp_SwitchAudioSource(me, AUDIO_SOURCE_ASETK);
        //}
    }

    if (volEvt.has_local)
    {   //Configure whether to use local playback or not when ever the input/output path allows it.
        TP_PRINTF("ASE-TK Set Local playback: %d\r\n", volEvt.local);
        if(volEvt.local)
        {
            MainApp_SwitchAudioSource(me, AUDIO_SOURCE_EXT_SOURCE);
        }
        else
        {
            MainApp_SwitchAudioSource(me, AUDIO_SOURCE_ASETK);
        }
    }
    
    AseTkSrv_ReplySetAudioInput(/*success:*/TRUE);
    
}


static QState MainApp_AseFepEventHandler(cMainApp * me, AseFepEvent aseFepEvent)
{
    QState ret= Q_UNHANDLED();
    switch (aseFepEvent.event)
    {
        case AseFepEvent_Event_SW_UPDATE_STARTED:
        {
            TP_PRINTF("AseFepEvent_Event_SW_UPDATE_STARTED\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_SW_UPDATING, /*force:*/FALSE);
            ret= Q_TRAN(&MainApp_WaitUpgrade);  //will mute amplifer on state
            break;
        }
        case AseFepEvent_Event_SW_UPDATE_FINISHED:
        {
            TP_PRINTF("AseFepEvent_Event_SW_UPDATE_FINISHED ==> *** unexpected UPDATE_FINISH ***\r\n");
            break;
        }
        case AseFepEvent_Event_SW_UPDATE_FAILED:
        {
            TP_PRINTF("AseFepEvent_Event_SW_UPDATE_FINISHED ==> *** unexpected UPDATE_FAILED ***\r\n");
            break;
        }
        case AseFepEvent_Event_BT_PAIRING_ENABLED:
        {
            TP_PRINTF("AseFepEvent_Event_BT_PAIRING_ENABLED\r\n");
            me->aseBtEnable= TRUE;
            MainApp_UpdateConnLed(me); 
            break;
        }
        case AseFepEvent_Event_BT_PAIRING_DISABLED:
        {
            TP_PRINTF("AseFepEvent_Event_BT_PAIRING_DISABLED\r\n");
            me->aseBtEnable= FALSE;
            MainApp_UpdateConnLed(me); 
            break;
        }
        case AseFepEvent_Event_BT_PAIRING_FAILED:
        {
            TP_PRINTF("AseFepEvent_Event_BT_PAIRING_FAILED\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_FAIL, /*force:*/FALSE);
            me->aseBtEnable= FALSE;
            MainApp_UpdateConnLed(me); 
            break;
        }
        case AseFepEvent_Event_BT_PAIRING_SUCCEEDED:
        {
            TP_PRINTF("AseFepEvent_Event_BT_PAIRING_SUCCEEDED\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_SUCCESS, /*force:*/FALSE);
            me->aseBtEnable= FALSE;
            MainApp_UpdateConnLed(me); 
            break;
        }
        case AseFepEvent_Event_BTLE_PAIRING_ENABLED:
        {
            TP_PRINTF("AseFepEvent_Event_BTLE_PAIRING_ENABLED\r\n");
            me->aseBtleEnable= TRUE;
            MainApp_UpdateConnLed(me); 
            break;
        }
        case AseFepEvent_Event_BTLE_PAIRING_DISABLED:
        {
            TP_PRINTF("AseFepEvent_Event_BTLE_PAIRING_DISABLED\r\n");
            me->aseBtleEnable= FALSE;
            MainApp_UpdateConnLed(me); 
            break;
        }
        case AseFepEvent_Event_BTLE_PAIRING_FAILED:
        {
            TP_PRINTF("AseFepEvent_Event_BTLE_PAIRING_FAILED\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_FAIL, /*force:*/FALSE);
            me->aseBtleEnable= FALSE;
            MainApp_UpdateConnLed(me); 
            break;
        }
        case AseFepEvent_Event_BTLE_PAIRING_SUCCEEDED:
        {
            TP_PRINTF("AseFepEvent_Event_BTLE_PAIRING_SUCCEEDED\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_SUCCESS, /*force:*/FALSE);
            me->aseBtleEnable= FALSE;
            MainApp_UpdateConnLed(me); 
            break;
        }
        case AseFepEvent_Event_PLAYER_PLAYING:
        {
            TP_PRINTF("AseFepEvent_Event_PLAYER_PLAYING\r\n");
            me->musicPlaying= TRUE;
            MainApp_MuteUpdate(me);
            MainApp_SendLedReq(me, LED_IND_ID_PLAYBACK_START, /*force:*/FALSE);
            break;
        }
        case AseFepEvent_Event_PLAYER_PAUSED:
        {
            TP_PRINTF("AseFepEvent_Event_PLAYER_PAUSED\r\n");
            me->musicPlaying= FALSE;
            MainApp_MuteUpdate(me);
            MainApp_SendLedReq(me, LED_IND_ID_PLAYBACK_STOP, /*force:*/FALSE);
            break;
        }
        case AseFepEvent_Event_PLAYER_STOPPED:
        {
            TP_PRINTF("AseFepEvent_Event_PLAYER_STOPPED\r\n");
            me->musicPlaying= FALSE;
            MainApp_MuteUpdate(me);
            MainApp_SendLedReq(me, LED_IND_ID_PLAYBACK_STOP, /*force:*/FALSE);
            break;
        }
        case AseFepEvent_Event_PLAYER_MUTED:
        { 
            TP_PRINTF("AseFepEvent_Event_PLAYER_MUTED\r\n");
            me->asetkMute= TRUE;
            MainApp_MuteUpdate(me);
            break;
        }
        case AseFepEvent_Event_PLAYER_UNMUTED:
        {
            TP_PRINTF("AseFepEvent_Event_PLAYER_UNMUTED\r\n");
            me->asetkMute= FALSE;
            MainApp_MuteUpdate(me);
            break;
        }
        case AseFepEvent_Event_SOFTAP_STARTED:
        {
            TP_PRINTF("AseFepEvent_Event_SOFTAP_STARTED\r\n");
            //MainApp_SendLedReq(me, LED_IND_ID_SOFTAP_MODE);
            break;
        }
        case AseFepEvent_Event_SOFTAP_STOPPED:
        {
            TP_PRINTF("AseFepEvent_Event_SOFTAP_STOPPED\r\n");
            //TODO: any event to indicate next mode?
            break;
        }
        case AseFepEvent_Event_APPLE_WAC_STARTED:
        {
            TP_PRINTF("AseFepEvent_Event_APPLE_WAC_STARTED\r\n");
            //MainApp_SendLedReq(me, LED_IND_ID_SOFTAP_MODE); 
            break;
        }
        case AseFepEvent_Event_APPLE_WAC_STOPPED:
        {
            TP_PRINTF("AseFepEvent_Event_APPLE_WAC_STOPPED\r\n");
            break;
        }
        case AseFepEvent_Event_APPLE_WAC_TIMEOUT:
        {
            TP_PRINTF("AseFepEvent_Event_APPLE_WAC_TIMEOUT\r\n");
            break;
        }
        case AseFepEvent_Event_ASE_BOOTED:
        {
            TP_PRINTF("AseFepEvent_Event_ASE_BOOTED\r\n");
            break;
        }
        case AseFepEvent_Event_ASE_OVERHEAT_ALERT:
        {
            TP_PRINTF("*** AseFepEvent_Event_ASE_OVERHEAT_ALERT ***\r\n");
            /* TODO: do anything when ASE-TK is over heat?   
             * If ASE-TK want to shutdown, it should send OFF event
             */
            break;
        }
        case AseFepEvent_Event_SW_UPDATE_FEP_BOOTLOADER:
        {
            TP_PRINTF("AseFepEvent_Event_SW_UPDATE_FEP_BOOTLOADER\r\n");
            break;
        }
        case AseFepEvent_Event_SW_UPDATE_FEP_APPLICATION:
        {
            TP_PRINTF("AseFepEvent_Event_SW_UPDATE_FEP_APPLICATION\r\n");
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_STORAGE:
        {
            TP_PRINTF("AseFepEvent_Event_SYSTEM_STATUS_STORAGE\r\n");
#ifdef BnO_fs1
            ret= Q_TRAN(&MainApp_Storage);
#endif
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_STANDBY:
        {
            TP_PRINTF("AseFepEvent_Event_SYSTEM_STATUS_STANDBY\r\n");
            me->aseNetMode= aseFepEvent.event;
            MainApp_UpdateConnLed(me);

            /* ASE-TK v1.6.46 sometimes enter network-standby but do not STOP.
             * It cause FEP do not mute amplififer.
             * To workaround, we set STOP state when enter network-standby.
             */
            me->musicPlaying= FALSE;
            MainApp_MuteUpdate(me);
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_ON:
        {
            TP_PRINTF("AseFepEvent_Event_SYSTEM_STATUS_ON\r\n");
            me->aseNetMode= aseFepEvent.event;
            MainApp_UpdateConnLed(me);
            MainApp_MuteUpdate(me);
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_OFF:
        {
            TP_PRINTF("\r\n\r\n**** AseFepEvent_Event_SYSTEM_STATUS_OFF ==> power off ****\r\n\r\n");
            if( !(me->powerEvt.dcInStatus) && me->powerEvt.batteryInfo.battStatus==BatteryStatus_LEVEL_CRITICAL )
            {
                me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_ERROR;
                me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_ERROR;
            }
            ret= Q_TRAN(&MainApp_PoweringDown);
            break;
        }
        case AseFepEvent_Event_FACTORY_RESET_START:
        {
            TP_PRINTF("AseFepEvent_Event_FACTORY_RESET_START\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_FACTORY_RESET_TRIGGER, /*force:*/TRUE);
            MainApp_SendLedReq(me, LED_IND_ID_ALL_BG_OFF, /*force:*/TRUE);            
            ret= Q_TRAN(&MainApp_WaitFactoryReset); //will mute amplifer on state
            break;
        }
        case AseFepEvent_Event_FACTORY_RESET_DONE:
        {
            TP_PRINTF("AseFepEvent_Event_FACTORY_RESET_DONE ==> *** unexpected FACTORY_RESET_DONE ***\r\n");  
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_ASE_RESTART:
        {
            TP_PRINTF("AseFepEvent_Event_SYSTEM_STATUS_ASE_RESTART\r\n"); 
            MainApp_SendLedReq(me, LED_IND_ID_FACTORY_RESET_TRIGGER, /*force:*/TRUE);
            MainApp_SendLedReq(me, LED_IND_ID_ALL_BG_OFF, /*force:*/TRUE);            
            ret= Q_TRAN(&MainApp_WaitFactoryReset); //will mute amplifer on state
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_RESTART:
        {
            TP_PRINTF("AseFepEvent_Event_SYSTEM_STATUS_RESTART\r\n");
            BSP_SoftReboot();
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_ON_NO_OPERATION:
        {
            TP_PRINTF("\r\n\r\n\r\n\r\n *** AseFepEvent_Event_SYSTEM_STATUS_ON_NO_OPERATION ***\r\n\r\n\r\n\r\n");
            me->aseNetMode= aseFepEvent.event;
            MainApp_UpdateConnLed(me);
            MainApp_MuteUpdate(me);
            break;
        }
        case AseFepEvent_Event_COMFORT_TONE_START:
        {
            TP_PRINTF("COMFORT_TONE_START\r\n");
            me->tonePlaying= TRUE;
            MainApp_MuteUpdate(me);
            
            /* ASETK-227: SUE expect FEP to change source to ASE-TK when TONE_START.
             *            ASE-TK may change source again after TONE_DONE.
             */
            MainApp_SwitchAudioSource(me, AUDIO_SOURCE_ASETK);
            break;
        }
        case AseFepEvent_Event_COMFORT_TONE_DONE:
        {
            TP_PRINTF("COMFORT_TONE_DONE\r\n");
            me->tonePlaying= FALSE;
            MainApp_MuteUpdate(me);
            break;
        }
        default:
        {
            TP_PRINTF("*** AseFepEvent_Event UNKNOW ***\r\n");
            break;
        }
    }
    
    return ret;
}


QState MainApp_AseTkEvtHandler(cMainApp * const me, QEvt const * const e)
{
    AseTkStateIndEvt* aseTkEvt = (AseTkStateIndEvt*) e;
    QState ret= Q_UNHANDLED();

    if(aseTkEvt->bIsComWdgTimeOut)
    {
        ASSERT(0);
        MainApp_DelayedErrorReboot(me);
        return ret;
    }


    switch(aseTkEvt->aseFepCmd.which_OneOf)
    {
        case AseFepMessage_aseFepEvent_tag:
        {
            ret= MainApp_AseFepEventHandler(me, aseTkEvt->aseFepCmd.OneOf.aseFepEvent);
            break;
        }
        case AseFepMessage_aseFepVolumeChangedEvent_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepVolumeChangedEvent_tag\r\n");
            MainApp_AseTkVolChanged(me, aseTkEvt->aseFepCmd.OneOf.aseFepVolumeChangedEvent); 
            break;
        }
        case AseFepMessage_aseFepVolumeFadeEvent_tag:
        {
            AseFepVolumeFadeEvent volFadeEvent = aseTkEvt->aseFepCmd.OneOf.aseFepVolumeFadeEvent;
#ifdef ASE_TK_DEBUG_SHOW_RX_DATA
            TP_PRINTF("    Volume fade: start_volume:%d target_volume:%d fade_duration:%d ms \r\n",
                      volFadeEvent.start_volume, volFadeEvent.target_volume, volFadeEvent.fade_duration);
#endif
            //do not change volume, and do not reply on special mode
            if(me->audioMode==AUDIO_MODE_NORMAL)
            {
                AudioSrv_SetAudio(AUDIO_VOL_FADE_SETT_ID, /*enable*/TRUE, volFadeEvent.target_volume, volFadeEvent.fade_duration);
            }
            break;
        }
        case AseFepMessage_aseFepSetLineInSensitivity_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepSetLineInSensitivity_tag\r\n");
            MainApp_AseTkSetLineInSensitivity(me, aseTkEvt->aseFepCmd.OneOf.aseFepSetLineInSensitivity); 
            break;            
        }
        case AseFepMessage_aseFepTunnel_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepTunnel_tag\r\n");
            //Handle by AsetkSrv
            break;
        }
        case AseFepMessage_aseFepSetAudioInputReq_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepSetAudioInputReq_tag\r\n");
            MainApp_AseTkSetInput(me, aseTkEvt->aseFepCmd.OneOf.aseFepSetAudioInputReq); 
            break;
        }
        case AseFepMessage_aseFepSetAudioOutputReq_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepSetAudioOutputReq_tag (not support)\r\n");
            //not support
            break;
        }
        case AseFepMessage_aseFepAliveResp_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepAliveResp_tag\r\n");
            //Handle by AsetkSrv
            break;
        }
        case AseFepMessage_aseFepSetPositionSoundModeReq_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepSetPositionSoundModeReq_tag\r\n");
            //not support
            break;
        }
        case AseFepMessage_aseFepGetPositionSoundModeReq_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepGetPositionSoundModeReq_tag (not support)\r\n");
            //not support
            break;
        }
        case AseFepMessage_aseFepNetworkInfo_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepNetworkInfo_tag\r\n");
            MainApp_UpdateConnLed(me);
            break;
        }
        case AseFepMessage_aseFepReq_tag:
        {
            //Handle by AsetkSrv
            TP_PRINTF("AseFepMessage_aseFepReq_tag: request=%d\r\n", aseTkEvt->aseFepCmd.OneOf.aseFepReq.request);
            break;
        }
        case AseFepMessage_aseFepSetAudioPcmFormatCommand_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepSetAudioPcmFormatCommand_tag\r\n");
            //Our DSP use ASRC to change sample rate automatically, do nothing here
            AseTkSrv_ReplyAudioPcmFormatCommand(TRUE);
            break;
        }
        case AseFepMessage_aseFepSetInternalSpeakerCompensationCommand_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepSetInternalSpeakerCompensationCommand_tag\r\n");
            break;
        }
        default:
            break;
    }
    return ret;
}




bool AseTkSrv_encodeInternalSpeakerCompensationDetails(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    //InternalSpeaker *pIntSpk= *(InternalSpeaker**)arg;
    //TP_PRINTF("AseTkSrv_encodeInternalSpeakerCompensationDetails: pos=%d, type=%d, gain=%f\r\n", pIntSpk->position, pIntSpk->type, pIntSpk->compensation);
    
#ifdef BnO_fs1
    //FS1
    InternalSpeaker spkWf, spkTw;

    //woofer
    if( Setting_IsReady(SETID_DSP_CAL_GAIN1_WF) )
    {
        spkWf.has_compensation= 1;
        spkWf.compensation= *(double*)Setting_Get(SETID_DSP_CAL_GAIN1_WF);
    }
    else
    {
        spkWf.has_compensation= 0;
        spkWf.compensation= 0.0;
    }
    spkWf.position=InternalSpeaker_Position_CENTRE;
    spkWf.type=InternalSpeaker_Type_WOOFER;
    

    //tweeter
    if( Setting_IsReady(SETID_DSP_CAL_GAIN2_TW) )
    {
        spkTw.has_compensation= 1;
        spkTw.compensation= *(double*)Setting_Get(SETID_DSP_CAL_GAIN2_TW);
    }
    else
    {
        spkTw.has_compensation= 0;
        spkTw.compensation= 0.0;
    }
    spkTw.position=InternalSpeaker_Position_CENTRE;
    spkTw.type=InternalSpeaker_Type_FULLRANGE;
    

    return pb_encode_tag_for_field(stream, InternalSpeaker_fields) &&
           pb_encode_submessage(stream, InternalSpeaker_fields, &spkTw) &&
           pb_encode_tag_for_field(stream, InternalSpeaker_fields) &&
           pb_encode_submessage(stream, InternalSpeaker_fields, &spkWf);
    
#else

    //FS2
    InternalSpeaker spkWf, spkMidA, spkMidB, spkTw;

    //woofer
    if( Setting_IsReady(SETID_DSP_CAL_GAIN1_WF) )
    {
        spkWf.has_compensation= 1;
        spkWf.compensation= *(double*)Setting_Get(SETID_DSP_CAL_GAIN1_WF);
    }
    else
    {
        spkWf.has_compensation= 0;
        spkWf.compensation= 0.0;
    }
    spkWf.position=InternalSpeaker_Position_CENTRE;
    spkWf.type=InternalSpeaker_Type_WOOFER;


    //Middle-A
    if( Setting_IsReady(SETID_DSP_CAL_GAIN2_MID_A) )
    {
        spkMidA.has_compensation= 1;
        spkMidA.compensation= *(double*)Setting_Get(SETID_DSP_CAL_GAIN2_MID_A);
    }
    else
    {
        spkMidA.has_compensation= 0;
        spkMidA.compensation= 0.0;
    }
    spkMidA.position=InternalSpeaker_Position_LEFT;
    spkMidA.type=InternalSpeaker_Type_MIDRANGE;


    //Middle-B
    if( Setting_IsReady(SETID_DSP_CAL_GAIN3_MID_B) )
    {
        spkMidB.has_compensation= 1;
        spkMidB.compensation= *(double*)Setting_Get(SETID_DSP_CAL_GAIN3_MID_B);
    }
    else
    {
        spkMidB.has_compensation= 0;
        spkMidB.compensation= 0.0;
    }
    spkMidB.position=InternalSpeaker_Position_RIGHT;
    spkMidB.type=InternalSpeaker_Type_MIDRANGE;


    //tweeter
    if( Setting_IsReady(SETID_DSP_CAL_GAIN4_TW) )
    {
        spkTw.has_compensation= 1;
        spkTw.compensation= *(double*)Setting_Get(SETID_DSP_CAL_GAIN4_TW);
    }
    else
    {
        spkTw.has_compensation= 0;
        spkTw.compensation= 0.0;
    }
    spkTw.position=InternalSpeaker_Position_CENTRE;
    spkTw.type=InternalSpeaker_Type_TWEETER;


    return pb_encode_tag_for_field(stream, InternalSpeaker_fields) &&
           pb_encode_submessage(stream, InternalSpeaker_fields, &spkWf) &&
           pb_encode_tag_for_field(stream, InternalSpeaker_fields) &&
           pb_encode_submessage(stream, InternalSpeaker_fields, &spkMidA) &&
           pb_encode_tag_for_field(stream, InternalSpeaker_fields) &&
           pb_encode_submessage(stream, InternalSpeaker_fields, &spkMidB) &&
           pb_encode_tag_for_field(stream, InternalSpeaker_fields) &&
           pb_encode_submessage(stream, InternalSpeaker_fields, &spkTw);

    
#endif
}





/* Callback function of ASE-TK Server
 */
FepAseInternalSpeakerCompensationResp_Error AseTkSrv_Callback_SetInternalSpeakerCompensation(InternalSpeaker_Position position, InternalSpeaker_Type type, double gain)
{
    TP_PRINTF("AseTkSrv_Callback_SetInternalSpeakerCompensation: pos=%d, type=%d, gain=%f\r\n", position, type, gain);

#ifdef BnO_fs1

    if( InternalSpeaker_Position_CENTRE==position && type==InternalSpeaker_Type_WOOFER )
    {
        Setting_Set(SETID_DSP_CAL_GAIN1_WF, &gain);
        AudioSrv_SetDspCalGain(DSP_CAL_GAIN1_ID, gain); //woofer
    }
    else if( InternalSpeaker_Position_CENTRE==position && type==InternalSpeaker_Type_FULLRANGE )
    {
        Setting_Set(SETID_DSP_CAL_GAIN2_TW, &gain);
        AudioSrv_SetDspCalGain(DSP_CAL_GAIN2_ID, gain); //tweeter
    }
    else
    {
        TP_PRINTF("AseTkSrv_Callback_SetInternalSpeakerCompensation: unknown position:%d / type%d \r\n", position, type);
        return FepAseInternalSpeakerCompensationResp_Error_POSITION_TYPE_COMBINATION_ERROR;
    }

#else //fs2

    if( InternalSpeaker_Position_CENTRE==position && type==InternalSpeaker_Type_WOOFER )
    {
        Setting_Set(SETID_DSP_CAL_GAIN1_WF, &gain);
        AudioSrv_SetDspCalGain(DSP_CAL_GAIN1_ID, gain); //woofer
    }
    else if( InternalSpeaker_Position_LEFT==position && type==InternalSpeaker_Type_MIDRANGE )
    {
        Setting_Set(SETID_DSP_CAL_GAIN2_MID_A, &gain);
        AudioSrv_SetDspCalGain(DSP_CAL_GAIN2_ID, gain); //middle A => left
    }
    else if( InternalSpeaker_Position_RIGHT==position && type==InternalSpeaker_Type_MIDRANGE )
    {
        Setting_Set(SETID_DSP_CAL_GAIN3_MID_B, &gain);
        AudioSrv_SetDspCalGain(DSP_CAL_GAIN3_ID, gain); //middle B => right
    }
    else if( InternalSpeaker_Position_CENTRE==position && type==InternalSpeaker_Type_TWEETER )
    {
        Setting_Set(SETID_DSP_CAL_GAIN4_TW, &gain);
        AudioSrv_SetDspCalGain(DSP_CAL_GAIN4_ID, gain); //tweeter
    }
    else
    {
        TP_PRINTF("AseTkSrv_Callback_SetInternalSpeakerCompensation: unknown position:%d / type%d \r\n", position, type);
        return FepAseInternalSpeakerCompensationResp_Error_POSITION_TYPE_COMBINATION_ERROR;
    }
    
#endif  

    return FepAseInternalSpeakerCompensationResp_Error_NO_ERROR;

}

