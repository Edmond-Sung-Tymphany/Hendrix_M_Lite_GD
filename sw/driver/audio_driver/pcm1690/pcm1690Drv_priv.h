/**
 * @file        AdcDrv_pcm1862_priv.h
 * @brief       The audio DAC driver: TI PCM1690
 * @author      Gavin Lee
 * @date        2017-8-10
 * @copyright   Tymphany Ltd.
 */
#ifndef DACDRV_PCM1890_PRIVATE_H
#define DACDRV_PCM1890_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pcm1690Drv.h"
#include "setting_id.h"

/***********************************************************
 * Feature
 ***********************************************************/

  
  
/***********************************************************
 * Definition
 ***********************************************************/  




  
/***********************************************************
 * Function
 ***********************************************************/  
static void Pcm1690Drv_I2cWrite(cI2CDrv * i2c, uint32 regAddr, const uint8 value);
static void Pcm1690Drv_I2cRead(cI2CDrv * i2c,uint32 regAddr, uint8 *pValue);
static void Pcm1690Drv_Init_RstLow(void *p);
static void Pcm1690Drv_Init_RstHigh(void *p);
static void Pcm1690Drv_Init_Section(void *p);

#ifdef __cplusplus
}
#endif

#endif /* DACDRV_PCM1890_PRIVATE_H */