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
 * @file        GpioDrv.h
 * @brief       The GPIO interfaces and implementation
 * @author      Bob.Xu 
 * @date        2014-03-10
 * @copyright   Tymphany Ltd.
 */

#ifndef GPIODRV_H
#define GPIODRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "deviceTypes.h"


typedef enum
{
  ExtiIntTri_Rising,
  ExtiIntTri_Falling,
  ExtiIntTri_Rising_Falling,
  ExtiIntTri_MAX
}tExtiIntTrigger;


CLASS(cGpioDrv)
    /* private data */
    const tGPIODevice *gpioConfig;
    bool isCreated;
METHODS
    /* public functions */
void GpioDrv_Ctor(cGpioDrv *me, const tGPIODevice *pConfig);
void GpioDrv_Xtor(cGpioDrv *me);

/**
* This function is only available for the pins which has bidirection feature.
* If you passed a GPIO ID which doesn't have bidirection feature or the
* GPIO Id doesn't belong to the GPIO object,  this funciton will do nothing
* and return a minus value
* project
* @param[in]    me              GPIO object
* @param[in]    gpioId          GPIO ID
*/
void GpioDrv_SetDigitalIn(cGpioDrv *me, eGPIOId gpioId);

/**
* This function is only available for the pins which has bidirection feature.
* If you passed a GPIO ID which doesn't have bidirection feature or the
* GPIO Id doesn't belong to the GPIO object,  this funciton will do nothing
* and return a minus value
* project
* @param[in]    me              GPIO object
* @param[in]    gpioId          GPIO ID
*/
void GpioDrv_SetDigitalOut(cGpioDrv *me, eGPIOId gpioId);

/**
* This function is only available for the pins which has bidirection feature.
* If you passed a GPIO ID which doesn't have bidirection feature or the
* GPIO Id doesn't belong to the GPIO object,  this funciton will do nothing
* and return a minus value
* project
* @param[in]    me              GPIO object
* @param[in]    gpioId          GPIO ID
*/
void GpioDrv_SetAnalogIn(cGpioDrv *me, eGPIOId gpioId);

/**
* This function is only available for the pins which has bidirection feature.
* If you passed a GPIO ID which doesn't have bidirection feature or the
* GPIO Id doesn't belong to the GPIO object,  this funciton will do nothing
* and return a minus value
* project
* @param[in]    me              GPIO object
* @param[in]    gpioId          GPIO ID
*/
void GpioDrv_SetAnalogOut(cGpioDrv *me, eGPIOId gpioId);

/**
* If you passed a GPIO ID doesn't belong to the GPIO object,
* this funciton will do nothing and return a minus value
* project
* @param[in]    me              GPIO object
* @param[in]    gpioId          GPIO ID
*/
void GpioDrv_SetBit(cGpioDrv *me, eGPIOId gpioId);

/**
* If you passed a GPIO ID doesn't belong to the GPIO object,
* this funciton will do nothing and return a minus value
* project
* @param[in]    me              GPIO object
* @param[in]    gpioId          GPIO ID
*/
int8 GpioDrv_ReadBit(cGpioDrv *me, eGPIOId gpioId);

/**
* @brief        check if the gpio is set
* @param[in]    me              GPIO object
* @param[in]    gpioId          GPIO ID
*/
bool GpioDrv_isBitSet(cGpioDrv *me, eGPIOId gpioId);

/**
* If you passed a GPIO ID doesn't belong to the GPIO object,
* this funciton will do nothing and return a minus value
* project
* @param[in]    me              GPIO object
* @param[in]    gpioId          GPIO ID
*/
void GpioDrv_ClearBit(cGpioDrv *me, eGPIOId gpioId);

/**
* If you passed a GPIO ID doesn't belong to the GPIO object,
* this funciton will do nothing and return a minus value
* project
* @param[in]    me              GPIO object
* @param[in]    gpioId          GPIO ID
*/
void GpioDrv_ToggleBit(cGpioDrv * me, eGPIOId gpioId);

/**
* Enable EXTI Interrupt for GPIO pin.
* On STM32F0xx, trigger the following interupt
*    void EXTI0_1_IRQHandler(void)
*    void EXTI2_3_IRQHandler(void) 
*    void EXTI4_15_IRQHandler(void)
*
* Note type is better to set a EXTITrigger_TypeDef, but GpioDrv.h should not include
* either stm32f0xx.h oro stm32f10x.h, thus set type to uint8
*
* @param[in]    me              GPIO object
* @param[in]    gpioId          GPIO ID
* @param[in]    type            Interrupt trigger type
*/


void GpioDrv_EnableExtInterrupt(cGpioDrv *me, eGPIOId gpioId, tExtiIntTrigger type);

void GpioDrv_DisableExtInterrupt(cGpioDrv *me, eGPIOId gpioId);


void GpioDrv_EnableIntOfWakeUpKeyPin();
void GpioDrv_AssertWakeUpKeyEnable();
#ifdef HW_GPIO_DEBUG
#define INVALID_GPIO_READING    0x99
void GpioDrv_DirectPinAccessSet(eIoPort ioPort, eIoBit ioBit, eGPIOInitAttr act);
uint8 GpioDrv_DirectPinAccessRead(eIoPort ioPort, eIoBit ioBit, eGPIODrection direction);
#endif
END_CLASS




#ifdef __cplusplus
}
#endif

#endif /* GPIODRV_H */

