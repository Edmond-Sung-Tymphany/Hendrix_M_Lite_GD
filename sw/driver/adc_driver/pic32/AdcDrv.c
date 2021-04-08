/**
 *  @file      AdcDrv.c
 *  @brief     This file implements the interface of ADC driver, hardware
 *             dependent functions such as ADC_Disable(),ADC_SetAnalogInputPins()
 *             should be redesigned based on the specific chip.
 *             you should follow the fowllowing steps to use ADC driver after you
 *             have created the ADC object
 *             Step1:ADCDrv_StartScanning();
 *             Step2:ADCDrv_IsConvertionDone()
 *             Step3:ADCDrv_GetData() based on Step2, you could ignore step2
 *             by jumping to Step3 directly, in this case, you may possiblely get
 *             the previous data instead of the latest ADC data.
 *  @author    Bob.Xu
 *  @date      9-Feb-2013
 *  @copyright Tymphany Ltd.
 */
#include "sortLib.h"
#include "trace.h"
#include "AdcDrv_priv.h"

#define ADC_INITIAL_VALUE   -1
/* private variables */
static bool conversionDone = TRUE;
static uint32 enabledPinBits = 0;
/* initial the arrray with an invalid pin id */
static eAdcPin allADCPins[NUM_OF_ALL_ENABLED_ADC_PIN] =
{
    [0 ... (NUM_OF_ALL_ENABLED_ADC_PIN-1)] = ADC_PIN_MAX
};
static int32 adcAllPinBuf[NUM_OF_ALL_ENABLED_ADC_PIN] =
{
    [0 ... (NUM_OF_ALL_ENABLED_ADC_PIN-1)] = ADC_INITIAL_VALUE
};
/* End of private variables */

/**
* Local function,Set ADC PIN
* @param[in]    pConfig        configuration of the enabled pins
*/
static void ADC_SetPins(const tADCDevice* pConfig)
{
    uint8 i;
    enabledPinBits = 0; /* reset */
    for(i = 0; i < pConfig->ADCEnabledPinNum; i++)
    {
        enabledPinBits |= (1 << pConfig->pAdcPinIoAttr[i].adcPin);
    }
    /* 
     * Configure port Pins to be used as analog inputs
     * Only set the pins which belong to pConfig
     */
    mPORTBSetPinsAnalogIn(enabledPinBits);
    /* Set analog input pins to ADC mux */
    SetChanADC10(enabledPinBits);
}

/**
* Local function,Configure ADC main control registers
* Default sampling is manual mode.
* @param[in]    pConfig        configuration of the enabled pins
*/
static void ADC_ConfigMainRegs(const tADCDevice* pConfig)
{
    ASSERT(pConfig->ADCEnabledPinNum);

    MainReg1_Config();
    /* config control register AD1CON2 */
    if(NUM_OF_ALL_ENABLED_ADC_PIN < ADC_CHANNEL_MAX)
    {
        MainReg2_Config(NUM_OF_ALL_ENABLED_ADC_PIN);
    }
    else
    {
        /**
        * More than 16 ADC pin are used, ADCDrv_GetResult() must be redesigned
        */
        ASSERT(0);
        MainReg2_Config(ADC_CHANNEL_MAX);
    }
    /* config control register AD1CON3 */
    MainReg3_Config();
    MainReg4_Config(enabledPinBits);
    Enable_AdcInterrupt();
}

/**
* ADC Driver object constructor
* @param[in]    me             the adc object
* @param[in]    pConfig        configuration of the ADC instance
*/
void ADCDrv_Ctor(cADCDrv * me, const tADCDevice * pConfig)
{
    uint8 i;
    uint8 j;
    {
        /* check user's config */
        ASSERT(me && pConfig && ((pConfig->ADCEnabledPinNum) <= ADC_PIN_MAX));
    }

        /* Copy the config to object */
        me->ADCConfig = pConfig;
        /* Hardware init*/
    {
        /* ensure the adc is disabled */
        ADC_Disable();
        ADC_SetPins(me->ADCConfig);
        ADC_ConfigMainRegs(me->ADCConfig);
    }
        /* put the pins to the allADCPins[] array */
        for(i = 0; i < me->ADCConfig->ADCEnabledPinNum; i++)
        {
            for( j = 0; j < NUM_OF_ALL_ENABLED_ADC_PIN; j++)
            {
                if( ADC_PIN_MAX == allADCPins[j])
                {
                    allADCPins[j] = me->ADCConfig->pAdcPinIoAttr[i].adcPin;
                    break;
                }
            }
        }

        /**
         * sort the pin array, this actually maped the pin to channel
         */
        tpQsort((int32*)(&(allADCPins)), 0, (ArraySize(allADCPins)-1));

        me->isCreated = TRUE;
        /*
         * make sure that there is some data available in adcAllPinBuf if user
         * disobey the rules of using ADC driver.
         */
        ADCDrv_StartScanning(me);
}

/**
* ADC Driver object destructor
* @param[in]    me              the ADC Driver object
*/
void ADCDrv_Xtor(cADCDrv * me)
{
    ASSERT(me);
    me->isCreated = FALSE;
    ADC_Disable();
}

/**
* Start the ADC scanning
* @param[in]    me              the ADC Driver object
*/
void ADCDrv_StartScanning(cADCDrv * me)
{
    ASSERT(me && (me->isCreated));

    if(conversionDone)
    {
        conversionDone = FALSE;
        ADC_Enable();
        ADC_StartScanning();
    }
}

void ADCDrv_ResetAdcConversionStatus(cADCDrv * me)
{
    ASSERT(me && (me->isCreated));
    conversionDone = TRUE;
}

/**
* Check ADC conversion status
* @param[in]    me              the ADC Driver object
*/
bool ADCDrv_IsConvertionDone(cADCDrv * me)
{
    ASSERT(me && (me->isCreated));
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
    uint8 i;
    int32 returnValue = ADC_DATA_IS_NOT_READY;
    ASSERT(me && (me->isCreated) && (adcPin < ADC_PIN_MAX));

    /**
     *  pin belongs to this object
     *  otherwise it will return an minus value
     */
    for(i = 0; i < me->ADCConfig->ADCEnabledPinNum; i++)
    {
        if(adcPin == me->ADCConfig->pAdcPinIoAttr[i].adcPin)
        {
            break;
        }
        if(i == (me->ADCConfig->ADCEnabledPinNum - 1))
        {
            /* 
             * The adcPin is not belongs to this object
             * There are something wrong with your code
             */
            ASSERT(FALSE);
        }
    }

    for(i = 0; i < NUM_OF_ALL_ENABLED_ADC_PIN; i++)
    {
        if(adcPin == allADCPins[i])
        {
            break;
        }
    }
    /**
     * read the converted ADC value from the adc buffer.
     */
    if(adcAllPinBuf[i] != ADC_INITIAL_VALUE)
    {
        returnValue = adcAllPinBuf[i];
    }
    ASSERT((returnValue >= ADC_MIN_VALUE && returnValue <= ADC_MAX_VALUE));
    return returnValue;
}


void __ISR (_ADC_VECTOR, IPL2) __interrupt_AD(void)
{
    uint8 i;
    ADC_Disable();
    conversionDone = TRUE;
    for(i =0 ; i < NUM_OF_ALL_ENABLED_ADC_PIN; i++)
    {
        adcAllPinBuf[i] = ADC_ReadADCBuffer(i);
    }
}
