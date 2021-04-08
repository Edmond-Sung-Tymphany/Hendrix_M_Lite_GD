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
 *  @file      I2CDrv_priv.h
 *  @brief     This file defines the I2C macros and defines for the PIC32 family
 *  @author    Dmitry Abdulov
 *  @date      28-Aug-2013
 *  @copyright Tymphany Ltd.
 */

#ifndef I2CDRV_PRIV_H
#define	I2CDRV_PRIV_H

typedef struct tI2CSpeedMap
{
    uint16   speedInKHz;
    uint32   timing;
}tI2CSpeedMap;

//#define I2C_TIMING      (0x0010020A)      //400 KHz    
//#define I2C_TIMING      (0x100098F3)   //100 KHz ?
//#define I2C_TIMING        (0x00731012) 

static void I2CDrv_LowLevelInit(cI2CDrv * me);

static void I2CDrv_RecoverFromBusy(I2C_TypeDef* I2Cx);

#endif	/* I2CDRV_PRIV_H */

