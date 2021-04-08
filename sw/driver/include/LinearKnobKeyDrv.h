/**
 * @file        LinearKnobKeyDrv.h
 * @brief       The knob key driver(Linear adc value) interfaces and implementation
 * @author      Bob.Xu
 * @date        2015-09-22
 * @copyright   Tymphany Ltd.
 */

#ifndef LENEAR_KNOBKEYDRV_H
#define	LENEAR_KNOBKEYDRV_H

#include "KeyDrv.h"
#ifdef	__cplusplus
extern "C" {
#endif

SUBCLASS(cLinearKnobKeyDrv,cKeyDrv)
	const tLinearKnobKeyDevice* pLinearKnobKeyConfig;
    int8 preIndex; /* the previous position of the adc range*/
METHODS
    
/**
* Key Driver(Linear adc value) object constructor
* @param[in]    me              the Key Driver object
* @param[in]    pConfig         configuration of the Key instance
* @param[in]    keyID           the keyID
*/
void LinearKnobKeyDrv_Ctor(cLinearKnobKeyDrv *me, const tLinearKnobKeyDevice *pKeyConfig, eKeyID keyID);

/**
* Key Driver(Linear adc value) object destructor
* @param[in]    me              the Key Driver object
*/
void LinearKnobKeyDrv_Xtor(cLinearKnobKeyDrv *me);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* LENEAR_KNOBKEYDRV_H */

