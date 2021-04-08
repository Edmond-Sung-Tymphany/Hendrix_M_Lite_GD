#ifndef ADAU1962DRV_H
#define ADAU1962DRV_H
#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "I2CDrv.h"

typedef struct
{
    void (*initSectionFunc)(void* me);         // function point to a specific initialization section
    uint16 delaytime;   // time duration to next intialization section in ms
}tDacInitSection;

CLASS(cAdau1962Drv)
    bool            isCreated:1;
    tDacInitSection const *pInitTable;
    uint8           sectionSize;
    uint8           initPhase;
    cI2CDrv         *i2cObj;  /* A i2c obj should be ctored before ctor dspDrv*/
    bool            i2cEnable; /* When DSP cable insert, disable I2C */
METHODS

/**
 * Construct the DSP driver instance.
 * @param      me - instance of the driver
 */
void Adau1962Drv_Ctor(cAdau1962Drv* me, cI2CDrv *pI2cObj);
uint16 Adau1962Drv_Init(cAdau1962Drv* me);

/**
 * Destruct the DSP driver instance.
 * @param      me - instance of the driver
 */
void Adau1962Drv_Xtor(cAdau1962Drv* me);

END_CLASS

#endif /* ADAU1962DRV_H */


