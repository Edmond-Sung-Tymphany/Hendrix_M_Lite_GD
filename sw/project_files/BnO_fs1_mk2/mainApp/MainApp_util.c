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
#include <stdarg.h> //va_list
#include "product.config"
#include "trace.h"
#include "controller.h"
#include "Setting_id.h"
#include "AseNgSrv.h"
#include "LedSrv.h"
#include "SettingSrv.h"
#include "MainApp.h"
#include "MainApp_util.h"
#include "MainApp_priv.h"
#include "bl_common.h"



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
void MainApp_SendLedReq(cMainApp* const me, eLedIndID ledIndID, bool force)
{       
    /* Over-heat status always flash RED, can ont change LED pattern
     * But when power down (force==TRUE) still can show LED
     */
    uint32 errorReason= 0;
    errorReason= *(uint32*)Setting_GetEx(SETID_ERROR_REASON, &errorReason);
    if(errorReason && !force) {
        return;
    }
       
    if(me->audioMode!=AUDIO_MODE_NORMAL) {
        return;
    }
    
    TP_PRINTF("MainApp_SendLedReq: ledIndID=%d\r\n", ledIndID);
    ASSERT(ledIndID < LED_IND_ID_MAX);
    LedSrv_SetPatt((QActive*)me, me->ledInds[ledIndID].leds, me->ledInds[ledIndID].patternId);
}


void MainApp_WakeupFromIdle(cMainApp * const me)
{
    if( me->aseNetMode==Proto_AseFep_Event_Type_SYSTEM_STATUS_ON_NO_OPERATION ) {
        me->aseNetMode= Proto_AseFep_Event_Type_SYSTEM_STATUS_ON;
        MainApp_UpdateConnLed(me);
    }    
}


void MainApp_InitStatusVariables(cMainApp * const me)
{
    me->audioSource= AUDIO_SOURCE_ASE;
    me->audioPassEnable= FALSE;
    me->relativeVol= 0;
    me->tempLevelSystem= TL_NORMAL;
    
    //impact connective LED
    me->aseNetMode   = Proto_AseFep_Event_Type_SYSTEM_STATUS_ON;
    me->aseBtEnable  = FALSE;
    me->aseBtleEnable= FALSE;
    me->tonePlaying  = FALSE;
    me->musicPlaying = FALSE;
    
    /* ASE-TK v1.6.46 seems have bug, sometimes do not send UNMUTE after BOOTED.
     * To workaround it, FEP set unmute for default state.
     */
    me->asetkMute= FALSE;
    
    //impact connective + productive LED
    me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_CONN_PRIV;
    me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_PROD_WHITE;
    
    uint32 errorReason= 0; //means no error
    Setting_Set(SETID_ERROR_REASON, &errorReason);
    
    AseNgNetworkInfo* pNetInfo= AseNgSrv_GetDecodedNetworkInfo();
    memset(pNetInfo, 0, sizeof(AseNgNetworkInfo)); //set all to unknown
    
    //for special audio mode
    me->audioMode= AUDIO_MODE_NORMAL;
    me->absoluteVol=  0;
    
    //for debug feature
#ifdef  DEBUG_DEMO_FEATURE
    me->timeLastSoftApKey= 0;
#endif    
    
    Setting_Reset(SETID_DSP_PARAM_POSITION);
    Setting_Reset(SETID_DSP_PARAM_TREBLE);
    Setting_Reset(SETID_DSP_PARAM_BASS);
    Setting_Reset(SETID_DSP_PARAM_LOUDNESS);
}


void MainApp_DelayedErrorReboot(cMainApp * const me)
{
    //delayed reboot for release biuld
    //MainApp_SendLedReq(me, LED_IND_ID_ERROR);

    /* wait 5 second(for finished LED indication), then reboot system. */
    if( me->tickHandlers[TIMER_ID_DELAYED_ERROR_REBOOT].timer==0 )
    {
        me->tickHandlers[TIMER_ID_DELAYED_ERROR_REBOOT].timer = MAINAPP_DELAYED_ERROR_REBOOT_TIMEOUT_IN_MS;    
    }
}


void MainApp_UpdateProdLed(cMainApp * const me)
{
    eLedIndID ledIndId= LED_IND_ID_MAX;
        
#ifdef BnO_fs1
    TP_PRINTF("\r\n\r\n\r\n *** MainApp_UpdateProdLed: play=%d, netMode=%d, dc:%d, ch:%d, batt-level:%d *** \r\n\r\n\r\n",
                 me->musicPlaying, 
                 me->aseNetMode,
                 me->powerEvt.dcInStatus,
                 me->powerEvt.batteryInfo.chargerState, 
                 me->powerEvt.batteryInfo.battStatus);
#else
    TP_PRINTF("\r\n\r\n\r\n *** MainApp_UpdateProdLed: play=%d, netMode=%d *** \r\n\r\n\r\n",
                 me->musicPlaying,
                 me->aseNetMode);
#endif
    
    if( me->musicPlaying && !me->asetkMute )
    {
        if(me->aseNetMode==Proto_AseFep_Event_Type_SYSTEM_STATUS_ON)
        {
            ledIndId= LED_IND_ID_PLAYING;
        }
        else //Proto_AseFep_Event_Type_SYSTEM_STATUS_ON_NO_OPERATION or others
        {            
            ledIndId= LED_IND_ID_PLAYING_DIM;
        }
    }
    else
    {
        
#ifdef BnO_fs1

        //FS1
        if( !(me->powerEvt.dcInStatus)  )
        {   //No DC
            if( me->powerEvt.batteryInfo.battStatus==BatteryStatus_NO_BATTERY )
            {
                ledIndId= LED_IND_ID_PROD_OFF;
            }
            else if( me->powerEvt.batteryInfo.battStatus==BatteryStatus_LEVEL_CRITICAL ) // Battert capacity 0%~2%
            {
                //not update LED, will power off
            }
            else if( me->powerEvt.batteryInfo.battStatus==BatteryStatus_LEVEL_LOW ) // Battert capacity 3%~10%
            {
                ledIndId= LED_IND_ID_BATT_LOW_NO_DC;
            }
            else  // Battert capacity 11%~100%
            {
                ledIndId= LED_IND_ID_PROD_OFF;
            }
        }
        else
        {   //Have DC
            if( me->powerEvt.batteryInfo.battStatus==BatteryStatus_NO_BATTERY )
            {
                ledIndId= LED_IND_ID_PROD_OFF;
            }        
            else if( me->powerEvt.batteryInfo.battStatus==BatteryStatus_LEVEL_HIGH ) // Battert capacity 81%~100%
            {
                ledIndId= LED_IND_ID_PROD_OFF;
            }
            else if (CHARGER_STA_CHARGING == me->powerEvt.batteryInfo.chargerState)
            {
                ledIndId= LED_IND_ID_BATT_LOW_HAVE_DC;
            }
            else
            {
                ledIndId= LED_IND_ID_PROD_OFF;
            }
        }
        
#else   

        //FS2
        MainApp_SendLedReq(me, LED_IND_ID_PROD_OFF, /*force:*/FALSE);  

#endif /* #ifdef BnO_fs1 */
    }
    

    if( ledIndId != LED_IND_ID_MAX )
    {
        MainApp_SendLedReq(me, ledIndId, /*force:*/FALSE);
        if( ledIndId==LED_IND_ID_PROD_OFF )
        {
            me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_PROD_WHITE;
        }
        else 
        {
            me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_PROD_PREV;
        }
    }
}



void MainApp_UpdateConnLed(cMainApp * const me)
{
    AseNgNetworkInfo* pNetInfo = AseNgSrv_GetDecodedNetworkInfo();
    eLedIndID newConnLedIndId= LED_IND_ID_MAX;
    
    //Print message
    char *modeName= "unknown";
    if(me->aseNetMode==Proto_AseFep_Event_Type_SYSTEM_STATUS_ON)
        modeName= "ON";
    else if(me->aseNetMode==Proto_AseFep_Event_Type_SYSTEM_STATUS_ON_NO_OPERATION)
        modeName= "ON_IDLE";
    else if(me->aseNetMode==Proto_AseFep_Event_Type_SYSTEM_STATUS_STANDBY)
        modeName= "NET_STBY";
    else {
        ASSERT(0);
    }
    TP_PRINTF("\r\n\r\n\r\n**** MainApp_UpdateConnLed: netMode=%s, bt=%d, btle=%d, Wifi[%d], Ethernet[%d], SoftAp[%d], Wifi:Configured[%d],Quality[%d]  *****\r\n\r\n\r\n\r\n", 
                  modeName, me->aseBtEnable, me->aseBtleEnable, pNetInfo->wifi_state, pNetInfo->ethernet_state, pNetInfo->soft_ap_state, pNetInfo->wifi_configured, pNetInfo->wifi_quality);

    //For golden sample, always show
    uint32 gainAllow= TRUE;
    gainAllow= *(uint32*)Setting_GetEx(SETID_GAIN_ALLOW, &gainAllow);
    

    //Update Connective LED
    if(!gainAllow)
    {
        newConnLedIndId=LED_IND_ID_GOLDEN_SAMPLE;
    }
    else if(me->aseBtEnable|| me->aseBtleEnable)
    {
        newConnLedIndId=LED_IND_ID_BT_PAIRING_ENABLED;
    }
    else if(pNetInfo->ethernet_state==Proto_System_NetworkInfo_NetworkInterface_State_SCANNING ||
            pNetInfo->ethernet_state==Proto_System_NetworkInfo_NetworkInterface_State_CONNECTING ||
            pNetInfo->ethernet_state==Proto_System_NetworkInfo_NetworkInterface_State_AUTHENTICATING ||
            pNetInfo->ethernet_state==Proto_System_NetworkInfo_NetworkInterface_State_ACQUIRING ||
            pNetInfo->ethernet_state==Proto_System_NetworkInfo_NetworkInterface_State_CONNECTED )
    {
        /* When insert Ethernet calbe on SoftAp mode, the temperary state is
         *    Wifi[6], Ethernet[2], SoftAp[6], Wifi:Configured[0],Quality[0]  ==> SoftAP
         *
         * To avoid show blue LED, we treat connecting/scanning/... state as connected
         */
        if(me->aseNetMode==Proto_AseFep_Event_Type_SYSTEM_STATUS_ON)
            newConnLedIndId=LED_IND_ID_ETHERNET_MODE;
        else
            newConnLedIndId=LED_IND_ID_ETHERNET_MODE_IDLE;
    }
    else if(pNetInfo->soft_ap_state==Proto_System_NetworkInfo_NetworkInterface_State_SCANNING ||
            pNetInfo->soft_ap_state==Proto_System_NetworkInfo_NetworkInterface_State_CONNECTING ||
            pNetInfo->soft_ap_state==Proto_System_NetworkInfo_NetworkInterface_State_AUTHENTICATING ||
            pNetInfo->soft_ap_state==Proto_System_NetworkInfo_NetworkInterface_State_ACQUIRING ||
            pNetInfo->soft_ap_state==Proto_System_NetworkInfo_NetworkInterface_State_CONNECTED )
    {
        /* When set wifi on SoftAP mode, the sequence is
         *    (1) Wifi[6], Ethernet[6], SoftAp[5], Wifi:Configured[0],Quality[0]  ==> SoftAP
         *    (2) Wifi[6], Ethernet[6], SoftAp[2], Wifi:Configured[0],Quality[0]  ==> SOftAP connecting
         *    (3) Wifi[2], Ethernet[6], SoftAp[6], Wifi:Configured[0],Quality[0]  ==> Wifi connecting
         *    (4) Wifi[5], Ethernet[6], SoftAp[6], Wifi:Configured[0],Quality[0]  ==> Wifi connected
         *
         * To avoid show blue LED, we treat connecting/scanning/... state as connected
         */
        newConnLedIndId=LED_IND_ID_SOFTAP_MODE;
    }
    else if(pNetInfo->wifi_state==Proto_System_NetworkInfo_NetworkInterface_State_SCANNING ||
            pNetInfo->wifi_state==Proto_System_NetworkInfo_NetworkInterface_State_CONNECTING  ||
            pNetInfo->wifi_state==Proto_System_NetworkInfo_NetworkInterface_State_AUTHENTICATING ||
            pNetInfo->wifi_state==Proto_System_NetworkInfo_NetworkInterface_State_ACQUIRING )
    {
        newConnLedIndId=LED_IND_ID_WIFI_CONNECTING;
    }
    else if(pNetInfo->wifi_state==Proto_System_NetworkInfo_NetworkInterface_State_CONNECTED)
    {
        if( pNetInfo->wifi_quality==Proto_System_NetworkInfo_NetworkInterface_WiFi_Quality_EXCELLENT )
        {
            if(me->aseNetMode==Proto_AseFep_Event_Type_SYSTEM_STATUS_ON)
                newConnLedIndId=LED_IND_ID_WIFI_MODE_QUALITY_EXELENT;
            else
                newConnLedIndId=LED_IND_ID_WIFI_MODE_QUALITY_EXELENT_IDLE;
        }
        else if( pNetInfo->wifi_quality==Proto_System_NetworkInfo_NetworkInterface_WiFi_Quality_GOOD )
        {
            if(me->aseNetMode==Proto_AseFep_Event_Type_SYSTEM_STATUS_ON)
                newConnLedIndId=LED_IND_ID_WIFI_MODE_QUALITY_GOOD;
            else
                newConnLedIndId=LED_IND_ID_WIFI_MODE_QUALITY_GOOD_IDLE;
        }
        else /*if( pNetInfo->wifi_quality==WiFi_Quality_POOR ) */
        {
            if(me->aseNetMode==Proto_AseFep_Event_Type_SYSTEM_STATUS_ON)
                newConnLedIndId=Proto_System_NetworkInfo_NetworkInterface_WiFi_Quality_POOR;
            else
                newConnLedIndId=LED_IND_ID_WIFI_MODE_QUALITY_POOR_IDLE;
        }
    }
    else if(pNetInfo->wifi_state==Proto_System_NetworkInfo_NetworkInterface_State_FAILED || 
            pNetInfo->ethernet_state==Proto_System_NetworkInfo_NetworkInterface_State_FAILED )
    {
        newConnLedIndId= LED_IND_ID_WIFI_CONNECT_FAIL;
    }
    else if(pNetInfo->wifi_state==Proto_System_NetworkInfo_NetworkInterface_State_UNKNOWN &&
            pNetInfo->ethernet_state==Proto_System_NetworkInfo_NetworkInterface_State_UNKNOWN &&
            pNetInfo->soft_ap_state==Proto_System_NetworkInfo_NetworkInterface_State_UNKNOWN )
    {
        /* When boot up, ASE-TK send this temperary state, do not change LED
         */
    }
    else /* if( pNetInfo->wifi_state==NetworkInfo_State_DISCONNECTED) ||
          *     pNetInfo->ethernet_state==NetworkInfo_State_FAILED||
          *     pNetInfo->soft_ap_state==NetworkInfo_State_FAILED
          */
    {
        //ignore pNetInfo->wifi_configure
        if(me->aseNetMode==Proto_AseFep_Event_Type_SYSTEM_STATUS_ON)
            newConnLedIndId= LED_IND_ID_WIFI_UNCONFIGURED;
        else
            newConnLedIndId= LED_IND_ID_WIFI_UNCONFIGURED_IDLE;
    }

    if(newConnLedIndId<LED_IND_ID_MAX)
    {       
        MainApp_SendLedReq(me, newConnLedIndId, /*force:*/FALSE);  
        me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_CONN_PRIV;
    }
}


void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}


/* This function is only for production test
 * Before ASE-TK finish LINE-IN implementation, it also used for source change
 */          
void MainApp_SetAudioMode(cMainApp * const me, eAudioMode newAudioMode)
{
    if( newAudioMode >= AUDIO_MODE_NUM ) 
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
            TP_PRINTF("\r\n\r\nMainApp_SetAudioMode: NORMAL\r\n\r\n\r\n");
            MainApp_UpdateConnLed(me);
            AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, FALSE, /*not-used*/0, /*not-used*/0);
            AudioSrv_SetChannel((QActive *)me, AUDIO_CHNANNEL_ASETK);
            
            //Rollback original audio source
            MainApp_SwitchAudioSource(me, me->audioSource);
            
            break;
        }
        case AUDIO_MODE_EXT_SOURCE: //fs1:SPDIF, fs1:LINE-IN
        {
            TP_PRINTF("\r\n\r\nMainApp_SetAudioMode: EXT_SOURCE\r\n\r\n\r\n");
            //MainApp_SendLedReq is disabled, thus we call lower API directly
            LedSrv_SetPatt((QActive*)me, me->ledInds[LED_IND_ID_EXT_SOURCE].leds, me->ledInds[LED_IND_ID_EXT_SOURCE].patternId);
            AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, FALSE, /*not-used*/0, /*not-used*/0);
            AudioSrv_SetChannel((QActive *)me, AUDIO_CHNANNEL_EXT_SOURCE);
            break;
        }
        case AUDIO_MODE_DSP_ONLINE_TUNING:
        {
            TP_PRINTF("\r\n\r\nMainApp_SetAudioMode: DSP_ONLINE_TUNING\r\n\r\n\r\n");
            //MainApp_SendLedReq is disabled, thus we call lower API directly
            LedSrv_SetPatt((QActive*)me, me->ledInds[LED_IND_ID_DSP_ONLINE_TUNING].leds, me->ledInds[LED_IND_ID_DSP_ONLINE_TUNING].patternId);
            AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, TRUE, /*not-used*/0, /*not-used*/0);        
            break;
        }
        default:
        {
            ASSERT(0);
        }
    }
    
}


#ifdef BnO_fs1
void MainApp_SetDcStatus(cMainApp * const me, bool bDcOn)
{
    //TP_PRINTF("MainApp_SetDcStatus: set dc=%d\r\n", bDcIn);

    /* FS-245: 
     *    When remove charger on battery mode, and battery capacity <10%,
     *    the high volume output may cause system shutdown. Although ASE-TK will 
     *    limit volume to 45 on <10% capacity, it need long 2s to reponse new volume
     *
     * Solution:
     *    When DC remove, do the same volume change with ASE-TK (set volume to 45 
     *     on battery mode with <10% capacity), to avoid dynamic boost enable and
     *     high volume output
     */
    uint8 volLevel=0, battCapLevel=BatteryStatus_LEVEL_MID;
    volLevel= *(uint8*)Setting_GetEx(SETID_VOLUME, &volLevel);
    battCapLevel= *(uint8*)Setting_GetEx(SETID_BATTERY_CAPACITY_LEVEL, &battCapLevel);
    if(me->bDcOn && !bDcOn && volLevel>45 && battCapLevel==BatteryStatus_LEVEL_LOW)
    {
        TP_PRINTF("\r\n\r\n\r\n*** Low batt capacity and remove DC => set DSP volume to 45 ***\r\n\r\n\r\n");
        MainApp_SetAbsoluteVolume(me, /*volLevel:*/45);
    }
    me->bDcOn= bDcOn;
            
    AseNgSrv_SendPowerStatus();
    AudioSrv_SystemTuning(DSP_DC_STATUS_SETT_ID, bDcOn);
    //PowerDrv is automatically enable/disable charging, thus MainApp do not send to PowerDrv
}
#endif




/***************************************************
 * Audio Operations are disable
 * for Debug SPDIF/LINE-IN and DSP-TUNING-MODE
 ****************************************************/
void MainApp_VolumeButtonHandler(cMainApp * const me, int8 relativeVol)
{
    if(me->audioMode==AUDIO_MODE_NORMAL)    
    {
        me->relativeVol+= relativeVol;
        //TP_PRINTF("MainApp_VolumeButtonHandler, relativeVol= %d\r\n", me->relativeVol);
        
        /* If send events to ASE-TK too fast, ASE-TK have delayed response.
         * Thus we only send events on MainApp_VolumeChangeTimeoutHandler(),
         * to avoid fast sending.
         */
    }
    else if(me->audioMode==AUDIO_MODE_EXT_SOURCE)
    {
        int8 newAbsVol= relativeVol + me->absoluteVol;
        if(newAbsVol > MAX_VOLUME) {
            newAbsVol= MAX_VOLUME;
        }
        if(newAbsVol < MIN_VOLUME) {
            newAbsVol= MIN_VOLUME;
        }
        TP_PRINTF("MainApp_VolumeButtonHandler, absoluteVol= %d\r\n", newAbsVol);
        MainApp_SetAbsoluteVolume(me, newAbsVol);
    }
    else if(me->audioMode==AUDIO_MODE_DSP_ONLINE_TUNING)
    {
        TP_PRINTF("MainApp_VolumeButtonHandler, disable (AUDIO_MODE_DSP_ONLINE_TUNING)\r\n");
    } 
    else
    {
        ASSERT(0);
    }  
}


void MainApp_AsetkSetVolume(cMainApp* const me, int8 absoluteVol)
{
    if(me->audioMode==AUDIO_MODE_NORMAL)     
    {
        //TP_PRINTF("MainApp_AsetkSetVolume: %d\r\n", absoluteVol);  
        MainApp_SetAbsoluteVolume(me, absoluteVol);
    }
    else if(me->audioMode==AUDIO_MODE_EXT_SOURCE)
    {
        //do nothing
    }
    else if(me->audioMode==AUDIO_MODE_DSP_ONLINE_TUNING)
    {    
        //do nothing
    }  
    else
    {
        ASSERT(0);
    } 
}
                              

void MainApp_SetAbsoluteVolume(cMainApp* const me, int8 absoluteVol)
{
    ASSERT(absoluteVol>=MIN_VOLUME && absoluteVol<=MAX_VOLUME);
    //TP_PRINTF("MainApp_SetAbsoluteVolume: %d\r\n", absoluteVol);
    me->absoluteVol= absoluteVol;
    AudioSrv_SetVolume(absoluteVol);
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
        else if( me->tonePlaying || me->musicPlaying )
            mute= FALSE;
        else
            mute= TRUE; //when not play music/tone, include standby mode
    }
        
#ifdef DEBUG_DEMO_FEATURE
    TP_PRINTF("MainApp_MuteUpdate: mute=%d (audioMode=%d, tonePlay=%d, musicPlay=%d, asetkMute=%d)\r\n", mute, me->audioMode, me->tonePlaying, me->musicPlaying, me->asetkMute );
#else
    TP_PRINTF("MainApp_MuteUpdate: mute=%d (tonePlay=%d, musicPlay=%d, asetkMute=%d)\r\n", mute, me->tonePlaying, me->musicPlaying, me->asetkMute );
#endif
    
    if (mute)
    {
        AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
        MainApp_AudioShutdown(me, TRUE);
    }
    else
    {
        MainApp_AudioShutdown(me, FALSE);
        AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, FALSE);
    }
}



void MainApp_AudioShutdown(cMainApp* const me, bool enable)
{
    //TP_PRINTF("MainApp_AudioShutdown: shutdown=%d\r\n", enable);  
    AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, enable, /* NO USED*/0, /*not-used*/0);
    bool slow_charging= !enable; //fast charge when shutdown audio (means network standby)
    PowerSrv_Set((QActive *)me, POWER_SET_ID_SLOW_CHARGER, slow_charging);   
}


void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source)
{ 
    me->audioSource= source;
    Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
        
    if(me->audioMode==AUDIO_MODE_NORMAL)     
    {
        MainApp_MuteUpdate(me);
        
        if(AUDIO_SOURCE_ASE==source)
        {
            TP_PRINTF("MainApp_SwitchAudioSource: ASETK\r\n");
            AudioSrv_SetChannel((QActive *)me, AUDIO_CHNANNEL_ASETK);
        }
        else if(AUDIO_SOURCE_EXT_SOURCE==source)
        {
            TP_PRINTF("MainApp_SwitchAudioSource: EXT_SOURCE\r\n");
            AudioSrv_SetChannel((QActive *)me, AUDIO_CHNANNEL_EXT_SOURCE);
        }
        else
        {
            ASSERT(0);
        }
    }

    else if(me->audioMode==AUDIO_MODE_EXT_SOURCE)
    {
        //do nothing
    }
    else if(me->audioMode==AUDIO_MODE_DSP_ONLINE_TUNING)
    {    
        //do nothing
    } 
    else
    {
        ASSERT(0);
    }
}

           
#ifdef DEBUG_DEMO_FEATURE
void MainApp_DspBypassEnable(cMainApp * const me, bool enable)
{
    //Disable audio for audio debug mode
    if(me->audioMode==AUDIO_MODE_NORMAL)
    {
        AudioSrv_SetEq(DSP_PASSTHROUGH_SETT_ID, enable);
    }
    else if(me->audioMode==AUDIO_MODE_EXT_SOURCE)
    {
        AudioSrv_SetEq(DSP_PASSTHROUGH_SETT_ID, enable);
    }
    else if(me->audioMode==AUDIO_MODE_DSP_ONLINE_TUNING)
    {    
        //do nothing
    }
    else
    {
        ASSERT(0);
    }
}
#endif



void MainApp_WriteBootRequest(cMainApp * const me, eBootReq boot_req)
{    
    uint32 boot_req_new= boot_req;
    Setting_Set(SETID_BOOT_REQUEST, &boot_req_new);
    SettingSrv_BookkeepingEx(); //flush setting to flash
}




ePattern MainApp_GetReplyLedPattern(cMainApp * const me, eLedIndID ledIndId)
{   
    extern tPatternData patternConfig[PAT_MAX_NUMBER];    
    ePattern patt= me->ledInds[ledIndId].patternId;
    ASSERT(patt<PAT_MAX_NUMBER);
    
    int i;
    for(i=0 ; i<5 ; i++)
    {
        if( patternConfig[patt].nextPattern == PAT_MAX_NUMBER) 
        {
            return patt;
        }
        patt= patternConfig[patt].nextPattern;
    }

    ASSERT(0); //not found
    return PAT_MAX_NUMBER;
}



void MainApp_PrintVersion(cMainApp * const me)
{
    char *hw_ver = "not-ready";
    hw_ver= (char*)Setting_GetEx(SETID_HW_VER, hw_ver);

    char piu_ver[20];
    char *piu_ver_tmp= bl_readVersion( (void*)FEP_ADDR_PIU_VER );
    snprintf(piu_ver, sizeof(piu_ver), "%s", piu_ver_tmp);

    char ubl_ver[20];
    char *ubl_ver_tmp= bl_readVersion( (void*)FEP_ADDR_UBL_VER );
    snprintf(ubl_ver, sizeof(ubl_ver), "%s", ubl_ver_tmp);
    
    char *dsp_ver= "not-ready";
    dsp_ver= (char*)Setting_GetEx(SETID_DSP_VER, dsp_ver);
    
#ifndef NDEBUG
    char *build_type= "d";
#else
    char *build_type= "";
#endif                
    
    TP_PRINTF("\r\n--------------------------------------------------------------------------\r\n");
    TP_PRINTF(TP_PRODUCT "\r\n");
    TP_PRINTF("  SW:  PIU:%s  /  UBL:%s  /  SW:%s%s (DSP:%s)\r\n", piu_ver, ubl_ver, PRODUCT_VERSION_MCU, build_type, dsp_ver);
    TP_PRINTF("  HW:  %s \r\n", hw_ver);
    TP_PRINTF("--------------------------------------------------------------------------\r\n");

}

