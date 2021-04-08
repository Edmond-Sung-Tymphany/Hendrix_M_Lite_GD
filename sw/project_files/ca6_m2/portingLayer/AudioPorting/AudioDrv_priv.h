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
static void AMP_TempMonitor();
static void AudioDrv_AmpHealthMonitor();
static bool AudioDrv_IsAsetkPluggedIn(void);
static bool AudioDrv_IsAuxinJackPluggedIn(void);
static bool AudioDrv_IsOptJackPluggedIn(void);
static bool AudioDrv_IsAsetkHasMusicOn(void);
static bool AudioDrv_IsAuxinHasMusicOn(void);
static bool AudioDrv_IsOptHasMusicOn(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIODRV_PRIV_H */
