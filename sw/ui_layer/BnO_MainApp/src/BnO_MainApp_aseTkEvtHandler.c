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
#include "tym_qp_lib.h"
#include "BnO_MainApp.h"

/*****************************************************************
 * Global Variable
 *****************************************************************/


/*****************************************************************
 * Function Implemenation
 *****************************************************************/
static void BnO_MainApp_AseTkVolChanged(cMainApp * const me, AseFepVolumeChangedEvent volEvt)
{
    if (volEvt.has_volume)
    {    
        //TP_PRINTF("ASE-TK Set Volume: %d\r\n", volEvt.volume);
        ASSERT(volEvt.volume>=MIN_VOLUME && volEvt.volume<=MAX_VOLUME);            
        AudioSrv_SetVolume(volEvt.volume);
    }        

#if 0
    uint8   vol         = *(uint8*)Setting_Get(SETID_VOLUME);
    int     volCurr     = volEvt.volume;
    uint16  aseTkVol  = MainApp_mcuVol2AseTkVol(vol);

    if(aseTkVol != volCurr)
    {
        uint8 vol_temp  = MainApp_AseTkVol2McuVol(volCurr);
        if((volCurr > aseTkVol) && (vol_temp <= vol))
        {/* There is a calculation error cause vol_temp <= vol in this case
          * So a margin of error should be added here.
          */
            vol_temp = vol_temp + MainApp_AseTkVol2McuVol_errorMargin(volCurr-aseTkVol);
        }
        AudioSrv_SetVolume(vol_temp);
    }
#endif
}


static QState BnO_MainApp_AseFepEventHandler(cMainApp * me, AseFepEvent aseFepEvent)
{
    QState ret= Q_UNHANDLED();
    switch (aseFepEvent.event)
    {
    case AseFepEvent_Event_SW_UPDATE_STARTED:
        //ASE-TK will restart, we should mute amplifer to avoid pop noise
        AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
        break;
    case AseFepEvent_Event_SW_UPDATE_FINISHED:
        break;
    case AseFepEvent_Event_SW_UPDATE_FAILED:
        break;
    case AseFepEvent_Event_BT_PAIRING_ENABLED:
    {
        //BnO_MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_ENABLED);
        break;
    }
    case AseFepEvent_Event_BT_PAIRING_DISABLED:
        break;
    case AseFepEvent_Event_BT_PAIRING_FAILED:
        break;
    case AseFepEvent_Event_BT_PAIRING_SUCCEEDED:
    {
        //BnO_MainApp_SendLedReq(me, LED_IND_ID_BT_MODE );
        break;
    }
    case AseFepEvent_Event_BTLE_PAIRING_ENABLED:
        break;
    case AseFepEvent_Event_BTLE_PAIRING_DISABLED:
        break;
    case AseFepEvent_Event_BTLE_PAIRING_FAILED:
        break;
    case AseFepEvent_Event_BTLE_PAIRING_SUCCEEDED:
        break;
    case AseFepEvent_Event_PLAYER_PLAYING:
        break;
    case AseFepEvent_Event_PLAYER_PAUSED:
        break;
    case AseFepEvent_Event_PLAYER_STOPPED:
        break;
    case AseFepEvent_Event_PLAYER_MUTED:
        //ASE-TK v1.15.16.71177340 seems only send muted on startup, then never send again
        break;
    case AseFepEvent_Event_PLAYER_UNMUTED:
        //ASE-TK v1.15.16.71177340 seems never send unmuted event
        break;
    case AseFepEvent_Event_SOFTAP_STARTED:
        break;
    case AseFepEvent_Event_SOFTAP_STOPPED:
        break;
    case AseFepEvent_Event_APPLE_WAC_STARTED:
        break;
    case AseFepEvent_Event_APPLE_WAC_STOPPED:
        break;
    case AseFepEvent_Event_APPLE_WAC_TIMEOUT:
        break;
    case AseFepEvent_Event_ASE_BOOTED:
        break;
    case AseFepEvent_Event_ASE_OVERHEAT_ALERT:
        break;
    case AseFepEvent_Event_SW_UPDATE_FEP_BOOTLOADER:
        break;
    case AseFepEvent_Event_SW_UPDATE_FEP_APPLICATION:
        break;
    case AseFepEvent_Event_SYSTEM_STATUS_STORAGE:
        //Product specific event, handle by MainApp
        break;
    case AseFepEvent_Event_SYSTEM_STATUS_STANDBY:
        ret= Q_TRAN(&MainApp_NetworkStandby);
        //ret= Q_TRAN(&MainApp_Active);
        break;
    case AseFepEvent_Event_SYSTEM_STATUS_ON:
        ret= Q_TRAN(&MainApp_Active);
        break;
    case AseFepEvent_Event_SYSTEM_STATUS_OFF:
    {   //Common event, handled by BnO_MainApp
        /* ASE_TK v1.15.16.71177340 send OFF on two situation
         *   (1) After FEP send FepAseCommand_Command_OFF
         *   (2) After AseFepEvent_Event_SW_UPDATE_STARTED
         * For (2), FEP should not power off system.
         * Thus we have bug:
         *   after firmware upgade, system power off instead of restart.
         */
        //ret= Q_TRAN(&MainApp_PoweringDown);
        break;
    }
    case AseFepEvent_Event_FACTORY_RESET_START:
    {   //Common event, handled by BnO_MainApp
        //Wait 3 seconds then reboot
      
        //TODO: consider to wait on other state (not reboot system), and remember to mute amplifiler
        //AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_MUTE, TRUE);
      
        TP_PRINTF("Get AseFepEvent_Event_FACTORY_RESET_START, will reboot after %dms\r\n", MAINAPP_FACTORY_RESET_TIMEOUT_MS);
        me->tickHandlers[TIMER_ID_FACTORY_RESET_TIMEOUT].timer = MAINAPP_FACTORY_RESET_TIMEOUT_MS;
        PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
        break;
    }
    case AseFepEvent_Event_FACTORY_RESET_DONE:
        break;
    default:
        break;
    }
    
    return ret;
}

void BnO_MainApp_BtEvtHandler(cMainApp * const me, QEvt const * const e)
{

}

QState BnO_MainApp_AseTkEvtHandler(cMainApp * const me, QEvt const * const e)
{
    AseTkStateIndEvt* aseTkEvt = (AseTkStateIndEvt*) e;
    QState ret= Q_UNHANDLED();

    switch(aseTkEvt->aseFepCmd.which_OneOf)
    {
        case AseFepMessage_aseFepEvent_tag:
        {
            ret= BnO_MainApp_AseFepEventHandler(me, aseTkEvt->aseFepCmd.OneOf.aseFepEvent);
            break;
        }
        case AseFepMessage_aseFepVolumeChangedEvent_tag:
        {   //Common behavior, handled by BnoMainApps
            BnO_MainApp_AseTkVolChanged(me, aseTkEvt->aseFepCmd.OneOf.aseFepVolumeChangedEvent); 
            break;
        }
        case AseFepMessage_aseFepSetLineInSensitivity_tag:
            break;
        case AseFepMessage_aseFepTunnel_tag:
            break;
        case AseFepMessage_aseFepSetAudioInputReq_tag:
        {   //do something.
            break;
        }
        case AseFepMessage_aseFepSetAudioOutputReq_tag:
            break;
        case AseFepMessage_aseFepAliveResp_tag:
            break;
        case AseFepMessage_aseFepSetPositionSoundModeReq_tag:
            break;
        case AseFepMessage_aseFepGetPositionSoundModeReq_tag:
            break;
        case AseFepMessage_aseFepNetworkInfo_tag:
            break;
        case AseFepMessage_aseFepReq_tag:
        {
            switch (aseTkEvt->aseFepCmd.OneOf.aseFepReq.request)
            {
            case AseFepReq_Request_PRODUCT_INFO:
                break;
            case AseFepReq_Request_POWER_STATUS:
                break;
            case AseFepReq_Request_VERSION_INFO:
                break;
            case AseFepReq_Request_STATISTICS:
                break;
            case AseFepReq_Request_NETWORK_INFO:
                break;
            }
            break;
        }
        case AseFepMessage_aseFepSetAudioPcmFormatCommand_tag:
            break;
        case AseFepMessage_aseFepSetInternalSpeakerCompensationCommand_tag:
            break;
        case AseFepMessage_aseFepVolumeFadeEvent_tag:
            break;
        default:
            break;
    }
    return ret;
}
