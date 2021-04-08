/**
*  @file      I2CDmaDrv_priv.h
*  @brief     This file defines the I2C macros
*  @version   v0.1
*  @author    Alex LI
*  @date      2017/7/13
*  @copyright Tymphany Ltd.
*/
#ifndef I2CDMADRV_PRIV_H
#define I2CDMADRV_PRIV_H
#include "stm32f0xx_i2c.h"
#include "I2CDmaDrv.Config"

typedef struct tI2CSpeedMap
{
    uint16   speedInKHz;
    uint32   timing;
} tI2CSpeedMap;

static void I2CDmaDrv_LowLevelInit(cI2CDmaDrv * me);
static eTpRet I2CDmaDrv_StartDmaWrite(I2C_TypeDef* I2Cx, uint32_t ReloadEndMode);
static eTpRet I2CDmaDrv_StartDmaRead(I2C_TypeDef* I2Cx, uint32_t ReloadEndMode);
static uint8 I2CDmaDrv_SwitchChannel(cI2CDmaDrv * const me, I2C_TypeDef* I2Cx);
static void timeoutCallback(void *pCbPara);
#endif
