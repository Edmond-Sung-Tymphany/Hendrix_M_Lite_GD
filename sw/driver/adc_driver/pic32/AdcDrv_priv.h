/**
 *  @file      adcDrv_priv.h
 *  @brief     This file presents low level configuration to pic32MX ADC driver
 *  @author    Bob.Xu
 *  @date      19-Jan-2013
 *  @copyright Tymphany Ltd.
 */
#ifndef ADCDRV_PRIV_H
#define ADCDRV_PRIV_H

#include "AdcDrv.h"
#include "tp_hwsetup_pic32.h" //PERIPHERAL_CLOCK

/* ADC Main control registers configuration */

/**
 *  ADC result format | Auto convert | Disable module operation at idle mode \
 *  ADC sample auto-start off | Sample enable
 */
#define REG1_CONFIG                  (ADC_FORMAT_INTG | ADC_CLK_AUTO | \
                                      ADC_IDLE_STOP | ADC_AUTO_SAMPLING_OFF | \
                                      ADC_SAMP_OFF | (1<<_AD1CON1_CLRASAM_POSITION) )

/**
*  ADC ref external | disable offset test \
* | Scan Input | perform enabledPinNum samples \
* | use dual buffers | use alternate mode (use muxa only)
*/
#define REG2_CONFIG(enabledPinNum)   (ADC_VREF_AVDD_AVSS|ADC_OFFSET_CAL_DISABLE \
                                     |ADC_SCAN_ON|((enabledPinNum-1) << _AD1CON2_SMPI_POSITION)\
                                     |ADC_ALT_BUF_OFF|ADC_ALT_INPUT_OFF)

/**
*  Use ADC internal clock as conversion clock source and
*  set auto sample time in case if auto sampling is set
*
* According to pic32 spec, Tad = Tpb * 2 * (ADCS<7:0>+1), the Minimum value of
* Tad is 154 nS required in spec. So in here the target velue of Tad is 200 nS
* then ADCS = (Tad/(Tpb * 2) - 1).
*/
#define ADC_TPB_IN_NS                      (1000000000/PERIPHERAL_CLOCK)
#define ADC_TAD_IN_NS                      (200)
#define REG3_ADCS                          (ADC_TAD_IN_NS / (ADC_TPB_IN_NS * 2) - 1)
#define REG3_CONFIG                        (ADC_CONV_CLK_SYSTEM | ADC_SAMPLE_TIME_31 \
                                            | REG3_ADCS)

#define MainReg1_Config()                  AD1CON1=REG1_CONFIG
#define MainReg2_Config(enabledPinNum)     AD1CON2=REG2_CONFIG(enabledPinNum)
#define MainReg3_Config()                  AD1CON3=REG3_CONFIG
#define MainReg4_Config(enabledPinSet)     AD1CSSL |= enabledPinSet

#define ADC_Enable()                       {AD1CON2bits.CSCNA = TRUE;\
                                            AD1CON1bits.ON=TRUE;}

#define ADC_StartScanning()                 AD1CON1bits.ASAM = TRUE;
/**
 * AD1CON1bits.ON=FALSE only disable the adc sampling, it doesn't diable adc
 * Interrupt. IFS0bits.AD1IF  is set after the sampling is done,
 * the adc interrupt is generated based on this bit, so if you do
 * AD1CON1bits.ON=FALSE, you should also set IFS0bits.AD1IF to FALSE.
 * The previous project did this setting inside the interrupt
 */
#define ADC_Disable()                      {AD1CON2bits.CSCNA = FALSE;\
                                            AD1CON1bits.ON=FALSE;\
                                            AD1CON1bits.ASAM=FALSE;\
                                            IFS0bits.AD1IF=FALSE;}

#define ADC_SetMode(mode)                  (AD1CON1bits.ASAM=(mode))
#define ADC_GetConversionStatus()          BusyADC10()
#define ADC_ReadADCBuffer(bufIndex)        ReadADC10(bufIndex)
#define Enable_AdcInterrupt()              {IFS0bits.AD1IF=0;\
                                            IPC5bits.AD1IP=2;\
                                            IEC0bits.AD1IE=1;}

#define ADC_MIN_VALUE                       0
#define ADC_MAX_VALUE                       1023
#define ADC_CHANNEL_MAX                     16
#endif /* ADCDRV_PRIV_H */
