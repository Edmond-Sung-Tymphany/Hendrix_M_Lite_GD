#ifndef CS42528_DRV
#define CS42528_DRV
#include "I2CDrv.h"


typedef struct
{
    void (*initSectionFunc)(void* me);         // function point to a specific initialization section
    uint16 delaytime;   // time duration to next intialization section in ms
}tDspInitSection;



typedef struct cCS42528Drv
{
    bool            isCreated:1;
    tDspInitSection const *pInitTable;
    uint8           sectionSize;
    uint8           initPhase;
    cI2CDrv         *i2cObj;  /* A i2c obj should be ctored before ctor dspDrv*/
    uint8           deviceAddr;
    bool            i2cEnable; /* When DSP cable insert, disable I2C */
}cCS42528Drv;

#endif
