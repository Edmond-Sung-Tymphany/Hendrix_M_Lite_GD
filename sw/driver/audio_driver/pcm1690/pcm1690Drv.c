/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  ADC PCM1690 driver
                  -------------------------

                  SW Module Document




@file        adau1690Drv.c
@brief       This file implements the drivers for ADAU1690 ADC
@author      Gavin Lee
@date        2017-8-3
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/
#include <stdio.h>
#include "trace.h"
#include "cplus.h"
#include "commonTypes.h"
#include "I2CDrv.h"
#include "Pcm1690Drv.h"
#include "./pcm1690Drv_priv.h"
#include "setting_id.h"
#include "attachedDevices.h"


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
static tDacInitSection DacInitSection[] =
{
    {&Pcm1690Drv_Init_RstLow,   50},
    {&Pcm1690Drv_Init_RstHigh,  50},
    {&Pcm1690Drv_Init_Section,  50},
};


void Pcm1690Drv_Ctor(cPcm1690Drv* me, cI2CDrv *pI2cObj)
{
    me->pI2cObj       = pI2cObj;
    me->pInitTable    = DacInitSection;
    me->sectionSize   = ArraySize(DacInitSection);
    me->initPhase     = 0;
    me->isCreated     = TRUE;
    me->i2cEnable     = TRUE;
}

void Pcm1690Drv_Xtor(cPcm1690Drv* me)
{
  //TODO:
  //  xtor I2C and GPIO
}


static void Pcm1690Drv_Init_RstLow(void *p)
{
    TP_PRINTF("Pcm1690Drv_Init_RstLow\r\n");
    cPcm1690Drv* me = (cPcm1690Drv*)p;
    const tDevice * pDevice = NULL;

    //Initialize I2C
    pDevice = getDevicebyIdAndType(AUDIO_DAC_DEV_ID, I2C_DEV_TYPE, NULL);
    ASSERT(pDevice);
    I2CDrv_Ctor(me->pI2cObj,(tI2CDevice*)pDevice);
    me->pI2cObj->registeredUser++;

    //Initialize GPIO
    pDevice = getDevicebyIdAndType(AUDIO_DAC_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(pDevice);
    GpioDrv_Ctor(&(me->gpioObj), (tGPIODevice*)pDevice);

    //Pull low RST pin
    GpioDrv_ClearBit(&me->gpioObj, GPIO_OUT_AUDIO_DAC_RST_N);
}


static void Pcm1690Drv_Init_RstHigh(void *p)
{
    TP_PRINTF("Pcm1690Drv_Init_RstHigh\r\n");
    cPcm1690Drv* me= (cPcm1690Drv*)p;
    GpioDrv_SetBit(&(me->gpioObj), GPIO_OUT_AUDIO_DAC_RST_N);
}


static void Pcm1690Drv_Init_Section(void *p)
{
    TP_PRINTF("Pcm1690Drv_Init_Section\r\n");
    cPcm1690Drv* me= (cPcm1690Drv*)p;
    Pcm1690Drv_I2cWrite(me->pI2cObj, /*regAddr:*/0x41, /*value:*/0x06);
    
//      //debug
//      Pcm1690Drv_I2cRead(me->pI2cObj, /*regAddr:*/0x41, &value);
//      TP_PRINTF("\r\n");
}


uint16 Pcm1690Drv_Init(cPcm1690Drv* me)
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


static void Pcm1690Drv_I2cWrite(cI2CDrv * i2c, uint32 regAddr, const uint8 value)
{
    ASSERT(i2c->pConfig->regAddrLen==REG_LEN_8BITS);    
    uint8 data[2]= {regAddr, value};
    tI2CMsg i2cMsg =
    {
        .devAddr = i2c->pConfig->devAddress,
        .regAddr = NULL,
        .length  = 2,
        .pMsg    = (uint8*)data
    };
    bool ret= I2CDrv_MasterWrite(i2c, &i2cMsg);
    TP_PRINTF("Pcm1690Drv_I2cWrite: REG[0x%02x] set to 0x%02x\r\n", regAddr, value);
    ASSERT(ret==TP_SUCCESS);
}


static void Pcm1690Drv_I2cRead(cI2CDrv * i2c, uint32 regAddr, uint8 *pValue)
{
    tI2CMsg i2cMsg =
    {
        .devAddr = i2c->pConfig->devAddress,
        .regAddr = regAddr,
        .length  = 1,
        .pMsg    = (uint8*)pValue
    };
    bool ret= I2CDrv_MasterRead(i2c, &i2cMsg);
    TP_PRINTF("Pcm1690Drv_I2cRead: REG[0x%02x] read as 0x%02x\r\n", regAddr, *pValue);
    ASSERT(ret==TP_SUCCESS);
}
