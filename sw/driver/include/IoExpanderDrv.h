/**
 * @file        IoExpanderDrv.h
 * @brief       This file implements the driver for AW9523B
 * @author      Wesley Lee
 * @date        2014-06-06
 * @copyright   Tymphany Ltd.
 */

#ifndef IOEXPANDER_DRV_H
#define IOEXPANDER_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "product.config"
#include "I2CDrv.h"
#include "pattern.h"
#include "gpioDrv.h"

CLASS(cIoExpanderDrv)           // TODO: IO expander could be input
/* private data */
    cI2CDrv i2cDrv;
#ifdef IOEXPANDERDRV_RST_CONTROL
    cGpioDrv  gpioDrv;
#endif
    BOOL    isReady;
    eDeviceSubType  deviceSubType;
    uint32 count;
METHODS
    /* public functions */
void IoExpanderDrv_Ctor_aw9110b(cIoExpanderDrv *me, tIoeLedDevice *pIoeLedConfig);
void IoExpanderDrv_Xtor_aw9110b(cIoExpanderDrv *me);
void IoExpanderDrv_SetGpio_aw9110b(cIoExpanderDrv *me, uint8 port, uint8 pin);
void IoExpanderDrv_ClearGpio_aw9110b(cIoExpanderDrv *me, uint8 port, uint8 pin);
void IoExpanderDrv_SetBrightness_aw9110b(cIoExpanderDrv *me, uint8 port, uint8 pin, uint8 value);
void IoExpanderDrv_AutoBlink_aw9110b(cIoExpanderDrv *me, eIoeAutoPatt patt);
void IoExpanderDrv_AudoBlink_OnExp_aw9110b(cIoExpanderDrv *me, const tDevice* pConfig, eIoeAutoPatt patt);
void IoExpanderDrv_SetIoInput_aw9110b(cIoExpanderDrv *me, uint8 port, uint8 pin);
void IoExpanderDrvAw9523_SetBrightness_aw9110b(cIoExpanderDrv * me,uint8 port,uint8 pin,uint8 value);
void IoExpanderDrvAw9110b_SetBrightness(cIoExpanderDrv *me, uint8 port, uint8 pin, uint8 value);

void IoExpanderDrv_Ctor_aw9120(cIoExpanderDrv *me, tIoeLedDevice *pIoeLedConfig);
void IoExpanderDrv_Xtor_aw9120(cIoExpanderDrv *me, tIoeLedDevice *pIoeLedConfig);
void IoExpanderDrv_ReCtor_aw9120();

void IoExpanderDrv_TurnLedOn_aw9120(cIoExpanderDrv *me, uint8 pin);
void IoExpanderDrv_TurnLedOff_aw9120(cIoExpanderDrv *me, uint8 pin);
void IoExpanderDrv_SetBrightness_aw9120(cIoExpanderDrv *me, uint8 pin, uint8 brightness);
void IoExpanderDrv_AutoBlink_aw9120(cIoExpanderDrv *me, eIoeAutoPatt patt);

void IoExpanderDrv_Ctor_aw9523(cIoExpanderDrv *me, tIoeLedDevice *pIoeLedConfig);
void IoExpanderDrv_Xtor_aw9523(cIoExpanderDrv *me);
void IoExpanderDrv_SetBrightness_aw9523(cIoExpanderDrv *me, uint8 port, uint8 pin, uint8 value);
void IoExpanderDrv_SetGpio_aw9523(cIoExpanderDrv *me, uint8 port, uint8 pin);
void IoExpanderDrv_ClearGpio_aw9523(cIoExpanderDrv *me, uint8 port, uint8 pin);
uint8 IoExpanderDrv_ReadGpio_aw9523(cIoExpanderDrv *me, uint8 port, uint8 pin);

#ifdef HAS_IOEXPANDER_LED_TEST
void IoExpanderDrv_TestOn();
void IoExpanderDrv_TestOff();
#endif

END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* IOEXPANDER_DRV_H */

