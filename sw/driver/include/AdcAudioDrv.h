/**
* @file AudioAdcDrv.h
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 26-Mar-2015
* @copyright Tymphany Ltd.
*/

#ifndef AUDIO_ADC_DRV_H
#define AUDIO_ADC_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AudioDrv.h"
#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "AAdcDrv.h"

SUBCLASS(cAdcAudioDrv, cAudioDrv)
    cAAdcDrv    *pAAdcDrv;
METHODS

void AdcAudioDrv_Ctor(tHardwareOwnership * pHardwareOwnership);
void AudioAdcDrv_Xtor(cAdcAudioDrv * me);

END_CLASS

#ifdef __cplusplus
}
#endif

#endif //#ifndef AUDIO_ADC_DRV_H