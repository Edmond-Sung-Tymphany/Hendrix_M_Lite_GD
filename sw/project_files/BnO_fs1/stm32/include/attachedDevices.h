/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/
/**
* @file attachedDevices.h
* @brief The devices attached to the product.
* @author Christopher Alexander
* @date 26-Nov-2013
* @copyright Tymphany Ltd.
*/

/* TODO: Auto generate this data */
#ifndef ATTACHEDDEVICES_H
#define ATTACHEDDEVICES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "deviceTypes.h"
#include "stm32f0xx_exti.h" //EXTI_LineXX


/*******************************************************************************
 *  General settings configs go here
 ******************************************************************************/

/*******************************************************************************
 *  Pin definition
 ******************************************************************************/
/* GPIO pin definition
 */
 
typedef enum {
    //Power
    GPIO_IN_DC_IN=GPIO_0, //PB15, SMPS 15V DC input detection (FS1:PB15, FS2:PC6)
    GPIO_OUT_SYSPWR_ON,   //PB0, System power control (DCDC, AMP power)
    GPIO_OUT_BOOST_CAP1,  //PB1
    GPIO_OUT_BOOST_CAP2,  //PC7
    GPIO_OUT_BOOST_EN,    //PB4
    GPIO_IN_CH_STATE,     //PB5
    GPIO_OUT_CH_ON,       //PB9
    GPIO_OUT_SLOW_CH,     //PC5, low:1.9A, high:1.2A
    
    //GPIO Key
    GPIO_IN_POWER_KEY,    //PC14, Power key

    //Rotary Volume
    GPIO_IN_ROTARY_VOL_A, //Rotary volume A
    GPIO_IN_ROTARY_VOL_B, //Rotary volume B

    //Audio
    GPIO_OUT_AMP_SDZ,     //PB13
    GPIO_IN_AMP_FAULTZ,   //PB14, High:normal operation, Low:fault condition(Over-temp, DC Detect)
    GPIO_OUT_DSP_RST_N,   //PB2
    GPIO_IN_DSP_TUNE,     //PA15, DSP get in EQ tuning mode, MCU need disable DSP IIC

    //ASE
    GPIO_OUT_BOOT_STATUS_SW, //PB8, Notify ASE-TK MCU is on Bootlaoder(high) or application(low)
    GPIO_OUT_ASE_RST_N,      //PA11, ASE-TK Reset N
    GPIO_OUT_ASE_SYS_EN,     //PA12, ASE-TK System Enable
    GPIO_IN_ASE_REQ_PDN,     //PA8,  ASE-TK require Power Down

    //Touch
    GPIO_OUT_TCH_360_RST,  //PC10
    GPIO_IN_TCH_360_RDY,   //PC11
    GPIO_IN_TCH_572_RDY,   //PC12

    //IO Expender
    GPIO_OUT_IOEXP_RST,    //PB12

    //Touch
    GPIO_OUT_TCH_572_RST,  //PC8
    GPIO_OUT_TCH_POWER,    //PC9
}eGpioPinDef;


/* EXTI definition
 */
#define EXTI_LINE_TCH_360_RDY  EXTI_Line11  //PC11
#define EXTI_LINE_TCH_572_RDY  EXTI_Line12  //PC12



/* ADC pin definition
 *
 * Note Key order must be equal to channel order, for exmaple
 *     PA1 (ADC_IN1)  ==> assign to ADC_PIN1
 *     PC0 (ADC_IN10) ==> assign to ADC_PIN10
 *     PC4 (ADC_IN14) ==> assign to ADC_PIN14
 *
 * The wrong example is
 *     PC0 (ADC_IN10) ==> assign to ADC_PIN3
 *     PA1 (ADC_IN1)  ==> assign to ADC_PIN2
 *     PC4 (ADC_IN14) ==> assign to ADC_PIN1
 *  
 * The easist way is assign ADC_INx to ADC_PINx
 */
//Common
#define ADC_FS_KEY        ADC_PIN1   //PA1, ADC_IN1,  ADC key for SoftAP and Power
#define ADC_HW_VER        ADC_PIN14  //PC4, ADC_IN14, hardware version
#define ADC_NTC_WF_AMP    ADC_PIN10  //PC0, ADC_IN10, NTC for Woofer amplifier
#define ADC_NTC_TW_SPK    ADC_PIN11  //PC1, ADC_IN11, NTC for Twetter speaker
#define ADC_NTC_WF_SPK    ADC_PIN12  //PC2, ADC_IN12, NTF for Woofer speaker

//FS1 only
#define ADC_5V_SEN        ADC_PIN6   //PA6, ADC_IN6, ADC 5V Sense
#define ADC_PVDD_SEN      ADC_PIN7   //PA7, ADC_IN7, ADC PVDD Sense

   

/*******************************************************************************
 *  ADC configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ADC_KEY_PIN           1  //= ArraySize(attrADCPinsForKey) of attachedDevices.c
#define NUM_OF_ADC_POWER_PIN         3 //= ArraySize(attrAdcPowerPins) of attachedDevices.c
#define NUM_OF_ADC_AUDIO_PIN         3 //= ArraySize(attrAdcPowerPins) of attachedDevices.c
#define NUM_OF_ALL_ENABLED_ADC_PIN   (NUM_OF_ADC_KEY_PIN + NUM_OF_ADC_POWER_PIN + NUM_OF_ADC_AUDIO_PIN) /* Array length of attrAdcPowerPins[] on attachedDevices.c */


/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* Number of key event (1 adc pin may have multiple events) */
#define NUM_OF_GPIO_KEY_PIN          1  //= ArraySize(gpioKeyPins) of attachedDevices.c
#define NUM_OF_ADC_KEY               2  /* 2 key event on 1 ADC pin (Power, SoftAP), = ArraySize(adcKey) of attachedDevices.c */
#define NUM_OF_GPIO_KEY              1  /* 1 key event on 1 GPIO pin (Factory reset), = ArraySize(gpioKeyPins) of attachedDevices.c */
#define NUM_OF_AZ_INTEG_TOUCH_KEY    1  /* Although touch key have many event, config layer think it have only 1 event. = ArraySize(azIntegTOuchKeyPins) of attachedDevices.c */
#define NUM_OF_ROTATER_PAIR          1  /* 1 Rotater contain both Clock-wise and Counter-Clock-wise events */

#define NUM_OF_ALL_KEY               (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY + NUM_OF_AZ_INTEG_TOUCH_KEY + NUM_OF_ROTATER_PAIR)  // KeySrv: me->pConfig->keyboardSet[NUM_OF_ALL_KEY];

#define KEYBOARD_NUMBER              4  /* 4 keyboard include ADC + GPIO + TOUCH + ROTATER, == ArraySize(keyboardSet) */


/*******************************************************************************
 *  Led configs go here
 ******************************************************************************/
typedef enum
{
    LED_MIN = 0,
    LED_CONN_WHT = LED_MIN,
    LED_CONN_BLUE,
    LED_CONN_ORG,
    LED_CONN_RED,
    LED_PROD_RED,
    LED_PROD_WHT,
    LED_MAX
}eLed;

#define NUM_OF_IOEXPANDER_LED  LED_MAX


/*******************************************************************************
 *  OTHER CONFIGS
 ******************************************************************************/
extern const tDevice * const devices[];
extern const uint8 NUM_OF_ATTACHED_DEVICES;

//#define HW_I2C_DRV_LIB

/**
* UART Driver hardware pin initialization
* @param[in]    id              UART Driver ID (ref: enum eUartDevice)
*/
void UartDrv_Init(eTpUartDevice id);

/**
* UART Driver hardware pin de-initialization
* @param[in]    id              UART Driver ID (ref: enum eUartDevice)
*/
void UartDrv_Deinit(eTpUartDevice id);

/*
* I2C GPIO configuration (calls by I2CDrv_Ctor() from I2CDrv.c)
* init SDL and SDA pins according project
*/

void I2C1_LowLevel_Init(void);
void I2C2_LowLevel_Init(void);


#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
