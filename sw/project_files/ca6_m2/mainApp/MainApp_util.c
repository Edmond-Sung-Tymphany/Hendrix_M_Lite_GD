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
#include "AseTkSrv.h"
#include "LedSrv.h"
#include "SettingSrv.h"
#include "MainApp_priv.h"

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
    return;
    ASSERT(ledIndID < LED_IND_ID_MAX);
    //TP_PRINTF("MainApp_SendLedReq: ledIndID=%d\r\n", ledIndID);
    LedSrv_SetPatt((QActive*)me, me->ledInds[ledIndID].leds, me->ledInds[ledIndID].patternId);
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

void MainApp_Mute(cMainApp* me, bool muteEnable2)
{
    if(me->audioSource!=AUDIO_SOURCE_SPDIF_IN)
    {
        AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, muteEnable2);
    }
}

eAudioSource MainApp_GetNextAvialableSource(cMainApp * const me)
{
    eAudioSource audioSrc = me->audioSource;
#ifdef HAS_AUXIN
    /* update jack in status */
    bool jackInStatus = *(bool*)Setting_Get(SETID_IS_AUXIN_PLUG_IN); //use one byte to store jack in status in setting DB.
    me->sourceHandler[AUDIO_SOURCE_AUXIN].bIsValid = (jackInStatus>>AUX_IN_STATUS_BITMASK) & 0x01;
#endif
    uint8 i = 0;
    for(i = 0; i < AUDIO_SOURCE_MAX; i++)
    {
        if(TRUE == me->sourceHandler[audioSrc].bIsValid)
        {
            break;
        }

        /* goto check next audio source*/
        audioSrc++;
        if(audioSrc >= AUDIO_SOURCE_MAX)
        {
            audioSrc = 0;
        }
    }
    return audioSrc;
}

void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source)
{
    eAudioSource currentSrc = *(uint8 *)Setting_Get(SETID_AUDIO_SOURCE);
    TP_PRINTF("MainApp_SwitchAudioSource: source=%d \r\n", source);
    
    //me->audioSource = MainApp_GetNextAvialableSource(me);
    if (source >= AUDIO_SOURCE_MAX || source <= AUDIO_SOURCE_MIN)
    {
        source = AUDIO_SOURCE_MIN;
    }

    if ((*(bool *)Setting_Get(SETID_AUXIN_MUSIC_DET)) || (*(bool *)Setting_Get(SETID_SPDIF_IN_MUSIC_DET)))
    {
        AudioSrv_SendMuteReq((QActive *)me, AUDIO_DSP_LINEOUT_MUTE, FALSE);
    }
    else
    {
        AudioSrv_SendMuteReq((QActive *)me, AUDIO_DSP_LINEOUT_MUTE, TRUE);
    }
#ifndef NDEBUG
    bool enableDspTuning = FALSE;
    if (source == AUDIO_SOURCE_ETHERNET)
    {
        // Note: This source mode actually is used as DSP tuning mode.
        enableDspTuning = TRUE;
        AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, enableDspTuning, 0);
        Setting_Set(SETID_IS_DSP_TUNING, &enableDspTuning);
        MainApp_SendLedReq(me, me->sourceHandler[source].ledInd);
    }
    else if (currentSrc == AUDIO_SOURCE_ETHERNET)
    {
        AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, enableDspTuning, 0);
        Setting_Set(SETID_IS_DSP_TUNING, &enableDspTuning);
        MainApp_SendLedReq(me, me->sourceHandler[AUDIO_SOURCE_WIFI].ledInd);
    }
#endif
    if (currentSrc != source)
    {
        me->audioSource = source;
        
        //MainApp_SendLedReq(me, me->sourceHandler[me->audioSource].ledInd);
        eAudioChannel audioChannel = me->sourceHandler[me->audioSource].audioChannel;
        AudioSrv_SetChannel((QActive *)me, audioChannel);
        Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
    }
}

