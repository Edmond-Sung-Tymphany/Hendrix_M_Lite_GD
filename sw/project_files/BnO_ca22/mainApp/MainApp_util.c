/**
*  @file      MainApp_util.c
*  @brief     util function for BnO mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "trace.h"
#include "controller.h"
#include "Setting_id.h"
#include "AseNgSrv.h"
#include "LedSrv.h"
#include "SettingSrv.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "HdmiSrv.h"
   
/*****************************************************************
 * Definition
 *****************************************************************/
#define AUX_IN_STATUS_BITMASK       AUXIN_JACK


/*****************************************************************
 * Function Implemenation
 *****************************************************************/

/* Note: The Api: MainApp_VolumeDown() and MainApp_VolumeUp() is just for HW bring-up in ES stage,
 * they can be removed if not need in later stages.
 */
void MainApp_VolumeDown(cMainApp * const me, uint8 step)
{
#ifndef DEBUG_QUICK_BOOT_NO_AUDIO
    // FIX-ME: avoid un-init setting server
    if (!Setting_IsReady(SETID_VOLUME))
        return;

    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
    if(vol >= step)
    {
        vol-=step;
    }
    else
    {
        vol = MIN_VOLUME;
    }
    AudioSrv_SetVolume(vol); //pass to DSPDrv1451_SetVol()
    TP_PRINTF("[%d] v = %d\r\n", __LINE__, vol);
#endif
}

void MainApp_VolumeUp(cMainApp * const me, uint8 step)
{
#ifndef DEBUG_QUICK_BOOT_NO_AUDIO
    // FIX-ME: avoid un-init setting server
    if (!Setting_IsReady(SETID_VOLUME))
        return;

    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
    if((step + vol)<= MAX_VOLUME)
    {
        vol+=step;
    }
    else
    {
        vol = MAX_VOLUME;
    }
    AudioSrv_SetVolume(vol); //pass to DSPDrv1451_SetVol()
    TP_PRINTF("[%d] v = %d\r\n", __LINE__, vol);
#endif
}

void MainApp_SendLedReq(cMainApp* me, eLedIndID ledIndID)
{
    ASSERT(ledIndID < LED_IND_ID_MAX);
    //TP_PRINTF("MainApp_SendLedReq: ledIndID=%d\r\n", ledIndID);

    /* Do NOT update LED indication if in DSP tuning mode or temperature level is less than TL_NORMAL. */
    if ((me->sysTempLevel < TL_NORMAL && me->ledInds[ledIndID].leds != LED_MASK_CONN_LEDS &&
        ledIndID != LED_IND_ID_PROD_FW_ERROR && ledIndID != LED_IND_ID_HW_OVER_HEAT &&
        ledIndID != LED_IND_ID_SHORT_PRESS && ledIndID != LED_IND_ID_CONTINUOUS_PRESS &&
        ledIndID != LED_IND_ID_LP_AND_VLP))
    {
        TP_PRINTF("Do NOT update LED indication if in DSP tuning mode or temperature level is less than TL_NORMAL. \r\n");
    }
    else
    {
        LedSrv_SetPatt((QActive*)me, me->ledInds[ledIndID].leds, me->ledInds[ledIndID].patternId);
    }
}

void MainApp_SendLedStripReq(cMainApp* const me, eStripPattern pattern)
{
    TP_PRINTF("MainApp_SendLedStripReq: pattern=%d\r\n", pattern);
    LedSrv_SetStripPatt((QActive*)me, pattern);
}

void MainApp_DelayedErrorReboot(cMainApp * const me)
{
    //delayed reboot for release biuld
    MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);

    /* wait 5 second(for finished LED indication), then reboot system. */
    me->tickHandlers[TIMER_ID_DELAYED_ERROR_REBOOT].timer = MAINAPP_DELAYED_ERROR_REBOOT_TIMEOUT_IN_MS;
}

void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}


void MainApp_MuteUpdate(cMainApp* const me)
{
    bool mute;
          
    if(me->audioMode!=AUDIO_MODE_NORMAL)
    {   //turn on amplifier on special test mode (EXT_SOURCE or DSP-TUNING)
        mute= FALSE;
    }
    else 
    {
        if( me->asetkMute )
            mute= TRUE;
        else if( me->isTonePlaying || me->musicPlaying )
            mute= FALSE;
        else
            mute= TRUE; //when not play music/tone, include standby mode
    }
        
    TP_PRINTF("MainApp_MuteUpdate: mute=%d (audioMode=%d, tonePlay=%d, musicPlay=%d, aseMute=%d)\r\n", mute, me->audioMode, me->isTonePlaying, me->musicPlaying, me->asetkMute );
    
    AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, mute);
    AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, mute, /* NO USED*/0, /*not-used*/0); 
}


/* This function is only for production test
 * Before ASE-TK finish LINE-IN implementation, it also used for source change
 */          
void MainApp_SetAudioMode(cMainApp * const me, eAudioMode newAudioMode)
{
#ifdef HAS_DSP_TUNING_MODE  
    if( newAudioMode >= AUDIO_MODE_NUM ) 
#else      
    if( newAudioMode >= AUDIO_MODE_DSP_ONLINE_TUNING ) 
#endif      
    {
        newAudioMode= AUDIO_MODE_NORMAL;
    }
    me->audioMode= newAudioMode;
    MainApp_MuteUpdate(me);
    
    switch (me->audioMode)
    {
        case AUDIO_MODE_NORMAL:
        { 
            /* Note when change back to NORMAL, source may be wrong
             * But it is only for debug, wrong status is acceptable
             */
            TP_PRINTF("MainApp_SetAudioMode: NORMAL\r\n");
            AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, FALSE, /*not-used*/0, /*not-used*/0);
            AudioSrv_SetChannel((QActive *)me, AUDIO_CHNANNEL_ASE);
            break;
        }
        case AUDIO_MODE_EXT_SOURCE: //fs1:SPDIF, fs1:LINE-IN
        {
            TP_PRINTF("MainApp_SetAudioMode: EXT_SOURCE\r\n");
            AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, FALSE, /*not-used*/0, /*not-used*/0);
            AudioSrv_SetChannel((QActive *)me, AUDIO_CHNANNEL_EXT_SOURCE);
            break;
        }
        case AUDIO_MODE_DSP_ONLINE_TUNING:
        {
            TP_PRINTF("MainApp_SetAudioMode: DSP_ONLINE_TUNING, and change source to ASE\r\n");
            AudioSrv_SetChannel((QActive *)me, AUDIO_CHNANNEL_ASE);
            AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, TRUE, /*not-used*/0, /*not-used*/0);        
            break;
        }
        default:
        {
            ASSERT(0);
        }
    }
    
    MainApp_UpdateConnLed(me);
    
}



void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source)
{
    eAudioSource currentSrc = *(uint8 *)Setting_Get(SETID_AUDIO_SOURCE);
    TP_PRINTF("MainApp_SwitchAudioSource: source=%d \r\n", source);

    //me->audioSource = MainApp_GetNextAvialableSource(me);
    if (source >= AUDIO_SOURCE_MAX || source < AUDIO_SOURCE_MIN)
    {
        source = AUDIO_SOURCE_MIN;
    }

    if (currentSrc != source)
    {
        me->audioSource = source;

        //MainApp_SendLedReq(me, me->sourceHandler[me->audioSource].ledInd);
        eAudioChannel audioChannel = me->sourceHandler[me->audioSource].audioChannel;
        AudioSrv_SetChannel((QActive *)me, audioChannel);
        Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
        MainApp_UpdateConnLed(me);
    }
}


eLedIndID MainApp_GetUpdatedProdLed(cMainApp * const me)
{
    eLedIndID prodLedIndId = LED_IND_ID_MAX;    
    TP_PRINTF("\r\nMainApp_GetUpdatedProdLed: sysTempLevel=%d, systemStatus=%d *** \r\n",
                 me->sysTempLevel,
                 me->systemStatus);

    if( me->sysTempLevel==TL_CRITICAL )
    {
        prodLedIndId= LED_IND_ID_PROD_FW_ERROR;
    }
    else if( me->sysTempLevel==TL_WARN || me->sysTempLevel==TL_SERIOUS )
    {
        prodLedIndId= LED_IND_ID_HW_OVER_HEAT;
    }
    else //TL_NORMAL
    {
        if (me->systemStatus == SYSTEM_STA_ON)
        {
            prodLedIndId= LED_IND_ID_POWERED_ON;
        }
        else if (me->systemStatus == SYSTEM_STA_IDLE)
        {
            prodLedIndId= LED_IND_ID_PRODUCT_IDLE;
        }
        else //SYSTEM_STA_STANDBY_HIGH, SYSTEM_STA_STANDBY_LOW, SYSTEM_STA_OFF
        {
            prodLedIndId= LED_IND_ID_PROD_OFF;
        }
    }
    
    return prodLedIndId;
}



eLedIndID MainApp_GetUpdatedConnLed(cMainApp * const me)
{
    eLedIndID connLedIndId = LED_IND_ID_MAX;

    AseNgNetworkInfo* pNetInfo = AseNgSrv_GetDecodedNetworkInfo();

    if (me->audioMode==AUDIO_MODE_EXT_SOURCE)
    {
        connLedIndId= LED_IND_ID_AUXIN_MODE;
    }
#ifdef HAS_DSP_TUNING_MODE
    else if (me->audioMode==AUDIO_MODE_DSP_ONLINE_TUNING)
    {
        connLedIndId= LED_IND_ID_DSP_TUNING_MODE;
    }
#endif    
    else //AUDIO_MODE_NORMAL
    {    
        //Update Connective LED
        if (me->aseBtleEnable || me->aseBtEnable)
        {
            connLedIndId = LED_IND_ID_BLE_PAIRING_ENABLED;
        }
        else if (pNetInfo->soft_ap_state == Proto_System_NetworkInfo_NetworkInterface_State_SCANNING ||
                 pNetInfo->soft_ap_state == Proto_System_NetworkInfo_NetworkInterface_State_CONNECTING ||
                 pNetInfo->soft_ap_state == Proto_System_NetworkInfo_NetworkInterface_State_AUTHENTICATING ||
                 pNetInfo->soft_ap_state == Proto_System_NetworkInfo_NetworkInterface_State_ACQUIRING ||
                 pNetInfo->soft_ap_state == Proto_System_NetworkInfo_NetworkInterface_State_CONNECTED)
        {
            connLedIndId = LED_IND_ID_WIFI_SETUP_IN_PROGRESS;
        }
        else if (pNetInfo->wifi_state == Proto_System_NetworkInfo_NetworkInterface_State_SCANNING ||
                 pNetInfo->wifi_state == Proto_System_NetworkInfo_NetworkInterface_State_CONNECTING ||
                 pNetInfo->wifi_state == Proto_System_NetworkInfo_NetworkInterface_State_AUTHENTICATING ||
                 pNetInfo->wifi_state == Proto_System_NetworkInfo_NetworkInterface_State_ACQUIRING )
        {
            connLedIndId = LED_IND_ID_WIFI_CONNECTING;
        }
        
        /* ASE have behavior: 
         * - playSource is correct if BLUETOOTH is paired
         * - playSource may be wrong (still BLUETOOTH) if BLUETOTOH is disconnected
         *
         * Workaround: 
         *  show BLUE led if BLUETOOTH is paired and it is active source
        */
        else if( me->playSource==Proto_Player_Data_Source_BLUETOOTH && me->bIsBtConnected )
        {
            if (me->systemStatus == SYSTEM_STA_ON)
                connLedIndId = LED_IND_ID_BLUETOOTH;
            else
                connLedIndId = LED_IND_ID_BLUETOOTH_IDLE;
        }
        
        else if (pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_SCANNING ||
                 pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_CONNECTING ||
                 pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_AUTHENTICATING ||
                 pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_ACQUIRING ||
                 pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_CONNECTED )
        {
            /* When insert Ethernet calbe on SoftAp mode, the temperary state is
             *    Wifi[6], Ethernet[2], SoftAp[6], Wifi:Configured[0],Quality[0]  ==> SoftAP
             *
             * To avoid show blue LED, we treat connecting/scanning/... state as connected
             */
            if (me->systemStatus == SYSTEM_STA_ON)
                connLedIndId = LED_IND_ID_ETH_CONNECTED;
            else
                connLedIndId = LED_IND_ID_ETH_CONNECTED_IDLE;
        }
        else if (pNetInfo->wifi_state == Proto_System_NetworkInfo_NetworkInterface_State_CONNECTED)
        {
            if (pNetInfo->wifi_quality == Proto_System_NetworkInfo_NetworkInterface_WiFi_Quality_EXCELLENT)
            {
                if (me->systemStatus == SYSTEM_STA_ON)
                    connLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_STRONG;
                else
                    connLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_STRONG_IDLE;
            }
            else if (pNetInfo->wifi_quality == Proto_System_NetworkInfo_NetworkInterface_WiFi_Quality_GOOD)
            {
                if (me->systemStatus == SYSTEM_STA_ON)
                    connLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_NORMAL;
                else
                    connLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_NORMAL_IDLE;
            }
            else /*if( pNetInfo->wifi_quality==Proto_System_NetworkInfo_NetworkInterface_WiFi_Quality_POOR ) */
            {
                if (me->systemStatus == SYSTEM_STA_ON)
                    connLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_WEAK;
                else
                    connLedIndId = LED_IND_ID_WIFI_SIG_STRENGTH_WEAK_IDLE;
            }
        }
        else if (pNetInfo->wifi_state == Proto_System_NetworkInfo_NetworkInterface_State_FAILED ||
                pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_FAILED)
        {
            connLedIndId = LED_IND_ID_WIFI_ERROR;
        }
        else if (pNetInfo->wifi_state == Proto_System_NetworkInfo_NetworkInterface_State_UNKNOWN &&
                pNetInfo->ethernet_state == Proto_System_NetworkInfo_NetworkInterface_State_UNKNOWN &&
                pNetInfo->soft_ap_state == Proto_System_NetworkInfo_NetworkInterface_State_UNKNOWN )
        {
            /* When boot up, ASE-TK send this temperary state, do not change LED
             */
        }
        else /* if( pNetInfo->wifi_state==NetworkInfo_State_DISCONNECTED) */
        {
            //ignore pNetInfo->wifi_configure
            if (me->systemStatus == SYSTEM_STA_ON)
                connLedIndId = LED_IND_ID_WIFI_UNCONFIGURED;
            else if (me->systemStatus == SYSTEM_STA_IDLE || me->systemStatus == SYSTEM_STA_STANDBY_HIGH)
                connLedIndId = LED_IND_ID_WIFI_UNCONFIGURED_IDLE;
            else { //SYSTEM_STA_STANDBY_LOW, SYSTEM_STA_OFF
            }
        }
    }
        
    return connLedIndId;
}


void MainApp_UpdateProdLed(cMainApp * const me)
{
    eLedIndID connLedIndId = LED_IND_ID_MAX;

    connLedIndId = MainApp_GetUpdatedProdLed(me);

    if(connLedIndId < LED_IND_ID_MAX)
    {
        MainApp_SendLedReq(me, connLedIndId);
    }
}


void MainApp_UpdateConnLed(cMainApp * const me)
{
    eLedIndID connLedIndId = LED_IND_ID_MAX;

    connLedIndId = MainApp_GetUpdatedConnLed(me);

    if(connLedIndId < LED_IND_ID_MAX)
    {
        MainApp_SendLedReq(me, connLedIndId);
    }
}

void MainApp_TurnOffConnLed(cMainApp * const me)
{
    eLedIndID currConnLedState = MainApp_GetUpdatedConnLed(me);
    switch(currConnLedState)
    {
        case LED_IND_ID_ETH_CONNECTED:  // fall through
        case LED_IND_ID_WIFI_SIG_STRENGTH_STRONG:
        {
            MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_ON_2_OFF_WHITE);
            break;
        }
        case LED_IND_ID_WIFI_SETUP_IN_PROGRESS: // fall through
        case LED_IND_ID_WIFI_CONNECTING:
        case LED_IND_ID_WIFI_SIG_STRENGTH_NORMAL:
        {
            MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_ON_2_OFF_AMBER);
            break;
        }
        case LED_IND_ID_WIFI_SIG_STRENGTH_WEAK: // fall through
        {
            MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_ON_2_OFF_AMBER);
            break;
        }
        case LED_IND_ID_BLE_PAIRING_ENABLED:
        case LED_IND_ID_WIFI_ERROR:
        case LED_IND_ID_WIFI_UNCONFIGURED:
        case LED_IND_ID_BLUETOOTH:
        {
            MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_ON_2_OFF_BLUE);
            break;
        }
        case LED_IND_ID_ETH_CONNECTED_IDLE:
        case LED_IND_ID_WIFI_SIG_STRENGTH_STRONG_IDLE:
        {
            MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_DIM_2_OFF_WHITE);
            break;
        }
        case LED_IND_ID_WIFI_SIG_STRENGTH_NORMAL_IDLE:
        {
            MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_DIM_2_OFF_AMBER);
            break;
        }
        case LED_IND_ID_WIFI_SIG_STRENGTH_WEAK_IDLE:
        {
            MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_DIM_2_OFF_AMBER);
            break;
        }
        case LED_IND_ID_WIFI_UNCONFIGURED_IDLE:
        case LED_IND_ID_WIFI_ERROR_IDLE:
        case LED_IND_ID_BLUETOOTH_IDLE:
        {
            MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_DIM_2_OFF_BLUE);
            break;
        }
        default:
            break;
    }
}

void MainApp_DimConnLed(cMainApp * const me)
{
    eLedIndID currConnLedState = MainApp_GetUpdatedConnLed(me);
    switch(currConnLedState)
    {
        case LED_IND_ID_ETH_CONNECTED:
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
        case LED_IND_ID_WIFI_UNCONFIGURED:
        case LED_IND_ID_WIFI_ERROR:
        case LED_IND_ID_BLUETOOTH:
        {
            MainApp_SendLedReq(me, LED_IND_ID_CONN_TRANS_DIM_BLUE);
            break;
        }
        default:
            break;
    }
}



