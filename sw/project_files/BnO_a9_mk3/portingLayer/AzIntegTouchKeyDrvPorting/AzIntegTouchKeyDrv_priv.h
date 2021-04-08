/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/
/**
 * @file        AzIntegTouchKeyDrv.h
 * @brief       The key driver(key type) interfaces and implementation 
 * @author      Gavin Lee
 * @date        2015-12-16
 * @copyright   Tymphany Ltd.
 */

#ifndef AZ_INTEG_TOUCHKEYDRV_H
#define	AZ_INTEG_TOUCHKEYDRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "KeyDrv.h"
#include "cplus.h"
#include "commonTypes.h"
#include "I2CDrv.h"
#include "gpioDrv.h"
#include "deviceTypes.h"
#include "attachedDevices.h"
#include "product.config"
  
  
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV
SUBCLASS(cAzIntegTouchKeyDrv,cKeyDrv)
    const tAzIntegTouchKeyboardDevice* pKeyboardConfig;
    cI2CDrv   i2c333Drv;
    cI2CDrv   i2c263Drv;
    cGpioDrv  gpioDrv;
METHODS


/**
* Key Driver(touch type) object constructor
* @param[in]    me              the Key Driver object
* @param[in]    pConfig         configuration of the Key instance
* @param[in]    keyID           the keyID
*/
void AzIntegTouchKeyDrv_Ctor(cAzIntegTouchKeyDrv *me, const tAzIntegTouchKeyboardDevice *pTouchKeyboardConfig);

/**
* Key Driver(touch type) object destructor
* @param[in]    me              the Key Driver object
*/
void AzIntegTouchKeyDrv_Xtor(cAzIntegTouchKeyDrv *me);

END_CLASS

#endif /* #ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV */

#ifdef	__cplusplus
}
#endif

#endif	/* AZ_INTEG_TOUCHKEYDRV_H */

