/**
* @file SpdifAudioAdcDrv.h
* @brief The devices attached to the product.
* @author Bob.Xu
* @date 7-Jul-2015
* @copyright Tymphany Ltd.
*/

#ifndef AUDIO_SPDIF_DRV_H
#define AUDIO_SPDIF_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AudioDrv.h"
#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "GpioDrv.h"

SUBCLASS(cSpdifAudioDrv, cAudioDrv)
    cGpioDrv    *pGpioDrv;
    uint16      digitalData;
    uint16      sampleCounter;
METHODS

void SpdifAudioDrv_Ctor(tHardwareOwnership * pHardwareOwnership);
void SpdifAudioDrv_Xtor(cSpdifAudioDrv * me);

END_CLASS

#ifdef __cplusplus
}
#endif

#endif //#ifndef AUDIO_SPDIF_DRV_H