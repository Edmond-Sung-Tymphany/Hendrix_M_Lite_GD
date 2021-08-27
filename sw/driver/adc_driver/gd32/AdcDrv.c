/**
 *  @file      AdcDrv.c
 *  @brief     This file implements the interface of ADC driver, hardware
 *             dependent functions such as ADC_Disable(),ADC_SetAnalogInputPins()
 *             should be redesigned based on the specific chip.
 *  @author    Bob.Xu
 *  @date      9-Feb-2013
 *  @copyright Tymphany Ltd.
 */

#include "trace.h"
#include "AdcDrv_priv.h"
#include "sortLib.h"
#include "bsp.h"
#ifndef ADCDRV_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
/* Private variables */
volatile static bool conversionDone = TRUE;
/* The first element will be init with ADC_PIN_MAX */
static eAdcPin allADCPins[NUM_OF_ALL_ENABLED_ADC_PIN] = {ADC_PIN_MAX};
static BOOL allADCPinsInitiated = FALSE;
static int16 adcAllPinBuf[NUM_OF_ALL_ENABLED_ADC_PIN] = {ADC_INITIAL_VALUE};
volatile static uint8 channelCounter = 0;
static uint8 currRegisteredCh = 0; /* Current registerred adc channel */
static uint8 numOfRegisteredAdcObj = 0;

DMA_InitTypeDef DMA_InitStructure;
ADC_InitTypeDef     ADC_InitStructure;
GPIO_InitTypeDef    GPIO_InitStructure;

/* End of private variables */

static const tStmIoInfo stmIOInfo[] =
{
    {GPIOA,         RCC_AHBPeriph_GPIOA},
    {GPIOB,         RCC_AHBPeriph_GPIOB},
    {GPIOC,         RCC_AHBPeriph_GPIOC},
    {GPIOD,         RCC_AHBPeriph_GPIOD},
    {GPIO_INVALIDD, GPIO_AHPPreiph_Invalid},  //port E is not available
    {GPIOF,         RCC_AHBPeriph_GPIOF},
};

#ifdef HAS_DIRECT_ADC_ACCESS_BY_PORT_BIT
static int8 indexOfTail = -1;
//This array is initialized by Ctor
static tAdcPinInfo enabledAdcPinInfo[NUM_OF_ALL_ENABLED_ADC_PIN];
#endif



/**
* ADC Driver object constructor
* @param[in]    me             the adc object
* @param[in]    pConfig        configuration of the ADC instance
*/
void ADCDrv_Ctor(cADCDrv * me, const tADCDevice * pConfig)
{

uint8 i; /* i is the index of the adc pin set which owns by this adc object*/
    int8 j; /* j is the index of all enabled adc pin set*/


    ASSERT(me && pConfig && ((pConfig->ADCEnabledPinNum) <= ADC_PIN_MAX));
    //ADC_InitTypeDef     ADC_InitStructure;
    //GPIO_InitTypeDef    GPIO_InitStructure;
    NVIC_InitTypeDef    NVIC_InitStructure;
    if(me->isCreated)
    {
        return;
    }
    else if(currRegisteredCh == NUM_OF_ALL_ENABLED_ADC_PIN)
    {
        //This is for the case if one specific adc obj is Ctor and Xtor serveal times.
        numOfRegisteredAdcObj++;
        me->isCreated = TRUE;
        return;
    }
    
    /* Before any operation, stop ADC Interrupt first 
     * Otherwise interrupt will change variable, and cause race condition
     */
 
    ADC_SoftwareStartConvCmd(ADC1, DISABLE); 
	
    numOfRegisteredAdcObj++;
    if(!allADCPinsInitiated)
    {
        allADCPinsInitiated = TRUE;
        /* 
         * The default value of allADCPins must be set to ADC_PIN_MAX and only once.
         * if not, the compiler will set it to defualt value which is ADC_PIN0,
         * This will introduce a bug in special senario. For example: adcObj1 owns ADC_PIN10,ADC_PIN11
         * adcObj2 has ADC_PIN8 and ADC_PIN9. Supose adcObjet1 is ctored first,
         * then we have allADCPins = {ADC_PIN10,ADC_PIN11,ADC_PIN0,ADC_PIN0}
         * after qsort, it will be allADCPins = {ADC_PIN0,ADC_PIN0,ADC_PIN10,ADC_PIN11}
         * adc scanning value will be assigned to ADC_PIN0 which doesn't belong to any adcobj
        */
        for(i = 0; i < NUM_OF_ALL_ENABLED_ADC_PIN; i++)
        {
            allADCPins[i] = ADC_PIN_MAX;
        }
    }

   
    /* Copy the config to object */
    me->ADCConfig = pConfig;

    /* Make sure the adc is disabled */
    ADC_Cmd(ADC1, DISABLE);

    /* reset the flags */
    channelCounter = 0;
    conversionDone = TRUE;
/* ADC1 Periph clock enable */
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
  /* ADCCLK = PCLK2/2 */
  RCC_ADCCLKConfig(RCC_PCLK2_Div2); 
#else
  /* ADCCLK = PCLK2/4 */
  RCC_ADCCLKConfig(RCC_ADCCLK_PCLK2_Div6); 
#endif
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Enable ADC1 and GPIOC clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    for(i = 0; i < me->ADCConfig->ADCEnabledPinNum; i++)
    {

#ifdef HAS_DIRECT_ADC_ACCESS_BY_PORT_BIT
      indexOfTail++;
      ASSERT(indexOfTail < NUM_OF_ALL_ENABLED_ADC_PIN);
      enabledAdcPinInfo[indexOfTail].adcPin = me->ADCConfig->pAdcPinIoAttr[i].adcPin;
      enabledAdcPinInfo[indexOfTail].ioPort = me->ADCConfig->pAdcPinIoAttr[i].ioPort;
      enabledAdcPinInfo[indexOfTail].gpioBit = me->ADCConfig->pAdcPinIoAttr[i].gpioBit;
#endif

      RCC_AHBPeriphClockCmd(stmIOInfo[(me->ADCConfig->pAdcPinIoAttr[i].ioPort)].rccAhbPeriph, ENABLE);
      GPIO_InitStructure.GPIO_Pin = (1 << (me->ADCConfig->pAdcPinIoAttr[i].gpioBit));
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
      GPIO_Init(stmIOInfo[(me->ADCConfig->pAdcPinIoAttr[i].ioPort)].gpioPort, &GPIO_InitStructure);
      
      ADC_RegularChannelConfig(ADC1, (me->ADCConfig->pAdcPinIoAttr[i].adcChannel), currRegisteredCh+1, ADC_SampleTime_239Cycles5);

      currRegisteredCh ++;
      ASSERT(currRegisteredCh <= NUM_OF_ALL_ENABLED_ADC_PIN);
    }

    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adcAllPinBuf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = currRegisteredCh;//me->ADCConfig->ADCEnabledPinNum;//1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);
    
    
    /* Initialize ADC structure */
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = currRegisteredCh;
    ADC_Init(ADC1, &ADC_InitStructure);   
   
    ADC_Cmd(ADC1, ENABLE); 
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    /* put the pins to the allADCPins[] array, for resort purpose */
    for(i = 0; i < me->ADCConfig->ADCEnabledPinNum; i++)
    {
        for( j = 0; j < NUM_OF_ALL_ENABLED_ADC_PIN; j++)
        {
            if( ADC_PIN_MAX == allADCPins[j])
            {
                allADCPins[j] = me->ADCConfig->pAdcPinIoAttr[i].adcPin;
                if(j < (NUM_OF_ALL_ENABLED_ADC_PIN -1))
                {
                  /* move invalid key flag to the next position */
                  allADCPins[j+1] = ADC_PIN_MAX;
                }
                break;
            }
        }
    }
    
    
    ADC_DMACmd(ADC1, ENABLE);
     
    
    /**
     * sort the pin array, this actually maped the pin to channel
     */
    //tpQsort((int32*)(&(allADCPins)), 0, (ArraySize(allADCPins)-1));
    
    me->isCreated = TRUE;
    //ADCDrv_StartScanning(me);

}



/**
* ADC Driver object destructor
* @param[in]    me              the ADC Driver object
*/
void ADCDrv_Xtor(cADCDrv * me)
{
    ASSERT(me);
    if(me->isCreated)
    {
        me->isCreated = FALSE;
        ASSERT(numOfRegisteredAdcObj);
        numOfRegisteredAdcObj--;
    }
    if(0 == numOfRegisteredAdcObj)
    {
        int8 i;
        for(i = 0; i < NUM_OF_ALL_ENABLED_ADC_PIN; i++)
        {
            allADCPins[i] = ADC_PIN_MAX;
        }
        currRegisteredCh = 0;
#ifdef HAS_DIRECT_ADC_ACCESS_BY_PORT_BIT
        indexOfTail = -1;
#endif
        /* Enable ADC1 DMA */
        ADC_DMACmd(ADC1, DISABLE);
        ADC_Cmd(ADC1, DISABLE);
    }
}

/**
* Start the ADC scanning
* @param[in]    me              the ADC Driver object
*/
void ADCDrv_StartScanning(cADCDrv * me)
{
    ASSERT(me && (me->isCreated));
    if(TRUE == conversionDone)
    {
        ASSERT(channelCounter==0);
        conversionDone = FALSE;
        /* Start ADC1 Software Conversion */ 
	//ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    }
    return;
}

/**
* This function reset the scanning status of the adc driver.Please note this function
* will not stop the adc scanning as long as your state machine is still polling adc, 
* if you want to stop adc scanning, you should stop the adc polling 
* @param[in]    me              the ADC Driver object
*/
void ADCDrv_ResetAdcConversionStatus()
{
    //ADC_StopOfConversion(ADC1);
    /* Start ADC1 Software Conversion */ 
	//ADC_SoftwareStartConvCmd(ADC1, DISABLE);
    channelCounter = 0;
    conversionDone = TRUE;
}

/**
* Check ADC conversion status
* @param[in]    me              the ADC Driver object
*/
bool ADCDrv_IsConversionDone(cADCDrv * me)
{
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
    ASSERT(me && (me->isCreated)&&(adcPin < ADC_PIN_MAX));
    /* i is the index of the all enabled adc pin set*/
    uint8 i;
    /**
     *  pin belongs to this object
     *  otherwise it will return an minus value
     */
    int32 returnValue = ADC_DATA_IS_NOT_READY;
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
        ASSERT((returnValue >= ADC_MIN_VALUE && returnValue <= ADC_MAX_VALUE));
    }
    
    return returnValue;
}

#ifdef HAS_DIRECT_ADC_ACCESS_BY_PORT_BIT
/**
* This function provide a way to read the adc raw value by pin port and bit info.
* @param[in]    ioPort       general io port
* @param[in]    ioBit        general io bit
* @param[out]   return raw adc value
*/
int32 ADCDrv_GetAdcRawDataByPortBit(eIoPort ioPort, eIoBit ioBit)
{
    int8 index = 0;
    int8 i = 0;
    int32 returnValue = ADC_DATA_IS_NOT_READY;
    ASSERT(-1 != indexOfTail);
    for(index = 0; index < NUM_OF_ALL_ENABLED_ADC_PIN; index++)
    {
        if(ioPort == enabledAdcPinInfo[index].ioPort && ioBit == enabledAdcPinInfo[index].gpioBit)
        {
          break;
        }
    }
    ASSERT(index < NUM_OF_ALL_ENABLED_ADC_PIN);
    for(i = 0; i < NUM_OF_ALL_ENABLED_ADC_PIN; i++)
    {
        if(enabledAdcPinInfo[index].adcPin == allADCPins[i])
        {
            break;
        }
    }
    ASSERT(i < NUM_OF_ALL_ENABLED_ADC_PIN);;
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
#endif

void ADC1_COMP_IRQHandler(void)
{
    if(ADC_GetITStatus(ADC1,ADC_IT_EOC))
    {
        ADC_ClearITPendingBit(ADC1,ADC_IT_EOC);
        ASSERT(conversionDone == FALSE);
        if(channelCounter >= currRegisteredCh)
        {
            ASSERT(0);//we should never come to here
            ADCDrv_ResetAdcConversionStatus();
        }
        else
        {
            adcAllPinBuf[channelCounter] = ADC_GetConversionValue(ADC1);//This reading also clean the EOC interrupt flag
            channelCounter++;
           
        }
    }
    
//    if(ADC_GetITStatus(ADC1,ADC_IT_EOSEQ))
//    {
//        ADC_ClearITPendingBit(ADC1,ADC_IT_EOSEQ);
//        /* 
//        * The sequence convertion is done, Here we reset all value again make sure that
//        * everything goes to default
//        */
//        ASSERT(channelCounter == currRegisteredCh);
//        ASSERT(conversionDone == FALSE);
//        channelCounter = 0;
//        conversionDone = TRUE;
//    }
    
}

