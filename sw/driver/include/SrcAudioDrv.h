/**
* @file AudioSRCDrv.h
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 9-Apr-2015
* @copyright Tymphany Ltd.
*/

#ifndef AUDIO_SRC_DRV_H
#define AUDIO_SRC_DRV_H

#ifdef __cplusplus
extern "C" {
#endif
#include "AudioDrv.h"
#include "SrcDrv.h"
#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"

SUBCLASS(cSrcAudioDrv, cAudioDrv)
    cSrcDrv     *srcDrv;
    cI2CDrv     *pAudioSRCI2c;
METHODS

void SrcAudioDrv_Ctor(tHardwareOwnership * pHardwareOwnership);
void AudioSRCDrv_Xtor(cSrcAudioDrv * me);

END_CLASS

#ifdef __cplusplus
}
#endif

#endif //#ifndef AUDIO_SRC_DRV_H