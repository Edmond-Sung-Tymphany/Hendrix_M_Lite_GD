#ifndef DSP_TLV320AIC3254_CUSTOMER_DRIVER_H
#define DSP_TLV320AIC3254_CUSTOMER_DRIVER_H








/* DSP Volume Coefs */
typedef struct
{
    uint8 address;
    uint8 param0;
    uint8 param1;
    uint8 param2;
} tDspVol;







/**
 * Get input signal detection, return 1 is there is signal, otherwise return 0
 * it is referring to overall signal existance in the DSP.
*/

bool DSPDrv_signalDetect(cDSPDrv *me);




#endif
