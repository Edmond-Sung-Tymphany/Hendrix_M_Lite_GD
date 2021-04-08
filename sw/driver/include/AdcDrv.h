/**
 *  @file       adcDrv.h
 *  @brief      This file presents the interface to ADC driver, user has to
 *              fowllow three steps to use this interface:
 *              Step1: check if the adc object has been created by exam 
 *                     isCreated flag,if the answer is no, call ADCDrv_Ctor()
 *                     to create the adc object.
 *              step2: Call ADCDrv_StartScanning() to scan adc pins
 *              step3: Call ADCDrv_GetData to read the adc value
 *  @author     Bob.Xu
 *  @date       19-Jan-2013
 *  @copyright  Tymphany Ltd.
 */

#ifndef ADCDRV_H
#define ADCDRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "attachedDevices.h"

#define ADC_DATA_IS_NOT_READY   -1

typedef enum
{
    ADC_AUTO_MODE,
    ADC_MANUAL_MODE,
    ADC_MODE_MAX,
}eADCSamplingModel;


CLASS(cADCDrv)
    const tADCDevice* ADCConfig;
    bool isCreated; 
METHODS
/* PUBLIC FUNCTION PROTOTYPES */

/**
* ADC Driver object constructor
* @param[in]    me              the ADC Driver object
* @param[in]    pConfig        configuration of the ADC instance
*/
void ADCDrv_Ctor(cADCDrv * me, const tADCDevice * pConfig);

/**
* ADC Driver object destructor
* @param[in]    me              the ADC Driver object
*/
void ADCDrv_Xtor(cADCDrv * me);

/**
* Start the ADC scanning
* @param[in]    me              the ADC Driver object
*/
void ADCDrv_StartScanning(cADCDrv * me);

/*
* Check ADC conversion status
* @param[in]    me              the ADC Driver object
*/
bool ADCDrv_IsConversionDone(cADCDrv * me);

/*
* Get ADC conversion value
* @param[in]    me              the ADC Driver object
* @param[in]    adcPin          the ADC pin id
* @param[out]   return the adc value
*/
int32 ADCDrv_GetData(cADCDrv* me,eAdcPin adcPin);

#ifdef HAS_DIRECT_ADC_ACCESS_BY_PORT_BIT
/**
* This function provide a way to read the adc raw value by pin port and bit info.
* @param[in]    ioPort       general io port
* @param[in]    ioBit        general io bit
* @param[out]   return raw adc value
*/
int32 ADCDrv_GetAdcRawDataByPortBit(eIoPort ioPort, eIoBit ioBit);
#endif

/*
* Reset ADC conversion status to default status
* @param[in]    me              the ADC Driver object
*/
void ADCDrv_ResetAdcConversionStatus();



END_CLASS

#ifdef	__cplusplus
}
#endif

#endif /* ADCDRV_H */
