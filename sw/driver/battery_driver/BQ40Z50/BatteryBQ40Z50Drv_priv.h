/**
*  @file      BatteryBQ40Z50Drv_priv.h
*  @brief     Private header file for  BatteryBQ40Z50Drv
*  @version   v0.1
*  @author    Alex.Li
*  @date      2017/8/28
*  @copyright Tymphany Ltd.
*/
#ifndef BATTERYBQ40Z50DRV_PRIV_H
#define BATTERYBQ40Z50DRV_PRIV_H

#include "BatteryBQ40Z50Drv.h"                                                                  


//Temperature() will get Kelvins so we need to tranform it to Celsius
#define BQ40Z50_KELVINS_2_CELSIUS(x)     ((x)-2731)   
static eTpRet BatteryDrv_I2cWrite_BQ40Z50(cBatteryDrv_BQ40Z50 * me, uint8 bytes, uint8 *pData);
static eTpRet BatteryDrv_I2cRead_BQ40Z50(cBatteryDrv_BQ40Z50 * me, uint32 regAddr, uint16 bytes, uint8 *pData);

#endif

