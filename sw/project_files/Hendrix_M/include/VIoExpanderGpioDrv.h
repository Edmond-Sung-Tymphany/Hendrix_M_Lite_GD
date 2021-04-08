
/**
 * @file        VIoExpanderGpioDrv.h
 * @brief       This file implements the virtul GPIO driver for IO expander, use to split different GPIO
 *              from One physic IO expander, let it can serve Different function driver.
 * @author      Colin Chen
 * @date        2018-01-25
 * @copyright   Tymphany Ltd.
 */


#ifndef VIOEXPANDER_GPIO_DRV_H
#define VIOEXPANDER_GPIO_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "IoExpanderDrv.h"
#include "IoExpanderLedDrv.h"


CLASS(cVIoExpanderGpioDrv)
/* private data */
cIoExpanderDrv *pIoExpander;
tIoeLedDevice  *pIoExpanderLedDevice;
METHODS
/* public functions */
void VIoExpanderGpioDrv_Ctor(cVIoExpanderGpioDrv *me, tVIoeGpioDevice *pVIoeGpioDevice);
void VIoExpanderGpioDrv_Xtor(cVIoExpanderGpioDrv *me);
void VIoExpanderGpioDrv_SetGpio(cVIoExpanderGpioDrv *me, uint8 index);
void VIoExpanderGpioDrv_ClearGpio(cVIoExpanderGpioDrv *me, uint8 index);
uint8 VIoExpanderGpioDrv_ReadGpio(cVIoExpanderGpioDrv *me, uint8 index);
END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* VIOEXPANDERLED_GPIO_DRV_H */



