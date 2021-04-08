/**
 * @file        GpioDrv_priv.h
 * @brief       The GPIO interfaces and implementation
 * @author      Bob.Xu 
 * @date        2014-03-10
 * @copyright   Tymphany Ltd.
 */
#ifndef GPIO_DRIVER_PRIVATE_H
#define GPIO_DRIVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"
#include "GpioDrv.h"

#define GPIO_INVALIDD           0
#define GPIO_AHPPreiph_Invalid  0
/* This structure remap the genaric IO to MCU specific IO */
typedef struct tStmIOInfo
{
    GPIO_TypeDef        *pMCUgpioPort;        /* IO port */
    uint32              rccAhbPeriph;     /* port clock ctr bit */
}tStmIOInfo;
/* private functions / data */

#ifdef __cplusplus
}
#endif

#endif /* GPIO_DRIVER_PRIVATE_H */