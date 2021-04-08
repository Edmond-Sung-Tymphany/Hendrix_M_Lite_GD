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
 *  General settings configs
 ******************************************************************************/
#define PRODUCT_VERSION "Linux v"

/*******************************************************************************
 *  Power configs
 ******************************************************************************/


/*******************************************************************************
 *  UART configs
 ******************************************************************************/
#define     NUM_OF_UARTS                2
#define     _UART_DEV_2


/*******************************************************************************
 *  DSP configs go here
 ******************************************************************************/
#define DSP_VERSION "N/A "

#define     MUTE_PIN                    GPIO_19
#define     SDZ_PIN                     GPIO_20

/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ADC_KEY_PIN              1
#define NUM_OF_ADC_POWER_PIN            0
#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_KEY_PIN + NUM_OF_ADC_POWER_PIN)
/* For GPIO PIN */
#define NUM_OF_GPIO_POWER_PIN           1
#define NUM_OF_GPIO_DISPLAY_PIN         3
#define NUM_OF_GPIO_AUDIO_PIN           2

#define NUM_OF_ADC_KEY                  4
#define NUM_OF_GPIO_KEY                 0
#define NUM_OF_ALL_KEY                  (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY)
 
#define KEYBOARD_NUMBER                 1

/* For Infrared keys*/
#define NUM_OF_IR_KEY                   11  //For new IR controller

/*******************************************************************************
 *  Display and menu
 ******************************************************************************/
#define NUM_OF_SCREEN_DIGIT             8
#define NUM_OF_PRESET                   3
#define NUM_OF_MENU_SETT_ITEMS          24
#define NUM_OF_PRESET_ITEMS             NUM_OF_MENU_SETT_ITEMS

/*******************************************************************************
 *  Led config
 ******************************************************************************/

#define NUM_OF_DISPLAY_GPIO_PIN

extern const tDevice * const devices[];
extern const uint8 NUM_OF_ATTACHED_DEVICES;

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


#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
