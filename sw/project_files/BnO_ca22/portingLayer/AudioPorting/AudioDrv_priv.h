/**
 * @file        AudioDrv_priv.h
 * @brief       This file implementes the middle layer of the audio service
 * @author      Bob.Xu
 * @date        2015-06-15
 * @copyright   Tymphany Ltd.
 */
#ifndef AUDIODRV_PRIV_H
#define AUDIODRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AudioDrv.h"

/* private functions / data */

static void AudioDrv_VolFadeTimerCallBack(void *pCbPara);
static void AudioDrv_DspVolSetId_Handler(void);
static void AudioDrv_SetOverheat(cAudioDrv *me, bool overheat);
static void AudioDrv_AmpHealthMonitor();
static void AudioDrv_SetFadeVolume(cAudioDrv *me, uint32 target_volume, uint32 fade_duration);
static void AudioDrv_TempMonitor();
static void AudioDrv_SetMute(bool mute);
static void AudioDrv_UpdateAudioStatus();

#ifdef __cplusplus
}
#endif

#endif /* AUDIODRV_PRIV_H */
