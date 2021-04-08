/**
 * @file        KnobKeyDrv.h
 * @brief       The knob key driver(key type) interfaces and implementation
 * @author      Daniel.Qin
 * @date        2015-03-04
 * @copyright   Tymphany Ltd.
 */

#ifndef KNOBKEYDRV_H
#define	KNOBKEYDRV_H

#include "KeyDrv.h"
#ifdef	__cplusplus
extern "C" {
#endif

SUBCLASS(cKnobKeyDrv,cKeyDrv)
    const tKnobKeyboardDevice* pKnobKeyboardConfig;
    eRotateStage currentStage;
METHODS
    
/**
* Key Driver(adc type) object constructor
* @param[in]    me              the Key Driver object
* @param[in]    pConfig         configuration of the Key instance
* @param[in]    keyID           the keyID
*/
void KnobKeyDrv_Ctor(cKnobKeyDrv *me, const tKnobKeyboardDevice *pKnobKeyboardConfig, eKeyID keyID);

/**
* Key Driver(adc type) object destructor
* @param[in]    me              the Key Driver object
*/
void KnobKeyDrv_Xtor(cKnobKeyDrv *me);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* KNOBKEYDRV_H */

