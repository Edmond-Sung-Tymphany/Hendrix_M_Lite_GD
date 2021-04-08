/**
 * @file        AdcKeyDrv.h
 * @brief       The key driver(key type) interfaces and implementation 
 * @author      Bob.Xu 
 * @date        2014-02-13
 * @copyright   Tymphany Ltd.
 */

#ifndef ADCKEYDRV_H
#define	ADCKEYDRV_H

#include "KeyDrv.h"
#ifdef	__cplusplus
extern "C" {
#endif

SUBCLASS(cAdcKeyDrv,cKeyDrv)
    const tAdcKeyboardDevice* pAdcKeyboardConfig;
METHODS
    
/**
* Key Driver(adc type) object constructor
* @param[in]    me              the Key Driver object
* @param[in]    pConfig         configuration of the Key instance
* @param[in]    keyID           the keyID
*/
void AdcKeyDrv_Ctor(cAdcKeyDrv *me, const tAdcKeyboardDevice *pAdcKeyboardConfig, eKeyID keyID);

/**
* Key Driver(adc type) object destructor
* @param[in]    me              the Key Driver object
*/
void AdcKeyDrv_Xtor(cAdcKeyDrv *me);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* ADCKEYDRV_H */

