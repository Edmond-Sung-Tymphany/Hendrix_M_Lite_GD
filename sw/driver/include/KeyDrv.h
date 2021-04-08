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
 * @file        KeyDrv.h
 * @brief       The key driver interfaces and implementation 
 * @author      Bob.Xu 
 * @date        2014-02-13
 * @copyright   Tymphany Ltd.
 */

#ifndef KEYDRV_H
#define	KEYDRV_H

#include "cplus.h"
#include "attachedDevices.h"
#ifdef	__cplusplus
extern "C" {
#endif

typedef enum
{
    KEY_INVALIDE_STATE = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_DEBOUNCING
}eKeyState;

CLASS(cKeyDrv)
    eKeyID keyID; /* set key function type, such as VOLUME_UP key */
    eKeyState keyState;
    eKeyState keySimulationState;
    bool isCreated;
#ifdef HAS_PARAM_KEY
    int32 param;
#endif
    void(*KeyUpdateStatusCb)(cKeyDrv *me);
    void(*KeyStartScanCb)(cKeyDrv *me);
    int32(*KeyGetRawDataCb)(cKeyDrv *me);
METHODS
    
/**
* Key Driver object constructor
* @param[in]    me              the Key Driver object
* @param[in]    pConfig         configuration of the Key instance
* @param[in]    keyID           the keyID
*/
void KeyDrv_Ctor(cKeyDrv *me, const tKeyboardDevice *pConfig, void *pKeyboardAttachedObj);

/**
* Key Driver object destructor
* @param[in]    me              the Key Driver object
*/
void KeyDrv_Xtor(cKeyDrv *me,const tKeyboardDevice *pKeyboardConfig);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* KEYDRV_H */

