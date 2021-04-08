#ifndef __CS48L11_DRIVER_H__
#define __CS48L11_DRIVER_H__

typedef enum tagUldID
{
    ULD_ID_OS,
    ULD_ID_AC3,
    ULD_ID_CROSSBAR,
    ULD_ID_APP,
    ULD_ID_PPM,
    ULD_ID_MAX
}UldID_t;

/**
 * Construct the DSP driver instance.
 * @return : none
 */
void Cs48L11Drv_Ctor(void);

/**
 * Exit & clean up the driver.
 */
void Cs48L11Drv_Xtor(void);

/**
 * Load the specify ULD file to DSP
 */
bool CS48L11Drv_LoadULD(UldID_t id);

/**
 * Write the dsp program/parameter data.
 */
bool Cs48L11Drv_Init(void);


#endif  // __CS48L11_DRIVER_H__
