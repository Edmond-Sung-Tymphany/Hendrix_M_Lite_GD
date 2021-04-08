/**
 * @file        KeyDrv_priv.h
 * @brief       The key driver interfaces and implementation 
 * @author      Bob.Xu 
 * @date        2014-02-13
 * @copyright   Tymphany Ltd.
 */
#ifndef KEYDRV_PRIV_H
#define KEYDRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    GPIO_HIGH,
    GPIO_LOW,
}eGPIOEvent;

#include "KeyDrv.h"    
/* private functions / data */

#ifdef __cplusplus
}
#endif

#endif /* KEYDRV_PRIV_H */