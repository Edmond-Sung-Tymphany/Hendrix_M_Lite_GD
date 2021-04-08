/*
*
****************************************************************************
* Copyright (C) 2016 Bosch Sensortec GmbH
*
* File : 
*
* Date : 2016/06/22
*
* Revision : 1.1.2 $
*
* Usage: Sensor Driver support file for BMI160 sensor
*
****************************************************************************
*
* \section License
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
*
*   Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the distribution.
*
*   Neither the name of the copyright holder nor the names of the
*   contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER
* OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
*
* The information provided is believed to be accurate and reliable.
* The copyright holder assumes no responsibility
* for the consequences of use
* of such information nor for any infringement of patents or
* other rights of third parties which may result from its use.
* No license is granted by implication or otherwise under any patent or
* patent rights of the copyright holder.
**************************************************************************/

/*! \file bmi160_support.h
    \brief BMI160 Sensor Driver Support Header File */
/* user defined code to be added here ... */
#ifndef __LDC2114_H__
#define __LDC2114_H__

#include "I2CDrv.h"
#include "GpioDrv.h"

SUBCLASS(cLdc2114TouchKeyDrv,cKeyDrv)
    cI2CDrv   i2cDrv;
    eKeyID    keyId;
    uint8     seqNo;
METHODS

/**
* Key Driver(touch type) object constructor
* @param[in]    me              the Key Driver object
* @param[in]    pConfig         configuration of the Key instance
* @param[in]    keyID           the keyID
*/
void Ldc2114TouchKeyDrv_Ctor(cLdc2114TouchKeyDrv *me, tTouchKeyboardDevice* cLdc2114TouchKeyDeviceDrv, eKeyID keyID, uint8 seqNo, uint8 gainSettId);

/**
* Key Driver(touch type) object destructor
* @param[in]    me              the Key Driver object
*/
void Ldc2114TouchKeyDrv_Xtor(cLdc2114TouchKeyDrv *me);

END_CLASS

#endif
