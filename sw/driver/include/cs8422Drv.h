/**
* @file AudioSRCDrv.h
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 9-Apr-2015
* @copyright Tymphany Ltd.
*/

#ifndef CS8422DRV_H
#define CS8422DRV_H

#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"

CLASS(cAudioSRCDrv)
    uint8       deviceI2cAddr;
    cI2CDrv     *pAudioSRCI2c;
    tI2CDevice  *pI2CConfig;
    bool isCreated;
METHODS

void Cs8422AudioSRCDrv_Ctor(cAudioSRCDrv * me, cI2CDrv *pI2cObj);
void Cs8422AudioSRCDrv_Xtor(cAudioSRCDrv * me);

#endif //#ifndef CS8422DRV_H