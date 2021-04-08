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
    //KEY
    GPIO_IN_STB_SW = GPIO_0,    //PC13  Standby button
    GPIO_IN_RESET_SW,           //PB0,  Config button
    GPIO_IN_BT_SW,              //PB4,  Bluetooth button
    GPIO_IN_MIC_MUTE_SW,        //PD2,  Mic mute button
    GPIO_IN_VOL_UP_SW,          //PB3,  Volume up button
    GPIO_IN_VOL_DOWN_SW,        //PD2,  Volume down button

    //Audio
    GPIO_IN_AUXIN_JACK_DET,     //PA5
    GPIO_OUT_AMP_SDZ,           //PB13
    GPIO_OUT_AMP_MUTE,          //PB14
    GPIO_IN_AMP_FAULTZ,         //PC14, High:normal operation, Low:fault condition(Over-temp, DC Detect)
    GPIO_OUT_BOOST_CAP_ON,      //PC15
    GPIO_OUT_AUDIO_DSP_DAC_3V3_EN,    //PC10, for Optical buffer in, DSP, DAC
    GPIO_OUT_AUDIO_ADC_3V3_EN,  //PA15
    GPIO_OUT_AUDIO_DAC_MUTE_IN_N,     //PB15
    GPIO_OUT_AUDIO_DAC_MUTE_OUT_N,    //PB14
    GPIO_OUT_AUDIO_DAC_RST_N,         //PB13
    GPIO_OUT_AUDIO_SMPS_EN,       //PC0, enable +/-14V, 33V for AMP + audio output circult 

    //DSP
    GPIO_OUT_DSP_SELFBOOT_EN_N, //PC11
    GPIO_OUT_DSP_RST_N,         //PB1
    GPIO_IN_DSP_TUNE,           //PB2, DSP get in EQ tuning mode,MCU need disable DSP IIC

    //ASE-TK
    GPIO_OUT_BOOT_STATUS_SW,    //PA8,  CONF_SW
    GPIO_OUT_ASE_RST_N,         //PA11, ASE Reset N
    GPIO_OUT_ASE_SYS_EN,        //PA12, ASE System Enable
    GPIO_IN_ASE_REQ_PDN,        //PA8,  ASE require Power Down
    GPIO_OUT_ASE_5V_EN,         //PC7,  ASE 5V enable

    //TOUCH KEY
    GPIO_IN_TCH_333_RDY,        //PB5
    GPIO_IN_TCH_263_RDY,        //PB15
    GPIO_OUT_TOUCH_I2C_SW,      //PB3

    //Other
    GPIO_OUT_IOEXP_RST,         //PB12
    GPIO_IN_AUDIO_JACK_DET5,
    GPIO_IN_AUDIO_JACK_DET1,
} eGpioPinDef;

/* EXTI definition
 */
#define EXTI_LINE_TCH_333_RDY  EXTI_Line5  //PB5
#define EXTI_LINE_TCH_263_RDY  EXTI_Line15  //PB15


/* ADC pin definition
 *
 * Note Key order must be equal to channel order, for exmaple
 *     PC0 (ADC_IN0)  ==> assign to ADC_PIN0
 *     PA1 (ADC_IN1)  ==> assign to ADC_PIN1
 *     PC4 (ADC_IN14) ==> assign to ADC_PIN14
 *
 * The wrong example is
 *     PC0 (ADC_IN0)  ==> assign to ADC_PIN3
 *     PA1 (ADC_IN1)  ==> assign to ADC_PIN2
 *     PC4 (ADC_IN14) ==> assign to ADC_PIN1
 *
 * The easist way is assign ADC_INx to ADC_PINx
 */
#define ADC_WF_NTC        ADC_PIN12   //PC2, ADC_IN12, Woofer NTC
#define ADC_AMP_NTC       ADC_PIN4    //PA4, ADC_IN4, AMP NTC
//#define ADC_DSP_NTC       ADC_PIN9    //PB1, ADC_IN9, DSP NTC (will support on EVT2)
#define ADC_HW_VER        ADC_PIN14   //PC4, ADC_IN14, hardware version



/*******************************************************************************
 *  DSP
 ******************************************************************************/
#define DSP_I2C_ADDR_ES_EVT   0x76  //old sample: ES1, EVT
#define DSP_I2C_ADDR_DVT_MP   0x70  //new sample: DVT, MP



/*******************************************************************************
 *  ADC configs go here
 ******************************************************************************/
#define NUM_OF_ALL_ENABLED_ADC_PIN   5  //array size of attrADCPins[]



/*******************************************************************************
 *  I2C config
 ******************************************************************************/
#define I2C_CLK_KHZ (100)
//#define I2C_CLK_KHZ (350)


/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* Number of key event (1 adc pin may have multiple events) */
#define NUM_OF_ADC_KEY               0
#define NUM_OF_GPIO_KEY              4
#define NUM_OF_AZ_INTEG_TOUCH_KEY    1  /* Although touch key have many event, config layer think it have only 1 event. = ArraySize(azIntegTOuchKeyPins) of attachedDevices.c */
#define NUM_OF_ALL_KEY               (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY + NUM_OF_AZ_INTEG_TOUCH_KEY)

#define KEYBOARD_NUMBER              2

#define IO_EXPANDER_NUM              (1)


/*******************************************************************************
 *  Led configs go here
 ******************************************************************************/
typedef enum
{
    LED_MIN = 0,
    LED1_GREEN = LED_MIN,
    LED2_BLUE,
    LED0_RED,
    LED4_GREEN,
    LED5_BLUE,
    LED3_RED,
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
void I2C1_LowLevel_ForcePullLow(void);
void I2C2_LowLevel_Init(void);


#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
