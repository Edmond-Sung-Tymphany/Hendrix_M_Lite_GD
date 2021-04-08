/**
* @file AudioDSPDrv.h
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 9-Apr-2015
* @copyright Tymphany Ltd.
*/

#ifndef AUDIO_DSP_DRV_H
#define AUDIO_DSP_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AudioDrv.h"
#include "DspDrv.h"
#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"

SUBCLASS(cDspAudioDrv, cAudioDrv)
    cDSPDrv     *pDspDrv;
    cI2CDrv     *pAudioAdcI2c;
METHODS

void DspAudioDrv_Ctor(tHardwareOwnership * pHardwareOwnership);
void AudioDSPDrv_Xtor(cDspAudioDrv * me);

END_CLASS

#ifdef __cplusplus
}
#endif


#endif //#ifndef AUDIO_SRC_DRV_H