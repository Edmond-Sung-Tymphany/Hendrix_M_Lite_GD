/**
 *  @file      AdcDrv.c
 *  @brief     This file implements the interface of ADC driver
 *  @author    
 *  @date      
 *  @copyright Tymphany Ltd.
 */

#include "trace.h"
#include "AdcDrv_priv.h"
#include "sortLib.h"

/* Private variables */
static bool conversionDone = TRUE;
/* End of private variables */

/**
* ADC Driver object constructor
* @param[in]    me             the adc object
* @param[in]    pConfig        configuration of the ADC instance
*/
void ADCDrv_Ctor(cADCDrv * me, const tADCDevice * pConfig)
{
    ASSERT(me);
    ASSERT(pConfig);
    ASSERT(((pConfig->ADCEnabledPinNum) <= ADC_PIN_MAX));
}

/**
* ADC Driver object destructor
* @param[in]    me              the ADC Driver object
*/
void ADCDrv_Xtor(cADCDrv * me)
{
    ASSERT(me);
}

/**
* Start the ADC scanning
* @param[in]    me              the ADC Driver object
*/
void ADCDrv_StartScanning(cADCDrv * me)
{
    ASSERT(me);

    if(conversionDone)
    {
        conversionDone = FALSE;
    }
}

void ADCDrv_ResetAdcConversionStatus(cADCDrv * me)
{
    ASSERT(me);
    conversionDone = TRUE;
}

/**
* Check ADC conversion status
* @param[in]    me              the ADC Driver object
*/
bool ADCDrv_IsConvertionDone(cADCDrv * me)
{
    ASSERT(me);
    return conversionDone;
}

/**
* The parameter me doesn't have the buffer
* to store the values of pins which belong to p(me), all adc value is
* stored in adcAllPinBuf[],This function controls the object access to
* adcAllPinBuf[]
* @param[in]    p              the ADC Driver object
* @param[in]    channel        ADC channel
* @param[out]   return the adc value
*/
int32 ADCDrv_GetData(cADCDrv* me,eAdcPin adcPin)
{
    int32 returnValue = ADC_DATA_IS_NOT_READY;

    ASSERT(me);
    ASSERT((adcPin < ADC_PIN_MAX));

    return returnValue;
}

void ADC1_COMP_IRQHandler(void)
{

}
