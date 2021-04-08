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


typedef enum {
    AUDIO_DRV_INIT_DSP_POWER = 0,
    AUDIO_DRV_INIT_DSP_CODE,
    AUDIO_DRV_INIT_AMP_PVDD,
    AUDIO_DRV_INIT_TW_AMP,
    AUDIO_DRV_INIT_END,
}eAudioDrvInitState;

static void AudioDrv_ReadDspVersion();
void AudioDrv_ShutDown(bool enable);

/* private functions / data */

#ifdef __cplusplus
}
#endif

#endif /* AUDIODRV_PRIV_H */
