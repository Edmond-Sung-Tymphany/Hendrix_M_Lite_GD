/**
 * @file        GpioKeyDrv.h
 * @brief       The key driver(GPIO type) interfaces and implementation 
 * @author      Bob.Xu 
 * @date        2014-02-13
 * @copyright   Tymphany Ltd.
 */

#ifndef GPIOKEYDRV_H
#define	GPIOKEYDRV_H

#include "KeyDrv.h"

#ifdef	__cplusplus
extern "C" {
#endif

SUBCLASS(cGpioKeyDrv,cKeyDrv)
    const tGpioKeyboardDevice* pKeyboardConfig;
METHODS
    
/**
* Key Driver(GPIO type) object constructor
* @param[in]    me              the Key Driver object
* @param[in]    pConfig         configuration of the Key instance
* @param[in]    keyID           the keyID
*/
void GpioKeyDrv_Ctor(cGpioKeyDrv *me, const tGpioKeyboardDevice *pConfig, eKeyID keyID);

/**
* Key Driver (GPIO type) object destructor
* @param[in]    me              the Key Driver object
*/
void GpioKeyDrv_Xtor(cGpioKeyDrv *me);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* GPIOKEYDRV_H */

