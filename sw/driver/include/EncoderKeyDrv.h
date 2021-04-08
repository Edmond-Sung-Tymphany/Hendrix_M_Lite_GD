/**
 * @file        RotaterKeyDrv.h
 * @brief       The key driver (Rotary Encoder) interfaces and implementation
 * @author      Jimmy.Zhong
 * @date        2016-05-13
 * @copyright   Tymphany Ltd.
 */

#ifndef ROTATER_KEY_DRV_H
#define	ROTATER_KEY_DRV_H

#include "KeyDrv.h"
#include "GpioDrv.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define ROTATER_ENC_A_MASK  (0x01)
#define ROTATER_ENC_B_MASK  (0x02)
#define ROTATER_UNINIT      (0xFF)

SUBCLASS(cEncoderKeyDrv,cKeyDrv)
    const tGpioEncoderKeyDevice* pKeyboardConfig;
    cGpioDrv    gpioDrv;
    int32       position;
    uint32      lastState;  // 00 or 11
    uint32      tranState;  // 10 or 01
METHODS

/**
* Key Driver (Rotary Encoder) object constructor
* @param[in]    me              the Key Driver object
* @param[in]    pConfig         configuration of the Key instance
*/
void EncoderKeyDrv_Ctor(cEncoderKeyDrv *me, const tGpioEncoderKeyDevice *pConfig);

/**
* Key Driver (Rotary Encoder) object destructor
* @param[in]    me              the Key Driver object
*/
void EncoderKeyDrv_Xtor(cEncoderKeyDrv *me);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* ROTATER_KEY_DRV_H */

