/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  dsp 1451 driver
                  -------------------------

                  SW Module Document




@file        dsp_adau1451_driver.c
@brief       This file implements the drivers for adau1451
@author      Daniel Qin
@date        2015-12-15
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/
#include <stdio.h>
#include "trace.h"
#include "cplus.h"
#include "commonTypes.h"
#include "I2CDrv.h"
#include "Adau1962Drv.h"
#include "setting_id.h"

/***********************************************************/
/********************* Definition **************************/
/***********************************************************/
#ifndef NULL
#define NULL          (0)
#endif

/*#ifndef DSPDRV_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif*/

#define ON_STATE      (1)
#define OFF_STATE     (0)

/***********************************************************/
/****************** Global Variable ************************/
/***********************************************************/

/***********************************************************/
/****************** PUBLIC FUNCTION ************************/
/***********************************************************/
/**
 * Construct the DSP driver instance.
 * @param      me - instance of the driver
 */

static void Adau1962Drv_I2cWrite(cI2CDrv * i2c, uint16 bytes, const uint8 *data);
static void Adau1962Drv_I2cRead(cI2CDrv * i2c,uint32 regAddr,uint16 bytes,const uint8 * data);
static void Adau1962Drv_InitI2c(void *p);
static void Adau1962Drv_InitSection(void *p);

static tDacInitSection DacInitSection[] =
{
    {&Adau1962Drv_InitI2c, 100},
    {&Adau1962Drv_InitSection, 100},
};

void Adau1962Drv_Ctor(cAdau1962Drv* me, cI2CDrv *pI2cObj)
{
    me->i2cObj        = pI2cObj;
    me->pInitTable    = DacInitSection;
    me->sectionSize   = ArraySize(DacInitSection);
    me->initPhase     = 0;
    me->isCreated     = TRUE;
    me->i2cEnable     = TRUE;
}

static void Adau1962Drv_InitI2c(void *p)
{
    TP_PRINTF("Adau1962Drv_InitI2c\r\n");

    //Initialize I2C
    cAdau1962Drv* me = (cAdau1962Drv*)p;
    const tDevice * pDevice = NULL;
    pDevice = getDevicebyIdAndType(DAC_DEV_ID, I2C_DEV_TYPE, NULL);
    ASSERT(pDevice);
    I2CDrv_Ctor(me->i2cObj,(tI2CDevice*)pDevice);
    me->i2cObj->registeredUser++;

    TP_PRINTF("Adau1962Drv_InitI2c finish\r\n");
}

static void Adau1962Drv_InitSection(void *p)
{
    TP_PRINTF("Adau1962Drv_InitSection\n\r");

    cAdau1962Drv* me = (cAdau1962Drv*)p;

    uint8 data[2] = {0};

    data[0] = 0x0;
    data[1] = 0x71;
    Adau1962Drv_I2cWrite(me->i2cObj, 2, data);

    data[0] = 0x1;
    data[1] = 0x36;
    Adau1962Drv_I2cWrite(me->i2cObj, 2, data);

    data[0] = 0x6;
    data[1] = 24;
    Adau1962Drv_I2cWrite(me->i2cObj, 2, data);

    data[0] = 0x7;
    data[1] = 64;
    Adau1962Drv_I2cWrite(me->i2cObj, 2, data);

}

uint16 Adau1962Drv_Init(cAdau1962Drv* me)
{
    uint16 delaytime;

    if (me->initPhase == me->sectionSize)
    {
        me->initPhase = 0;
        return 0;
    }

    ASSERT(me && me->pInitTable && me->pInitTable[me->initPhase].initSectionFunc);
    me->pInitTable[me->initPhase].initSectionFunc(me);
    delaytime = me->pInitTable[me->initPhase].delaytime;
    me->initPhase++;
    return (delaytime);
}

static void Adau1962Drv_I2cWrite(cI2CDrv * i2c, uint16 bytes, const uint8 *data)
{
    tI2CMsg i2cMsg =
    {
        .devAddr = i2c->pConfig->devAddress,
        .regAddr = NULL,
        .length  = bytes,
        .pMsg    = (uint8*)data
    };
    bool ret= I2CDrv_MasterWrite(i2c, &i2cMsg);

    if (ret != TP_SUCCESS)
    {
        ASSERT(ret==TP_SUCCESS);
    }
}

static void Adau1962Drv_I2cRead(cI2CDrv * i2c, uint32 regAddr, uint16 bytes, const uint8 *data)
{
    tI2CMsg i2cMsg =
    {
        .devAddr = i2c->pConfig->devAddress,
        .regAddr = regAddr,
        .length  = bytes,
        .pMsg    = (uint8*)data
    };
    bool ret= I2CDrv_MasterRead(i2c, &i2cMsg);

    if (ret != TP_SUCCESS)
    {
        ASSERT(ret==TP_SUCCESS);
    }
}
