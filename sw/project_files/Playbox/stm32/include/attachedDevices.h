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
#define MAJOR_VER    0
#define MINOR_VER1   0
#define MINOR_VER2   0

// SAM will check PRODUCT_VERSION_NUM to determine upgrade or not.
#define PRODUCT_VERSION "f072rb"

/*******************************************************************************
 *  Power configs go here
 ******************************************************************************/



#define     NUM_OF_UARTS                1


/*******************************************************************************
 *  DSP configs go here
 ******************************************************************************/
#define DSP_VERSION "N/A "

#define     MUTE_PIN                     GPIO_19
#define     SDZ_PIN                     GPIO_20

extern const tI2CDevice polk_allplay_DSP;


/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ADC_KEY_PIN              1
#define NUM_OF_ADC_POWER_PIN            2
#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_KEY_PIN + NUM_OF_ADC_POWER_PIN)

#define NUM_OF_ADC_KEY                  0//3//4
#define NUM_OF_GPIO_KEY                 1//3
#define NUM_OF_ALL_KEY                  (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY)

#define KEYBOARD_NUMBER                 1


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


/*******************************************************************************
 *  OTHER CONFIGS
 ******************************************************************************/
extern const tDevice * const devices[];
extern const uint8 NUM_OF_ATTACHED_DEVICES;

/* temp place for version line */
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

void sEE_LowLevel_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
