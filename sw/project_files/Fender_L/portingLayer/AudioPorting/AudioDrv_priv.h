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

#ifdef HAS_TPA3118_AMP
#define AMP_MUTE_ENABLE(x)      GpioDrv_SetBit(&(x), GPIO_OUT_AMP_T_MUTE);
#define AMP_MUTE_DISABLE(x)     GpioDrv_ClearBit(&(x), GPIO_OUT_AMP_T_MUTE); 
#endif

void AudioDrv_ShutDown(bool enable);

/* private functions / data */

#ifdef __cplusplus
}
#endif

#endif /* AUDIODRV_PRIV_H */
