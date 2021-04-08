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


/*******************************************************************************
 *  General settings configs go here
 ******************************************************************************/
#define MAJOR_VER    1
#define MINOR_VER1   0
#define MINOR_VER2   2

#define PRODUCT_VERSION "B&O S8MK2 v"


/*******************************************************************************
 *  Power configs go here 
 ******************************************************************************/




/*******************************************************************************
 *  DSP configs go here
 ******************************************************************************/


/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ADC_KEY_PIN              0
#define NUM_OF_ADC_POWER_PIN            0
#define NUM_OF_ADC_AUDIO_PIN            2
#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_KEY_PIN + NUM_OF_ADC_POWER_PIN+NUM_OF_ADC_AUDIO_PIN)
/* For GPIO PIN */

#define NUM_OF_ADC_KEY                  0
#define NUM_OF_GPIO_KEY                 1
#define NUM_OF_IR_KEY                   0
#define NUM_OF_ALL_KEY                  (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY + NUM_OF_IR_KEY)
//#define NUM_OF_ALL_KEY                  (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY/* + NUM_OF_IR_KEY*/)
 
#define KEYBOARD_NUMBER                 ((!!NUM_OF_ADC_KEY)+(!!NUM_OF_GPIO_KEY)+(!!NUM_OF_IR_KEY))
/*******************************************************************************
 *  Led configs go here
 ******************************************************************************/
typedef enum
{
    LED_MIN = 0,
    LED_RED = LED_MIN,
    LED_BLUE,
    LED_GREEN,
    LED_MAX
}eLed;

typedef enum
{
    PWM_CH_NULL,
    PWM_CH1,
    PWM_CH2,
    PWM_CH3,
    PWM_CH4,
    PWM_CH1N,
    PWM_CH2N,
    PWM_CH3N,
    PWM_CH4N,
}ePwmChannel;

#define NUM_OF_PWM_LED                  LED_MAX

extern const tDevice * const devices[];
extern const uint8 NUM_OF_ATTACHED_DEVICES;

#define mLED_Red              LATDbits.LATD1
#define mLED_Blue             LATDbits.LATD5
#define mLED_Red_Tgl()        mLED_Red   = !mLED_Red
#define mLED_Blue_Tgl()       mLED_Blue  = !mLED_Blue  // Yellow for USB Starter Kit III

/* temp place for version line */
#define DSP_VERSION "N/A "

#define SOFTWARE_VERSION_STRING PRODUCT_VERSION PLATFORM_VERSION DSP_VERSION

//#define HW_I2C_DRV_LIB

/* temp place for version line */
#define SOFTWARE_VERSION_STRING PRODUCT_VERSION PLATFORM_VERSION DSP_VERSION

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
void sEE_LowLevel_Init();

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
