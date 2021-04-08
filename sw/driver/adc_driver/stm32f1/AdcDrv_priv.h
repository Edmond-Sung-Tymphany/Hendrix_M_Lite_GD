/**
 *  @file      adcDrv_priv.h
 *  @brief     This file presents low level configuration to stm ADC driver
 *  @author    Bob.Xu
 *  @date      23-May-2013
 *  @copyright Tymphany Ltd.
 */
#ifndef ADC_STM_DRIVER_H
#define ADC_STM_DRIVER_H
#include "stm32f10x.h"
#include "AdcDrv.h"
/* This structure remap the genaric IO to MCU specific IO */
typedef struct tStmIoInfo
{
    GPIO_TypeDef        *gpioPort;         /* IO port */
    uint32               rccAhbPeriph;     /* port clock ctr bit */
}tStmIoInfo;
#define GPIO_INVALIDD           0
#define GPIO_AHPPreiph_Invalid  0
#define ADC_MIN_VALUE           0
#define ADC_MAX_VALUE           4095
#define ADC_INITIAL_VALUE       -1

#endif /* End of ADC_STM_DRIVER_H */
