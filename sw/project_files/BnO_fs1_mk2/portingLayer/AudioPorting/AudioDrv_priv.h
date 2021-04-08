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
static int32 AudioDrv_GetCrtIdIndex(eAudioSettId audioSettId);
static void AudioDrv_Set_DspVol(uint8 volLevel);
static void AudioDrv_DspCalGainSetId_Handler(eAudioSettId audioSettId, float fGainDb);
static uint16 AudioDrv_ConvertNtcToTemperature(uint16 ntcValue);
static void AudioDrv_UpdateTemp(cAudioDrv *me);
static void AudioDrv_SetOverheat(cAudioDrv *me, bool overheat);
static void AudioDrv_AmpHealthMonitor(cAudioDrv *me);
static void AudioDrv_AmpSafetyMonitor(cAudioDrv *me);
static void AudioDrv_SetFadeVolume(cAudioDrv *me, uint32 target_volume, uint32 fade_duration);

#ifdef HAA_DSP_ONLINE_TUNE    
static void AudioDrv_DspTuneUpdate(cAudioDrv *me);
#endif
static void AudioDrv_VolFadeTimerCallBack(void *pCbPara);

#ifdef __cplusplus
}
#endif

#endif /* AUDIODRV_PRIV_H */