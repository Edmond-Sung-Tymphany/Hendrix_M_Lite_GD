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
static void AudioDrv_AmpMute(BOOL muteEnable);
//#define AMP_MUTE                    AudioDrv_AmpMute(TRUE)
//#define AMP_UNMUTE                  AudioDrv_AmpMute(FALSE)
#define AMP_MUTE_IO                (audioGpioDrv.gpioConfig->pGPIOPinSet[1].gpioId)
/* private functions / data */
static bool AudioDrv_IsAuxinJackPluggedIn(void);
static bool AudioDrv_IsAuxinHasMusicOn(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIODRV_PRIV_H */