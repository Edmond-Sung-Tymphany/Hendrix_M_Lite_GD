/**
 * @file        IRKeyDrv.h
 * @brief       The key driver(IR type) interfaces and implementation 
 * @author      Bob.Xu 
 * @date        2014-02-13
 * @copyright   Tymphany Ltd.
 */

#ifndef IRKEYDRV_H
#define	IRKEYDRV_H

#include "KeyDrv.h"
#include "deviceTypes.h"

#ifdef	__cplusplus
extern "C" {
#endif

SUBCLASS(cIRKeyDrv,cKeyDrv)
    const tIRKeyboardDevice* pKeyboardConfig;
METHODS
    
/**
* Key Driver(IR type) object constructor
* @param[in]    me              the Key Driver object
* @param[in]    pConfig         configuration of the Key instance
* @param[in]    keyID           the keyID
*/
void IRKeyDrv_Ctor(cIRKeyDrv *me, const tIRKeyboardDevice *pConfig, eKeyID keyID);

/**
* Key Driver (IR type) object destructor
* @param[in]    me              the Key Driver object
*/
void IRKeyDrv_Xtor(cIRKeyDrv *me);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* IRKEYDRV_H */

