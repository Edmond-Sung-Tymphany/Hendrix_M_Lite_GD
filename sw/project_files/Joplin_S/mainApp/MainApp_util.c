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
#include "LedSrv.h"
#include "SettingSrv.h"
#include "MainApp_priv.h"
#include "bit_shift_64.h"

/*****************************************************************
 * Definition
 *****************************************************************/
#define AUX_IN_STATUS_BITMASK       AUXIN_JACK


/*****************************************************************
 * Function Implemenation
 *****************************************************************/

void MainApp_InitStatusVariables(cMainApp * const me)
{
    uint32 temp;

    me->combinedKey = COM_KEY_INVALID;
    me->systemStatus = SYSTEM_STA_OFF;
    me->muteStatus = FALSE;
    me->vol = DEFAULT_VOLUME*2;

    /* Resume data from flash */
    temp = *(uint32_t *)Setting_Get(SETID_BASS);
    me->bass = (temp < MAX_BASS_STEPS) ? temp : (DEFAULT_BASS*2);

    temp = *(uint32_t *)Setting_Get(SETID_TREBLE);
    me->treble = (temp < MAX_TREBLE_STEPS) ? temp : (DEFAULT_TREBLE*2);

    temp = *(uint32_t *)Setting_Get(SETID_IS_RCA_ACTIVATED);
    me->sourceHandler[AUDIO_SOURCE_RCA].bIsValid = (temp <= TRUE) ? temp : FALSE;

    temp = *(uint32_t *)Setting_Get(SETID_AUDIO_SOURCE);
    me->audioSource = (temp < AUDIO_SOURCE_MAX) ? temp : AUDIO_SOURCE_DEFAULT;
}



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
    // To Do: setting volume step down here
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

    // To Do: setting volume step up here

    AudioSrv_SetVolume(vol); //pass to DSPDrv1451_SetVol()
    TP_PRINTF("[%d] v = %d\r\n", __LINE__, vol);
#endif
}

void MainApp_SendLedReq(cMainApp* me, eLedIndID ledIndID)
{
    ASSERT(ledIndID < LED_IND_ID_MAX);
    //TP_PRINTF("MainApp_SendLedReq: ledIndID=%d\r\n", ledIndID);
    bool enableDspTuning = FALSE;
#ifdef HAS_DSP_TUNING_MODE
    enableDspTuning = *(bool *)Setting_Get(SETID_IS_DSP_TUNING);
#endif
    /* Do NOT update LED indication if in DSP tuning mode or temperature level is lager than TL_NORMAL. */
    if (ledIndID == LED_IND_ID_PROD_FW_ERROR || (enableDspTuning == FALSE))
    {
        LedSrv_SetPatt((QActive*)me, me->ledInds[ledIndID].leds, me->ledInds[ledIndID].patternId);
    }
}

void MainApp_SetRotaterLedOn(cMainApp* me, eLed startLed, uint32 level)
{
    uint8 i;
    eLedMask leds;
    leds = (1ULL)<<(startLed);
    for(i = 0; i <= level; i++)
    {
        leds |= shift_left64(1ULL, (startLed + i));
    }
    LedSrv_SetPatt((QActive*)me, leds, SOLID_PAT_RED);
}

void MainApp_SetRotaterLedOff(cMainApp* me, eLed startLed, uint32 level)
{
    uint8 i;
    eLedMask leds = 0ULL;
    for(i = level+1; i <= 10; i++)
    {
        leds |= shift_left64(1ULL, (startLed + i));
    }
    LedSrv_SetPatt((QActive*)me, leds, OFF_PATT);
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

    AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, muteEnable2);
}

eAudioSource MainApp_GetNextAvialableSource(cMainApp * const me)
{
    eAudioSource audioSrc = me->audioSource;
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
            audioSrc = AUDIO_SOURCE_MIN;
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

#ifdef HAS_DSP_TUNING_MODE
    if (source == AUDIO_SOURCE_DSP_TUNING)
    {
        bool enableDspTuning = TRUE;
        AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, enableDspTuning, /*No used*/0, /*No used*/0);
        MainApp_SendLedReq(me, me->sourceHandler[source].ledInd);
        Setting_Set(SETID_IS_DSP_TUNING, &enableDspTuning);
    }
    else if (currentSrc == AUDIO_SOURCE_DSP_TUNING)
    {
        TP_PRINTF("In dsp tuning mode, we can NOT switch source. \r\n");
        return;
    }
#endif

    me->audioSource = source;
    if (currentSrc != source)
    {
        eAudioChannel audioChannel = me->sourceHandler[me->audioSource].audioChannel;
        AudioSrv_SetChannel((QActive *)me, audioChannel);
        Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
        MainApp_SendLedReq(me, LED_IND_ID_OFF_ALL_SRC_LED);
        MainApp_SendLedReq(me, me->sourceHandler[me->audioSource].ledInd);
    }
}

