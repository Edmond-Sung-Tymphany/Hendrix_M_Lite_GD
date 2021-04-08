/**
* @file AudioAdcDrv.h
* @brief The devices attached to the product.
* @author Bob.Xu
* @date 8-Jul-2015
* @copyright Tymphany Ltd.
*/

#ifndef GPIO_AUDIO_DRV_H
#define GPIO_AUDIO_DRV_H

#include "AudioDrv.h"
#include "cplus.h"
#include "commonTypes.h"
#include "GpioDrv.h"

#ifdef __cplusplus
extern "C" {
#endif

SUBCLASS(cGpioAudioDrv,cAudioDrv)
    cGpioDrv    *pGpioDrv;
METHODS

void GpioAudioDrv_Ctor(tHardwareOwnership * pHardwareOwnership);
void GpioAudioDrv_Xtor(cGpioAudioDrv * me);

END_CLASS

#ifdef __cplusplus
}
#endif

#endif //#ifndef GPIO_AUDIO_DRV_H