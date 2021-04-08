
//TODO: change for FS UI

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
#include "AseNgSrv.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "AudioDrv.h"
#include "tym_qp_lib.h"
#include "MainApp.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "pb_encode.h"
#include "fep_addr.h"


/*****************************************************************
 * Definition
 *****************************************************************/
/* To avoid send too many log to ASE-TK, this file print to UART only
 */
#undef TP_PRINTF
#ifndef NDEBUG
#define TP_PRINTF(...)  ProjBsp_Printf(/*toAse:*/FALSE, __FILE__, __LINE__, __VA_ARGS__)
#else
#define TP_PRINTF(...)
#endif  // NDEBUG



/*****************************************************************
 * Function Prototype
 *****************************************************************/


/*****************************************************************
 * Function Implemenation
 *****************************************************************/
static void MainApp_AseNgVolChanged(cMainApp * const me, Proto_AseFep_Event *pAseFepEvent)
{
    ASSERT(pAseFepEvent);
    Proto_Dsp_AbsoluteVolume *pVolEvt = &pAseFepEvent->data.volume;

    //TP_PRINTF("MainApp_AseNgVolChanged: which_data=%d, has_vol=%d, vol=%d, %dms\r\n", pAseFepEvent->which_data, pVolEvt->has_volume, pVolEvt->volume, pVolEvt->fade_duration);
    if(pAseFepEvent->which_data == Proto_AseFep_Event_volume_tag &&
            pVolEvt->has_volume &&
            me->audioMode == AUDIO_MODE_NORMAL)
    {
        //error handling
        if(pVolEvt->volume < MIN_VOLUME || pVolEvt->volume > MAX_VOLUME)
        {
            TP_PRINTF("*** ASE send wrong volume: vol=%d ***\r\n", pVolEvt->volume);
            return;
        }

        /* CAXVII-40:
         * As fading volume move from AseFep_Event_Type_VOLUME_CHANGED to AseFep_ReqResp_DSP_VOLUME_FADE,
         * the fade_duration parameter here is no longer used.
         */
        me->absoluteVol = pVolEvt->volume;
        TP_PRINTF("ASE set volume = %d (no fading)\r\n", me->absoluteVol);
        AudioSrv_SetVolume(pVolEvt->volume);

        //Update Connective LED
        if(me->isTonePlaying == FALSE &&
                (me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE))
        {
            //MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
        }
    }
}

static void MainApp_AseNgToneTouchHandler(cMainApp * const me, Proto_AseFep_Event *pAseFepEvent)
{

}


static void MainApp_AseNgSetLineInSensitivity(cMainApp * const me, Proto_AseFep_Req *pAseFepReq)
{
    ASSERT(pAseFepReq);
    Proto_Dsp_LineInSensitivity *pLSenEvt = &pAseFepReq->data.lineInSensitivity;

    if(pAseFepReq->which_data == Proto_AseFep_Req_lineInSensitivity_tag  &&
            pLSenEvt->has_sensitivity)
    {
        uint32 lsens = (uint32)pLSenEvt->sensitivity;
        //TP_PRINTF("ASE-TK Set Linein Sensitivity: %d \r\n", pAseFepReq->data.lineInSensitivity.sensitivity);
        AudioSrv_SetAudio(AUDIO_LINEIN_SENSITIVITY_SETT_ID, TRUE, lsens, /*No used*/0);
    }

    AseNgSrv_GenericResponse(/*success:*/TRUE, pAseFepReq->id, pAseFepReq->type);
}



static void MainApp_AseNgSetInput(cMainApp * const me, Proto_AseFep_Req *pAseFepReq)
{
    ASSERT(pAseFepReq);
    Proto_Dsp_RequestAudioInput *pAudioEvt = &pAseFepReq->data.audioInput;

    if(pAseFepReq->which_data == Proto_AseFep_Req_audioInput_tag &&
            pAudioEvt->has_local)
    {
        //TP_PRINTF("ASE-NG Set Local playback: %d, has_input=%d, input=%d\r\n", pAudioEvt->local, pAudioEvt->has_input, pAudioEvt->input);
        if(pAudioEvt->local

                //Always play local aux-in
#ifdef FSxMK2_BRING_UP
                || (pAudioEvt->has_input && pAudioEvt->input == Proto_Dsp_RequestAudioInput_AudioInput_LINE)
#endif
          )
        {
            MainApp_SwitchAudioSource(me, AUDIO_SOURCE_EXT_SOURCE);
        }

        else
        {
            MainApp_SwitchAudioSource(me, AUDIO_SOURCE_ASE);
        }
    }

    AseNgSrv_GenericResponse(/*success:*/TRUE, pAseFepReq->id, pAseFepReq->type);
}


static void MainApp_AseNgPlayDataHandler(cMainApp * const me, Proto_AseFep_Event *pAseFepEvent)
{
    ASSERT(pAseFepEvent);
    Proto_Player_Data *pPlyEvt = &(pAseFepEvent->data.playerData);

    if(pAseFepEvent->which_data == Proto_AseFep_Event_playerData_tag)
    {
        if(pPlyEvt->has_state)
        {
            switch(pPlyEvt->state)
            {
            case Proto_Player_Data_State_PLAYING:
                TP_PRINTF("state: play\r\n");
                me->musicPlaying = TRUE;

                /* Change LED to BLUE led when play BLUETOOTH source.
                 */
                if(pPlyEvt->has_source)
                {
                    me->playSource = pPlyEvt->source;
                }

                break;

            case Proto_Player_Data_State_PAUSED:
                TP_PRINTF("state: pause\r\n");
                me->musicPlaying = FALSE;
                break;

            case Proto_Player_Data_State_STOPPED:
                TP_PRINTF("state: stop\r\n");
                me->musicPlaying = FALSE;
                break;

            default:
                break;
            }
        }

        //            /* Do NOT blink LED again if the event is triggered by local key pressed. */
        //            bool keyPressed = FALSE;
        //            keyPressed = Setting_GetEx(SETID_IS_LOCAL_KEY_PRESS, &keyPressed);
        //            if (keyPressed == TRUE)
        //            {
        //                keyPressed = FALSE;
        //                Setting_Set(SETID_IS_LOCAL_KEY_PRESS, &keyPressed);
        //            }
        //            else
        //            {
        //            MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
        //            }

        MainApp_UpdateConnLed(me);
        MainApp_UpdateProdLed(me);
        MainApp_MuteUpdate(me);
    }
}


static void MainApp_AseNgProdLedModeSet(cMainApp * const me, Proto_AseFep_Req *pAseFepReq)
{
    ASSERT(pAseFepReq);
    QState ret = Q_UNHANDLED();
    Proto_Production_LedModeSet *pLedMode = &pAseFepReq->data.ledMode;

    /* Move it to end of MainApp_AseNgProdLedModeSet() will cause erorr
     * Don't know why...
     */
    AseNgSrv_GenericResponse(/*success:*/true, pAseFepReq->id, pAseFepReq->type);

    if(pAseFepReq->which_data == Proto_AseFep_Req_ledMode_tag &&
            pLedMode->has_led)
    {
        if((pLedMode->onTimeMs > 0) && (pLedMode->offTimeMs == 0))
        {
            switch(pAseFepReq->data.ledMode.led)
            {
            case Proto_Production_LedModeSet_StatusLed_NetBlue:
                //LedSrv_SetPatt((QActive*)me, LED_MASK_ALL_LEDS, SOLID_PAT_BLUE);
                break;

            case Proto_Production_LedModeSet_StatusLed_NetOrange:
                //LedSrv_SetPatt((QActive*)me, LED_MASK_ALL_LEDS, SOLID_PAT_GREEN);
                break;

            case Proto_Production_LedModeSet_StatusLed_NetRed:
                //LedSrv_SetPatt((QActive*)me, LED_MASK_ALL_LEDS, SOLID_PAT_RED);
                break;

            case Proto_Production_LedModeSet_StatusLed_NetWhite:
                //LedSrv_SetPatt((QActive*)me, LED_MASK_ALL_LEDS, SOLID_PAT_WHITE);
                break;
            }
        }

        else if((pLedMode->onTimeMs == 0) && (pLedMode->offTimeMs == 0))
        {
            LedSrv_SetPatt((QActive*)me, LED_MASK_ALL_LEDS, OFF_PATT);
        }
    }
}


static QState MainApp_AseNgPowerRequest(cMainApp * const me, Proto_AseFep_Req *pAseFepReq)
{
    ASSERT(pAseFepReq);
    QState ret = Q_UNHANDLED();
    Proto_System_PowerRequest *pPwrReq = &pAseFepReq->data.powerRequest;

    /* As CAXVII-30 said, generic respone is not necessary
     */
    AseNgSrv_GenericResponse(/*success:*/true, pAseFepReq->id, pAseFepReq->type);

    if(pAseFepReq->which_data == Proto_AseFep_Req_powerRequest_tag &&
            pPwrReq->has_type)
    {
        /* As CAXVII-30 said, ASE is ready to power off when send PowerRequset.
         * FEP can ignore delay parameters and pwoer off ASE immediately
         */
//        if( pPwrReq->has_delay_ms )
//        {
//            uint32 delay_ms;
//            if(pAseFepReq->data.powerRequest.delay_ms > 500)
//            {
//                delay_ms= 500;
//            }
//            else
//            {
//                delay_ms= pAseFepReq->data.powerRequest.delay_ms;
//            }
//            TP_PRINTF("\r\nASE PowerRequest: delay %dms\r\n", delay_ms);
//            BSP_ExpBlockingDelayMs(delay_ms); //accurary delay 2*delay_ms
//        }

        // Power type
        if(pPwrReq->type == Proto_System_PowerRequest_Type_POWER_OFF)
        {
            TP_PRINTF("\r\nASE PowerRequest: power-off\r\n\r\n");

            //MainApp_TurnOffConnLed(me);
            if(me->systemStatus == SYSTEM_STA_ON)
            {
                //MainApp_SendLedReq(me, LED_IND_ID_PROD_TRANS_OFF);
            }
            else if(me->systemStatus == SYSTEM_STA_IDLE)
            {
                //MainApp_SendLedReq(me, LED_IND_ID_PROD_TRANS_DIM_2_OFF_WHITE);
            }

            me->systemStatus = SYSTEM_STA_STANDBY_LOW;

            ret = Q_TRAN(&MainApp_PoweringDown);
        }

        else if(pPwrReq->type == Proto_System_PowerRequest_Type_POWER_RESTART)
        {
            TP_PRINTF("\r\nASE PowerRequest: power-reset\r\n\r\n");
            me->systemStatus = SYSTEM_STA_STANDBY_LOW;
            ret = Q_TRAN(&MainApp_AseNgBootingUp);
        }

        else
        {
            TP_PRINTF("\r\nASE PowerRequest: unknown\r\n\r\n");
        }
    }

    return ret;
}


static void MainApp_AseNgSetPositionSoundMode(cMainApp * const me, Proto_AseFep_Req *pAseFepReq)
{
    ASSERT(pAseFepReq);
    Proto_Dsp_RequestPositionSoundMode *pSoundMode = &pAseFepReq->data.positionSoundMode;

    if(pAseFepReq->which_data == Proto_AseFep_Req_positionSoundMode_tag &&
            pSoundMode->has_positionSoundMode)
    {
        if(pSoundMode->positionSoundMode.has_orientation)
        {
            TP_PRINTF("ASE Set Sound Mode: orientation=%d (ignore)\r\n", pSoundMode->positionSoundMode.orientation);
        }

        if(pSoundMode->positionSoundMode.has_position)
        {
            TP_PRINTF("ASE Set Sound Mode: position=%d \r\n", pSoundMode->positionSoundMode.position);

            //make sure ASE position have the same definition with DSP position.
            ASSERT(Proto_Dsp_PositionSoundMode_Position_CORNER == SPEAKER_POSITION_CORNER);
            AudioSrv_SetAudio(AUDIO_POS_SOUND_MODE_SETT_ID, TRUE, pSoundMode->positionSoundMode.position, /*No used*/0);
        }
    }
}



static QState MainApp_AseFepEventHandler(cMainApp * const me, Proto_AseFep_Event *pAseFepEvent)
{
    QState ret = Q_UNHANDLED();

    if(pAseFepEvent->has_type)
    {
        switch(pAseFepEvent->type)
        {
        case Proto_AseFep_Event_Type_BOOTED:
            break;

        case Proto_AseFep_Event_Type_SYSTEM_STATUS_STANDBY:

            //MainApp_DimConnLed(me);
            if(me->systemStatus == SYSTEM_STA_ON)
            {
                //MainApp_SendLedReq(me, LED_IND_ID_PROD_TRANS_OFF);
            }
            else //SYSTEM_STA_IDLE, SYSTEM_STA_STANDBY_HIGH
            {
                //MainApp_SendLedReq(me, LED_IND_ID_PROD_TRANS_DIM_2_OFF_WHITE);
            }

            me->systemStatus = SYSTEM_STA_STANDBY_HIGH;
            break;

        case Proto_AseFep_Event_Type_SYSTEM_STATUS_ON:
            me->systemStatus = SYSTEM_STA_ON;
            MainApp_UpdateConnLed(me);
            MainApp_UpdateProdLed(me);
            break;

        case Proto_AseFep_Event_Type_SYSTEM_STATUS_ASE_RESTART:
            //ret= Q_TRAN(&MainApp_SoftReset); //will mute amplifer on state
            //MainApp_SendLedReq(me, LED_IND_ID_FACTORY_RESET_TRIGGERED);
            //MainApp_SendLedReq(me, LED_IND_ID_ALL_BG_OFF, /*force:*/TRUE);
            //ret= Q_TRAN(&MainApp_WaitFactoryReset); //will mute amplifer on state
            break;

        case Proto_AseFep_Event_Type_FACTORY_RESET_DONE:
            break;

        case Proto_AseFep_Event_Type_SYSTEM_STATUS_ON_NO_OPERATION:
            me->systemStatus = SYSTEM_STA_IDLE;
            MainApp_UpdateConnLed(me);
            MainApp_UpdateProdLed(me);
            break;

        case Proto_AseFep_Event_Type_TUNNEL:
            if(pAseFepEvent->data.productionTunnel.has_data)
            {
                AseNgSrv_FillTunnelMsg((uint8*)&pAseFepEvent->data.productionTunnel.data.bytes[0],
                                       pAseFepEvent->data.productionTunnel.data.size);
            }

            break;

        case Proto_AseFep_Event_Type_SW_UPDATE_STARTED:
            //MainApp_SendLedReq(me, LED_IND_ID_SW_UPDATING);
            ret = Q_TRAN(&MainApp_WaitUpgrade); //will mute amplifer on state
            break;

        case Proto_AseFep_Event_Type_SW_UPDATE_FINISHED:
            //TODO: back to normal LED?
            //MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
            MainApp_UpdateConnLed(me);
            TP_PRINTF("AseFepEvent_Event_SW_UPDATE_FINISHED ==> *** unexpected UPDATE_FINISH ***\r\n");
            break;

        case Proto_AseFep_Event_Type_SW_UPDATE_FAILED:
            //TODO: back to normal LED?
            //MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
            MainApp_UpdateConnLed(me);
            //AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, FALSE);
            TP_PRINTF("AseFepEvent_Event_SW_UPDATE_FINISHED ==> *** unexpected UPDATE_FAILED ***\r\n");
            break;

        case Proto_AseFep_Event_Type_COMFORT_TONE_START:
        {
            me->isTonePlaying = TRUE;

            /* ASETK-227 ASETK-238: SUE expect FEP to change source to ASE-TK when TONE_START.
             *            ASE-TK may change source again after TONE_DONE.
             */
            MainApp_SwitchAudioSource(me, AUDIO_SOURCE_ASE);
            MainApp_MuteUpdate(me);
            break;
        }

        case Proto_AseFep_Event_Type_COMFORT_TONE_DONE:
            me->isTonePlaying = FALSE;
            MainApp_MuteUpdate(me);
            break;

        case Proto_AseFep_Event_Type_VOLUME_CHANGED:
            MainApp_AseNgVolChanged(me, pAseFepEvent);
            break;

        case Proto_AseFep_Event_Type_MUTE_CHANGED:
            if(pAseFepEvent->data.mute.has_mute)
            {
                me->asetkMute = pAseFepEvent->data.mute.mute;
                MainApp_MuteUpdate(me);
            }

            break;

        case Proto_AseFep_Event_Type_NETWORK_INFO:
            if(pAseFepEvent->which_data == Proto_AseFep_Event_networkInfo_tag)
            {
                for(int i = 0; i < pAseFepEvent->data.networkInfo.networkInterface_count; i++)
                {
                    AseNgSrv_FillNetworkInfo(&pAseFepEvent->data.networkInfo.networkInterface[i]);
                }
            }

            MainApp_UpdateConnLed(me);
            break;

        case Proto_AseFep_Event_Type_PLAYER_DATA:
            MainApp_AseNgPlayDataHandler(me, pAseFepEvent);
            break;

        case Proto_AseFep_Event_Type_FACTORY_RESET_START:
            //MainApp_SendLedReq(me, LED_IND_ID_FACTORY_RESET_TRIGGERED);
            //MainApp_SendLedReq(me, LED_IND_ID_ALL_BG_OFF, /*force:*/TRUE);
            ret = Q_TRAN(&MainApp_WaitFactoryReset); //will mute amplifer on state
            break;

        case Proto_AseFep_Event_Type_BT_PAIRING_ENABLED:
            me->aseBtEnable = TRUE;
            MainApp_UpdateConnLed(me);
            break;

        case Proto_AseFep_Event_Type_BTLE_PAIRING_ENABLED:
            me->aseBtleEnable = TRUE;
            MainApp_UpdateConnLed(me);
            break;

        case Proto_AseFep_Event_Type_BT_PAIRING_DISABLED:
            me->aseBtEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;

        case Proto_AseFep_Event_Type_BTLE_PAIRING_DISABLED:
            me->aseBtleEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;


        case Proto_AseFep_Event_Type_BT_PAIRING_FAILED:
            //MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_FAIL);
            me->aseBtEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;

        case Proto_AseFep_Event_Type_BTLE_PAIRING_FAILED:
            //MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_FAIL);
            me->aseBtleEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;

        case Proto_AseFep_Event_Type_BT_PAIRING_SUCCEEDED:
            //MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_SUCCESSFULL);
            me->aseBtEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;

        case Proto_AseFep_Event_Type_BTLE_PAIRING_SUCCEEDED:
            //MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_SUCCESSFULL);
            me->aseBtleEnable = FALSE;
            MainApp_UpdateConnLed(me);
            break;

        case Proto_AseFep_Event_Type_LOG_MESSAGE_AVAILABLE:
            break;

        case Proto_AseFep_Event_Type_LOG_MESSAGE_UNAVAILABLE:
            break;

        case Proto_AseFep_Event_Type_DSP_TONE_TOUCH:
            MainApp_AseNgToneTouchHandler(me, pAseFepEvent);
            break;

        case Proto_AseFep_Event_Type_SYSTEM_STATUS_STANDBY_MULTIROOM:
            break;

        case Proto_AseFep_Event_Type_SYSTEM_STATUS_TURNING_ON:
            break;

        case Proto_AseFep_Event_Type_SYSTEM_STATUS_AUDIO_ONLY:
            break;

        case Proto_AseFep_Event_Type_BT_PLAYER_CONNECTED:
            me->bIsBtConnected = TRUE;
            MainApp_UpdateConnLed(me);
            break;

        case Proto_AseFep_Event_Type_BT_PLAYER_DISCONNECTED:
            me->bIsBtConnected = FALSE;
            MainApp_UpdateConnLed(me);
            break;

        default:
            break;
        }
    }

    return ret;
}




static QState MainApp_AseFepReqHandler(cMainApp * const me, Proto_AseFep_Req *pAseFepReq)
{
    QState ret = Q_UNHANDLED();

    if(pAseFepReq->has_type)
    {
        switch(pAseFepReq->type)
        {
        case Proto_AseFep_ReqResp_FIRMWARE_UPDATE_VERSION_INFO: //handle by ASE-NG server
            break;

        case Proto_AseFep_ReqResp_FIRMWARE_UPDATE_CHUNK: //ignore
            break;

        case Proto_AseFep_ReqResp_FIRMWARE_UPDATE_SWITCH_TO_BOOTLOADER: //handle by ASE-NG server
            break;

        case Proto_AseFep_ReqResp_HDMI_ARC_STATUS: //ignore
            break;

        case Proto_AseFep_ReqResp_HDMI_ARC_START: //ignore
            break;

        case Proto_AseFep_ReqResp_HDMI_ARC_END: //ignore
            break;

        case Proto_AseFep_ReqResp_HDMI_INPUT_SELECT: //ignore
            break;

        case Proto_AseFep_ReqResp_POWER_STATUS: //ignore
            break;

        case Proto_AseFep_ReqResp_LINE_IN_SENSITIVITY:
            MainApp_AseNgSetLineInSensitivity(me, pAseFepReq);
            break;

        case Proto_AseFep_ReqResp_AUDIO_INPUT:
            MainApp_AseNgSetInput(me, pAseFepReq);
            break;

        case Proto_AseFep_ReqResp_POSITION_SOUND_MODE:
            MainApp_AseNgSetPositionSoundMode(me, pAseFepReq);
            break;

        case Proto_AseFep_ReqResp_INTERNAL_SPEAKER_COMPENSATION:
            break;

        case Proto_AseFep_ReqResp_POWER_LINK_ALL_SENSES_STATUS:
            break;

        case Proto_AseFep_ReqResp_POWER_LINK_SET_ON: //ignore
            break;

        case Proto_AseFep_ReqResp_POWER_LINK_SET_MUTE: //ignore
            break;

        case Proto_AseFep_ReqResp_INTERNAL_AMPLIFIER_COMMAND: //ignore
            break;

        case Proto_AseFep_ReqResp_FEP_APPLICATION_IS_RUNNING: //handle by ASE-NG server
            break;

        case Proto_AseFep_ReqResp_EEB_TELEGRAM_TRANSMIT: //ignore
            break;

        case Proto_AseFep_ReqResp_PRODUCTION_LED_MODE_SET:
            MainApp_AseNgProdLedModeSet(me, pAseFepReq);
            break;

        case Proto_AseFep_ReqResp_PRODUCTION_GET_BUTTON_STATE: //ignore
            break;

        case Proto_AseFep_ReqResp_HDMI_CEC_SEND_STANDBY: //ignore
            break;

        case Proto_AseFep_ReqResp_HDMI_CEC_SEND_POWER_UP_TV: //ignore
            break;

        case Proto_AseFep_ReqResp_WPL_COMMAND: //ignore
            break;

        case Proto_AseFep_ReqResp_PUC_COMMAND_SEND: //ignore
            break;

        case Proto_AseFep_ReqResp_POWER_REQUEST:
            ret = MainApp_AseNgPowerRequest(me, pAseFepReq);
            break;

        case Proto_AseFep_ReqResp_HDMI_UHD_DEEP_COLOUR_ON:
            break;

        case Proto_AseFep_ReqResp_HDMI_UHD_DEEP_COLOUR_OFF:
            break;

        case Proto_AseFep_ReqResp_POWERSUPPLY_VOLTAGE:
            break;

        case Proto_AseFep_ReqResp_SEND_POWER_LINK_DATA:
            break;

        case Proto_AseFep_ReqResp_TOSLINK_OUT_ADJUST_SAMPLE_RATE:
            break;

        case Proto_AseFep_ReqResp_TOSLINK_OUT_VOLUME_REGULATION_ON:
            break;

        case Proto_AseFep_ReqResp_TOSLINK_OUT_VOLUME_REGULATION_OFF:
            break;

        case Proto_AseFep_ReqResp_GET_HDMI_UHD_DEEP_COLOUR_STATUS:
            break;

        case Proto_AseFep_ReqResp_GET_HDMI_AUDIO_FORMAT:
            break;

        case Proto_AseFep_ReqResp_GET_HDMI_INPUT_SELECTED:
            break;

        case Proto_AseFep_ReqResp_GET_HDMI_SENSE_STATUS:
            break;

        case Proto_AseFep_ReqResp_SPEAKER_ENABLE_COMMAND:
            break;

        case Proto_AseFep_ReqResp_HDMI_AUDIO_MODE_SELECT:
            break;

        case Proto_AseFep_ReqResp_LIGHT_SENSOR_TELEGRAM:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_GET_A2B_MODE:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_SET_A2B_MODE:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_SET_GAIN_AND_DELAY:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_SET_DRIVER_GAIN:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_SET_BASS_AND_ROOMEQ:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_GET_TOTAL_NODES:
            break;

        case Proto_AseFep_ReqResp_DSP_PARAMETER:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_GET_POWER_MODE:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_SET_POWER_MODE:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_WRITE_DSP_PARAM:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_SET_MUTE_MODE:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_GET_NTC_INFO:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_SYSTEM_RESTART:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_GET_GAIN_AND_DELAY:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_GET_DRIVER_GAIN:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_GET_BASS_AND_ROOMEQ:
            break;

        case Proto_AseFep_ReqResp_SOUNDWALL_SET_TEST_TONE:
            break;

        case Proto_AseFep_ReqResp_DSP_VOLUME_FADE:
        {
            if((pAseFepReq->which_data == Proto_AseFep_Req_dspAbsoluteVolume_tag)
                    && (pAseFepReq->data.dspAbsoluteVolume.has_fade_duration)
                    && (pAseFepReq->data.dspAbsoluteVolume.has_volume)
                    && (pAseFepReq->data.dspAbsoluteVolume.fade_duration > 0))
            {
                /* param ( vv:ffffff    8 bits volume : 24 bits fade duration )
                   param2 request id */
                me->absoluteVol = pAseFepReq->data.dspAbsoluteVolume.volume;
                uint32 param = pAseFepReq->data.dspAbsoluteVolume.volume << 24;
                param = param | (pAseFepReq->data.dspAbsoluteVolume.fade_duration & 0xFFFFFF);
                //TP_PRINTF("ASE set volume fading to %d, duration %d ms\r\n", pAseFepReq->data.dspAbsoluteVolume.volume, pAseFepReq->data.dspAbsoluteVolume.fade_duration);
                AudioSrv_SetAudio(AUDIO_VOL_FADE_SETT_ID, /*enable*/TRUE, param, pAseFepReq->id);
            }

            break;
        }

        default:
            break;
        }
    }

    return ret;
}


static QState MainApp_AseFepRespHandler(cMainApp * const me, Proto_AseFep_Resp *pAseFepResp)
{
    QState ret = Q_UNHANDLED();

    if(pAseFepResp->has_type)
    {
        bool genericRespDone = (pAseFepResp->has_genericResponse &&
                                pAseFepResp->genericResponse.has_status &&
                                pAseFepResp->genericResponse.status == Proto_Core_GenericResponse_Status_DONE);

        switch(pAseFepResp->type)
        {
        case Proto_FepAse_ReqResp_PING:
            break;

        case Proto_FepAse_ReqResp_PLAY:
            break;

        case Proto_FepAse_ReqResp_PAUSE:
            break;

        case Proto_FepAse_ReqResp_STOP:
            break;

        case Proto_FepAse_ReqResp_NEXT:
            break;

        case Proto_FepAse_ReqResp_PREV:
            break;

        case Proto_FepAse_ReqResp_NEXT_SOURCE:
            break;

        case Proto_FepAse_ReqResp_JOIN:
            break;

        case Proto_FepAse_ReqResp_PLAY_PAUSE_TOGGLE:
            break;

        case Proto_FepAse_ReqResp_SOUND_SILENCE_TOGGLE:
            break;

        case Proto_FepAse_ReqResp_SOUND:
            break;

        case Proto_FepAse_ReqResp_SILENCE:
            break;

        case Proto_FepAse_ReqResp_MUTE:
            break;

        case Proto_FepAse_ReqResp_UNMUTE:
            break;

        case Proto_FepAse_ReqResp_BT_PAIRING_ON:
            break;

        case Proto_FepAse_ReqResp_BT_PAIRING_OFF:
            break;

        case Proto_FepAse_ReqResp_BTLE_PAIRING_ON:
            break;

        case Proto_FepAse_ReqResp_BTLE_PAIRING_OFF:
            break;

        case Proto_FepAse_ReqResp_BT_PAIRING_TOGGLE:
            break;

        case Proto_FepAse_ReqResp_OFF:
            /* As CAXVII-30 said, FEP should not power off ASE when get response of OFF
             * FEP should wait PowerRequest commend then power off ASE
             */
            break;

        case Proto_FepAse_ReqResp_STORAGE:
            TP_PRINTF("AseFepEvent_Event_SYSTEM_STATUS_STORAGE\r\n");
#ifdef BnO_fs1
            ret = Q_TRAN(&MainApp_Storage);
#endif
            break;

        case Proto_FepAse_ReqResp_FACTORY_RESET:
            break;

        case Proto_FepAse_ReqResp_NETWORK_SETUP:
            break;

        case Proto_FepAse_ReqResp_STANDBY:
            break;

        case Proto_FepAse_ReqResp_ALL_STANDBY:
            break;

        case Proto_FepAse_ReqResp_NETWORK_INFO:
            MainApp_UpdateConnLed(me);
            break;

        case Proto_FepAse_ReqResp_VOLUME_CHANGE:
            break;

        case Proto_FepAse_ReqResp_WPL_COMMAND:
            break;

        default:
            break;
        }
    }

    return ret;
}


QState MainApp_AseNgMsgHandler(cMainApp * const me, QEvt const * const e)
{
    AseNgStateIndEvt* aseNgEvt = (AseNgStateIndEvt*) e;
    QState ret = Q_UNHANDLED();

    //TODO: recover comment when watch-dog ready
//    if(aseNgEvt->bIsComWdgTimeOut)
//    {   /* This flag indicates that MCU fail to communicat with ASE-TK. */
//        ASSERT(0);
//        MainApp_DelayedErrorReboot(me);
//        return ret;
//    }

    switch(aseNgEvt->aseFepCmd.which_OneOf)
    {
    case Proto_Core_AseFepMessage_aseFepEvent_tag:
        ret = MainApp_AseFepEventHandler(me, &aseNgEvt->aseFepCmd.OneOf.aseFepEvent);
        break;

    case Proto_Core_AseFepMessage_aseFepReq_tag:
        ret = MainApp_AseFepReqHandler(me, &aseNgEvt->aseFepCmd.OneOf.aseFepReq);
        break;

    case Proto_Core_AseFepMessage_aseFepResp_tag:
        ret = MainApp_AseFepRespHandler(me, &aseNgEvt->aseFepCmd.OneOf.aseFepResp);
        break;

    default:
        break;
    }

    return ret;
}
