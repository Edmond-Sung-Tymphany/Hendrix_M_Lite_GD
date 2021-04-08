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

#include "deviceTypes_v2.h"


/*******************************************************************************
 *  General settings configs go here
 ******************************************************************************/
// SAM will check PRODUCT_VERSION_NUM to determine upgrade or not.
#define PRODUCT_VERSION "Fender"

/*******************************************************************************
 *  DSP configs go here
 ******************************************************************************/
#define DSP_VERSION     "0.1.0.0"

// gpio key
#define GPIO_IN_SOURCE_KEY          GPIO_1
#define GPIO_IN_BT_KEY              GPIO_2

// BT pins
#define BT_INPUT0           GPIO_3      //LED0
#define BT_INPUT1           GPIO_4      //LED1
#define BT_OUTPUT1          GPIO_5
#define BT_OUTPUT2          GPIO_6
#define BT_OUTPUT3          GPIO_7
#define BT_OUTPUT4          GPIO_8
#define BT_OUTPUT5          GPIO_9
#define BT_PWR_EN           GPIO_10     /* BT power enable */

// power control
#define GPIO_OUT_BT_RESET           GPIO_11
#define BT_RESET_PIN                GPIO_OUT_BT_RESET
#define GPIO_OUT_AMP_W_PDN          GPIO_12     // woofer : TAS5760 SDN
#define GPIO_OUT_AMP_T_PDN          GPIO_13     // tweeter : TPA3118 SDN
#define GPIO_IN_DC_IN_DET           GPIO_14
#define GPIO_IN_AUDIO_DET           GPIO_15
#define GPIO_IN_POWER_KEY           GPIO_16
#define GPIO_OUT_DSP_3V3            GPIO_17
// amplifier mute control
#define GPIO_OUT_AMP_T_MUTE     GPIO_20         // tweeter : TPA3118 mute

#define GPIO_IN_DSP_TUNING          GPIO_21
#define GPIO_IN_SHAPE_EQ            GPIO_22


/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* For ADC PIN */
/* For ADC PIN */
#define NUM_OF_ADC_KEY_PIN              3//3
#ifdef HAS_HW_VERSION_TAG
#define NUM_OF_ADC_POWER_PIN            1
#else
#define NUM_OF_ADC_POWER_PIN            0
#endif
#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_KEY_PIN + NUM_OF_ADC_POWER_PIN)

#define NUM_OF_ADC_KEY                  0
#define NUM_OF_LINEAR_KNOB_KEY          3
#define NUM_OF_GPIO_KEY                 2
#define NUM_OF_ALL_KEY                  (NUM_OF_LINEAR_KNOB_KEY + NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY)

#define KEYBOARD_NUMBER                 2

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
void I2C1_LowLevel_Init();
void I2C2_LowLevel_Init(void);

void I2C1_GPIO_Deinit(void);
void I2C1_GPIO_ReInit(void);

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
