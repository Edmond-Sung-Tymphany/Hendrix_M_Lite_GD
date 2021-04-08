/**
 * @file        AudioDrv.h
 * @brief       This file implementes the middle layer of the audio service
 * @author      Bob.Xu
 * @date        2015-06-15
 * @copyright   Tymphany Ltd.
 */

#ifndef AUDIO_DRV_H
#define AUDIO_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "deviceTypes.h"

typedef struct _tVolFadeParam
{
    bool    isFadingInProg; // 0: no volume fade event  1: volume fade in progress
    bool    fadeType; // 0: fade down  1: fade up
    uint32  fadeStep;
    uint32  fadeStepTime; //in ms
    uint32  targetVol;
}tVolFadeParam;

CLASS(cAudioDrv)
    /* private data */
    BOOL drvIsReady;
    uint16 nextDelayTime;
    uint16 initState; //type: eAudioDrvInitState
METHODS
    /* public functions */
void AudioDrv_Ctor(cAudioDrv *me);
void AudioDrv_Xtor(cAudioDrv *me);

/**
 * Function for all hardware initialization
 * @param[in]   me  the audio driver object
 */
BOOL AudioDrv_Init(cAudioDrv *me);

/**
 * Function to change the channel.
 * @param[in]     theChannelToSet, the channel which will replace the current channel
 */
void AudioDrv_SetChannel(eAudioChannel audioChannel);

/**
 * Function to update the source in status including jackin status, music steam status and so on.
 */
void AudioDrv_UpdateStatus(cAudioDrv *audioDrvObj);

/**
 * Function to update the source in status including jackin status
 */
void AudioDrv_CheckJackInStatus(void);

/**
 * Function to update the music stream status
 */
void AudioDrv_CheckMusicStreamStatus(void);

/**
 * Function to monitor the amp status
 */
void AudioDrv_AmpSafetyMonitor();

/**
 * Function to change audio settings, such as EQ and system tunning.
 * @param     pAudioEvt, a pointer of the audio event
 */
void AudioDrv_SetAudio(cAudioDrv *me, eAudioSettId audioSettId, BOOL enabled, uint32 param, uint32 param2);

/**
 * Function to mute the system.
 * @param     mute or unmute
 */
void AudioDrv_Mute(eAudioMuteType muteType, bool muteEnable);

/**
 * Function to mute specific channel .
 * @param[in]   channel_maks    This parameter defines which channels should be muted.
 */
void AudioDrv_MuteChannel(uint32 channel_mask);

void AudioDrv_SetDspVol(uint32 vol);

#ifdef FENDER_ORANGE
void AudioDrv_ResetStandbyCounter(void);

#ifdef HAS_SSM3582_AMP_OUTPUT
/**
 * Function to set Amp output
 * @param[in]   level
 * @param[in]   
 */
void AudioDrv_SetAmpOutput(uint8 level);

/**
 * Function to get Amp temperature
 * @param[in]   level
 * @param[in]   
 */
uint16 AudioDrv_GetAmpTemperature(void);
#endif

#endif

END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_DRV_H */

