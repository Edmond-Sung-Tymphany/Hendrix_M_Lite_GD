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

/*****************************************************************
 * Global Variable
 *****************************************************************/


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
        TP_PRINTF("ASE-TK Set Volume: %d\r\n", volEvt.volume);
        ASSERT(volEvt.volume>=MIN_VOLUME && volEvt.volume<=MAX_VOLUME);
        AudioSrv_SetVolume(volEvt.volume);
        if(me->isVolChanged == FALSE)
        {
            me->absoluteVol = volEvt.volume;
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
        }
    }
}

void MainApp_UpdateConnLed(cMainApp * const me)
{
    eLedIndID newConnLedIndId = LED_IND_ID_MAX;

    newConnLedIndId = MainApp_GetUpdatedConnLed(me);

    if(newConnLedIndId < LED_IND_ID_MAX)
    {
        MainApp_SendLedReq(me, newConnLedIndId);
    }
}

eLedIndID MainApp_GetUpdatedConnLed(cMainApp * const me)
{
    eLedIndID newConnLedIndId = LED_IND_ID_MAX;

    AsetkNetworkInfo* pNetInfo = AseTkSrv_GetDecodedNetworkInfo();

    //Update Connective LED
    if (me->aseBtleEnable)
    {
        newConnLedIndId = LED_IND_ID_BLE_PAIRING_ENABLED;
    }
    else if (pNetInfo->ethernet_state == NetworkInfo_State_SCANNING ||
             pNetInfo->ethernet_state == NetworkInfo_State_CONNECTING ||
             pNetInfo->ethernet_state == NetworkInfo_State_AUTHENTICATING ||
             pNetInfo->ethernet_state == NetworkInfo_State_ACQUIRING ||
             pNetInfo->ethernet_state == NetworkInfo_State_CONNECTED )
    {
        /* When insert Ethernet calbe on SoftAp mode, the temperary state is
         *    Wifi[6], Ethernet[2], SoftAp[6], Wifi:Configured[0],Quality[0]  ==> SoftAP
         *
         * To avoid show blue LED, we treat connecting/scanning/... state as connected
         */
        if (me->systemStatus == SYSTEM_STA_ON)
            newConnLedIndId = LED_IND_ID_ETH_CONNECTED;
        else
            newConnLedIndId = LED_IND_ID_ETH_CONNECTED_IDLE;
    }
    else if (pNetInfo->soft_ap_state == NetworkInfo_State_SCANNING ||
             pNetInfo->soft_ap_state == NetworkInfo_State_CONNECTING ||
             pNetInfo->soft_ap_state == NetworkInfo_State_AUTHENTICATING ||
             pNetInfo->soft_ap_state == NetworkInfo_State_ACQUIRING ||
             pNetInfo->soft_ap_state == NetworkInfo_State_CONNECTED)
    {
        newConnLedIndId = LED_IND_ID_WIFI_SETUP_IN_PROGRESS;
    }
    else if (pNetInfo->wifi_state == NetworkInfo_State_SCANNING ||
             pNetInfo->wifi_state == NetworkInfo_State_CONNECTING ||
             pNetInfo->wifi_state == NetworkInfo_State_AUTHENTICATING ||
             pNetInfo->wifi_state == NetworkInfo_State_ACQUIRING )
    {
        newConnLedIndId = LED_IND_ID_WIFI_SETUP_IN_PROGRESS;
    }
    else if (pNetInfo->wifi_state == NetworkInfo_State_CONNECTED)
    {
        if (pNetInfo->wifi_quality == WiFi_Quality_EXELENT)
        {
            if (me->systemStatus == SYSTEM_STA_ON)
                newConnLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_STRONG;
            else
                newConnLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_STRONG_IDLE;
        }
        else if (pNetInfo->wifi_quality == WiFi_Quality_GOOD)
        {
            if (me->systemStatus == SYSTEM_STA_ON)
                newConnLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_NORMAL;
            else
                newConnLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_NORMAL_IDLE;
        }
        else /*if( pNetInfo->wifi_quality==WiFi_Quality_POOR ) */
        {
            if (me->systemStatus == SYSTEM_STA_ON)
                newConnLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_WEAK;
            else
                newConnLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_WEAK_IDLE;
        }
    }
    else if (pNetInfo->wifi_state == NetworkInfo_State_FAILED ||
            pNetInfo->ethernet_state == NetworkInfo_State_FAILED)
    {
        newConnLedIndId = LED_IND_ID_WIFI_ERROR;
    }
    else if (pNetInfo->wifi_state == NetworkInfo_State_UNKNOWN &&
            pNetInfo->ethernet_state == NetworkInfo_State_UNKNOWN &&
            pNetInfo->soft_ap_state == NetworkInfo_State_UNKNOWN )
    {
        /* When boot up, ASE-TK send this temperary state, do not change LED
         */
    }
    else /* if( pNetInfo->wifi_state==NetworkInfo_State_DISCONNECTED) */
    {
        //ignore pNetInfo->wifi_configure
        if (me->systemStatus == SYSTEM_STA_ON)
            newConnLedIndId = LED_IND_ID_WIFI_UNCONFIGURED;
        else
            newConnLedIndId = LED_IND_ID_WIFI_UNCONFIGURED;
    }

    return newConnLedIndId;
}


static void MainApp_AseTkNetworkInfo(cMainApp * const me, AseFepNetworkInfo netInfo)
{
    MainApp_UpdateConnLed(me);
}

static void MainApp_AseTkSetLineInSensitivity(cMainApp * const me, AseFepSetLineInSensitivity sens)
{
    if (sens.has_sensitivity)
    {
        TP_PRINTF("ASE-TK Set Linein Sensitivity: %d \r\n", sens.sensitivity);
        AudioSrv_SetAudio(AUDIO_LINEIN_SENSITIVITY_SETT_ID, TRUE, (uint32)sens.sensitivity, /*No used*/0);
    }
}

static void MainApp_AseTkSetPositionSoundMode(cMainApp * const me, AseFepSetPositionSoundModeReq soundMode)
{
    Setting_Set(SETID_POSITION_SOUND_MODE, &soundMode);
    if (soundMode.has_mode)
    {
        TP_PRINTF("ASE-TK Set Position Sound Mode has_position:%d position:%d \r\n", soundMode.mode.has_position, soundMode.mode.position);
        TP_PRINTF("ASE-TK Set Position Sound Mode has_orientation:%d orientation:%d \r\n", soundMode.mode.has_orientation, soundMode.mode.orientation);
        AudioSrv_SetAudio(AUDIO_POS_SOUND_MODE_SETT_ID, TRUE, NULL, /*No used*/0);
    }
}

static void MainApp_AseTkGetPositionSoundMode(cMainApp * const me)
{
    AseFepSetPositionSoundModeReq soundMode = *(AseFepSetPositionSoundModeReq *)Setting_Get(SETID_POSITION_SOUND_MODE);
    FepAsePositionSoundModeResp soundModeResp = {0};
    soundModeResp.has_error = FALSE; // no error
    soundModeResp.error = FepAsePositionSoundModeResp_Error_NO_ERROR;
    soundModeResp.has_mode = soundMode.has_mode;
    soundModeResp.mode = soundMode.mode;
    AseTkSrv_ReplySetPositionSoundMode(soundModeResp);
}

static void MainApp_AseTkSetInput(cMainApp * const me, AseFepSetAudioInputReq volEvt)
{
    eAudioSource audioSrc = AUDIO_SOURCE_WIFI;
    if (volEvt.has_input)
    {
        TP_PRINTF("ASE-TK Set Input: %d\r\n", volEvt.input);
    }

    AseFepSetAudioInputReq_AudioInput aseAudioInput = volEvt.input;
    Setting_Set(SETID_ASE_AUDIO_INPUT, &aseAudioInput);
    if (volEvt.has_local && volEvt.local)
    {   //Configure whether to use local playback or not when ever the input/output path allows it.
        TP_PRINTF("ASE-TK Set Local playback: %d\r\n", volEvt.local);
        if (AseFepSetAudioInputReq_AudioInput_LINE == volEvt.input)
        {
            audioSrc = AUDIO_SOURCE_AUXIN;
        }
        else if (AseFepSetAudioInputReq_AudioInput_TOS_LINK == volEvt.input)
        {
            audioSrc = AUDIO_SOURCE_SPDIF_IN;
        }
    }
    else
    {
        audioSrc = AUDIO_SOURCE_WIFI;
    }
#ifndef SOURCE_HIJACK_IN_MCU
    MainApp_SwitchAudioSource(me, audioSrc);
#endif
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
            MainApp_SendLedReq(me, LED_IND_ID_SW_UPDATING);
            ret= Q_TRAN(&MainApp_WaitUpgrade);  //will mute amplifer on state
            break;
        }
        case AseFepEvent_Event_SW_UPDATE_FINISHED:
        {
            //TODO: back to normal LED?
            MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
            MainApp_UpdateConnLed(me);
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, FALSE);
            TP_PRINTF("AseFepEvent_Event_SW_UPDATE_FINISHED ==> *** unexpected UPDATE_FINISH ***\r\n");
            break;
        }
        case AseFepEvent_Event_SW_UPDATE_FAILED:
        {
            //TODO: back to normal LED?
            MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
            MainApp_UpdateConnLed(me);
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, FALSE);
            TP_PRINTF("AseFepEvent_Event_SW_UPDATE_FINISHED ==> *** unexpected UPDATE_FAILED ***\r\n");
            break;
        }
        case AseFepEvent_Event_BT_PAIRING_ENABLED:
        {
            TP_PRINTF("AseFepEvent_Event_BT_PAIRING_ENABLED\r\n");
            /* Same UI as */
            me->aseBtleEnable = TRUE;
            MainApp_UpdateConnLed(me);
            break;
        }
        case AseFepEvent_Event_BT_PAIRING_DISABLED:
        {
            TP_PRINTF("AseFepEvent_Event_BT_PAIRING_DISABLED\r\n");
            me->aseBtleEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;
        }
        case AseFepEvent_Event_BT_PAIRING_FAILED:
        {
            TP_PRINTF("AseFepEvent_Event_BT_PAIRING_FAILED\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_FAIL);
            me->aseBtleEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;
        }
        case AseFepEvent_Event_BT_PAIRING_SUCCEEDED:
        {
            TP_PRINTF("AseFepEvent_Event_BT_PAIRING_SUCCEEDED\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_SUCCESSFULL);
            me->aseBtleEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;
        }
        case AseFepEvent_Event_BTLE_PAIRING_ENABLED:
        {
            TP_PRINTF("AseFepEvent_Event_BTLE_PAIRING_ENABLED\r\n");
            me->aseBtleEnable = TRUE;
            MainApp_UpdateConnLed(me);
            break;
        }
        case AseFepEvent_Event_BTLE_PAIRING_DISABLED:
        {
            TP_PRINTF("AseFepEvent_Event_BTLE_PAIRING_DISABLED\r\n");
            me->aseBtleEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;
        }
        case AseFepEvent_Event_BTLE_PAIRING_FAILED:
        {
            TP_PRINTF("AseFepEvent_Event_BTLE_PAIRING_FAILED\r\n");
            me->aseBtleEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;
        }
        case AseFepEvent_Event_BTLE_PAIRING_SUCCEEDED:
        {
            TP_PRINTF("AseFepEvent_Event_BTLE_PAIRING_SUCCEEDED\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_SUCCESSFULL);
            me->aseBtleEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;
        }
        case AseFepEvent_Event_PLAYER_PLAYING:
        {
            TP_PRINTF("AseFepEvent_Event_PLAYER_PLAYING\r\n");
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, FALSE);
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
            break;
        }
        case AseFepEvent_Event_PLAYER_PAUSED:
        {
            TP_PRINTF("AseFepEvent_Event_PLAYER_PAUSED\r\n");
            //AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
            break;
        }
        case AseFepEvent_Event_PLAYER_STOPPED:
        {
            TP_PRINTF("AseFepEvent_Event_PLAYER_STOPPED\r\n");
            /* fix issue: [LED indication] Product LED will quick flash twice more [IN:011958]
            * https://pm.tymphany.com/SpiraTeam/243/Incident/11958.aspx
            */
            if (SYSTEM_STA_STANDBY_HIGH != me->systemStatus)
            {
                MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
            }
            break;
        }
        case AseFepEvent_Event_PLAYER_MUTED:
        { 
            TP_PRINTF("AseFepEvent_Event_PLAYER_MUTED\r\n");
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
            //MainApp_SendLedReq(me, LED_IND_ID_PLAYBACK_STOP);
            break;
        }
        case AseFepEvent_Event_PLAYER_UNMUTED:
        {
            TP_PRINTF("AseFepEvent_Event_PLAYER_UNMUTED\r\n");
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, FALSE);
            //MainApp_SendLedReq(me, LED_IND_ID_PLAYBACK_START);
            break;
        }
        case AseFepEvent_Event_SOFTAP_STARTED:
        {
            TP_PRINTF("AseFepEvent_Event_SOFTAP_STARTED\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_WIFI_SETUP_IN_PROGRESS);
            break;
        }
        case AseFepEvent_Event_SOFTAP_STOPPED:
        {
            //Fix-Me: soft AP stop event will be send in wifi connected or ethernet connected?
            TP_PRINTF("AseFepEvent_Event_SOFTAP_STOPPED\r\n");
            AsetkNetworkInfo* pNetInfo= AseTkSrv_GetDecodedNetworkInfo();
            if ((!pNetInfo->wifi_configured) && (pNetInfo->ethernet_state == NetworkInfo_State_DISCONNECTED)
                && (pNetInfo->wifi_state == NetworkInfo_State_DISCONNECTED))
            {
                MainApp_SendLedReq(me, LED_IND_ID_WIFI_UNCONFIGURED);
            }
            break;
        }
        case AseFepEvent_Event_APPLE_WAC_STARTED:
        {
            TP_PRINTF("AseFepEvent_Event_APPLE_WAC_STARTED\r\n");
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
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_STANDBY:
        {
            TP_PRINTF("AseFepEvent_Event_SYSTEM_STATUS_STANDBY\r\n");

            eLedIndID currConnLedState = MainApp_GetUpdatedConnLed(me);

            switch(currConnLedState)
            {
                case LED_IND_ID_WIFI_SIG_STRENGTH_STRONG:
                {
                    MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_DIM_WHITE);
                    break;
                }
                case LED_IND_ID_WIFI_SIG_STRENGTH_NORMAL:
                {
                    MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_DIM_AMBER);
                    break;
                }
                case LED_IND_ID_WIFI_SIG_STRENGTH_WEAK:
                {
                    MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_DIM_RED);
                    break;
                }
                default:
                    break;
            }

            MainApp_SendLedReq(me, LED_IND_ID_PROD_TRANS_OFF);

            me->systemStatus = SYSTEM_STA_STANDBY_HIGH;

            if (me->isTonePlaying == FALSE)
            {
			    AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, TRUE, /* NO USED*/0, /*No used*/0);
            }
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_ON:
        {
            TP_PRINTF("AseFepEvent_Event_SYSTEM_STATUS_ON\r\n");
            me->systemStatus = SYSTEM_STA_ON;
            MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
            MainApp_UpdateConnLed(me);
            AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, FALSE, /* NO USED*/0, /*No used*/0);
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_OFF:
        {
            TP_PRINTF("\r\n\r\n**** AseFepEvent_Event_SYSTEM_STATUS_OFF ==> power off ****\r\n\r\n");
            me->systemStatus = SYSTEM_STA_STANDBY_LOW;
            ret= Q_TRAN(&MainApp_PoweringDown);
            break;
        }
        case AseFepEvent_Event_FACTORY_RESET_START:
        {
            TP_PRINTF("AseFepEvent_Event_FACTORY_RESET_START\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_FACTORY_RESET_TRIGGERED);
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
            ret= Q_TRAN(&MainApp_WaitFactoryReset); //will mute amplifer on state
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_RESTART:
        {
            TP_PRINTF("AseFepEvent_Event_SYSTEM_STATUS_RESTART\r\n");
            ret= Q_TRAN(&MainApp_SoftReset); //will mute amplifer on state
            break;
        }
        case AseFepEvent_Event_SYSTEM_STATUS_ON_NO_OPERATION:
        {
            TP_PRINTF("AseFepEvent_Event_SYSTEM_STATUS_ON_NO_OPERATION\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_PRODUCT_IDLE); // product led go to dim.
            me->systemStatus = SYSTEM_STA_IDLE;
            MainApp_UpdateConnLed(me);
            break;
        }
        case AseFepEvent_Event_COMFORT_TONE_START:
        {
            TP_PRINTF("COMFORT_TONE_START\r\n");
            me->isTonePlaying = TRUE;
            if (me->systemStatus == SYSTEM_STA_STANDBY_HIGH)
            {
                AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, FALSE, /* NO USED*/0, /*No used*/0);
            }
            //AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, FALSE);
            /* ASETK-227 ASETK-238: SUE expect FEP to change source to ASE-TK when TONE_START.
             *            ASE-TK may change source again after TONE_DONE.
             */
            MainApp_SwitchAudioSource(me, AUDIO_SOURCE_WIFI);
            break;
        }
        case AseFepEvent_Event_COMFORT_TONE_DONE:
        {
            TP_PRINTF("COMFORT_TONE_DONE\r\n");
            me->isTonePlaying = FALSE;
            if (me->systemStatus == SYSTEM_STA_STANDBY_HIGH)
            {
                AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, TRUE, /* NO USED*/0, /*No used*/0);
            }
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
    {/* This flag indicates that MCU fail to communicat with ASE-TK. */
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
            if((volFadeEvent.target_volume >= MIN_VOLUME && volFadeEvent.target_volume <= MAX_VOLUME)
                && (me->isVolChanged == FALSE))
            {
                 me->absoluteVol = volFadeEvent.target_volume;
            }
            AudioSrv_SetAudio(AUDIO_VOL_FADE_SETT_ID, /*enable*/TRUE, volFadeEvent.target_volume, volFadeEvent.fade_duration);
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
            TP_PRINTF("AseFepMessage_aseFepTunnel_tag (not implement currently)\r\n");
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
            //TODO
            break;
        }
        case AseFepMessage_aseFepSetPositionSoundModeReq_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepSetPositionSoundModeReq_tag\r\n");
            MainApp_AseTkSetPositionSoundMode(me, aseTkEvt->aseFepCmd.OneOf.aseFepSetPositionSoundModeReq);
            break;
        }
        case AseFepMessage_aseFepGetPositionSoundModeReq_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepGetPositionSoundModeReq_tag\r\n");
            MainApp_AseTkGetPositionSoundMode(me);
            break;
        }
        case AseFepMessage_aseFepNetworkInfo_tag:
        {
            TP_PRINTF("AseFepMessage_aseFepNetworkInfo_tag\r\n");
            MainApp_AseTkNetworkInfo(me, aseTkEvt->aseFepCmd.OneOf.aseFepNetworkInfo); 
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
            //TODO
            break;
        }
        default:
            break;
    }
    return ret;
}
