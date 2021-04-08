/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  ADC PCM1960 driver
                  -------------------------

                  SW Module Document




@file        Pcm1690Drv.h
@brief       This file implements the drivers for adau1451
@author      Alexey
@date        2017
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/
#ifndef PCM1960DRV_H
#define PCM1960DRV_H
#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "I2CDrv.h"
#include "GpioDrv.h"

typedef struct
{
    void (*initSectionFunc)(void* me);         // function point to a specific initialization section
    uint16 delaytime;   // time duration to next intialization section in ms
}tDacInitSection;

CLASS(cPcm1690Drv)
    bool            isCreated:1;
    tDacInitSection const *pInitTable;
    uint8           sectionSize;
    uint8           initPhase;
    cI2CDrv         *pI2cObj;  /* A i2c obj should be ctored before ctor dspDrv*/
    cGpioDrv        gpioObj; /* A GPIO obj should be ctored before ctor dspDrv*/
    bool            i2cEnable; /* When DSP cable insert, disable I2C */
METHODS

/**
 * Construct the DSP driver instance.
 * @param      me - instance of the driver
 */
void Pcm1690Drv_Ctor(cPcm1690Drv* me, cI2CDrv *pI2cObj);

uint16 Pcm1690Drv_Init(cPcm1690Drv* me);

/**
 * Destruct the DSP driver instance.
 * @param      me - instance of the driver
 */
void Pcm1690Drv_Xtor(cPcm1690Drv* me);

END_CLASS

#endif /* PCM1960DRV_H */


