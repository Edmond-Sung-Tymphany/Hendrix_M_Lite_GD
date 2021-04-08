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

/*******************************************************************************
 *  Pin definition
 ******************************************************************************/
/* GPIO pin definition
 */

typedef enum
{
    //Key
    GPIO_IN_MULTI_KEY= GPIO_0,
    GPIO_IN_INPUT_KEY,
    GPIO_IN_POWER_KEY,
    GPIO_IN_ROTARY_VOL_A,
    GPIO_IN_ROTARY_VOL_B,
    GPIO_IN_ROTARY_BAS_A,
    GPIO_IN_ROTARY_BAS_B,
    GPIO_IN_ROTARY_TRE_A,
    GPIO_IN_ROTARY_TRE_B,

    // BT pins
    BT_INPUT0,
    GPIO_OUT_BOOT_STATUS_SW,
    BT_OUTPUT1 = GPIO_OUT_BOOT_STATUS_SW,
    BT_PWR_EN,
    BT_3V3_EN,

    // power control
    BT_RESET_PIN,
    GPIO_OUT_PWR_EN,
    GPIO_OUT_AMP_ON,
    GPIO_OUT_PVDD_EN,
    GPIO_OUT_WF_MUTE,
    GPIO_OUT_TW_MUTE,
    GPIO_IN_WF_FAULT,
    GPIO_IN_TW_FAULT,
    GPIO_OUT_DSP_3V3,
    GPIO_IN_DSP_TUNING,

    GPIO_OUT_IOEXP_NRST,

} eGpioPinDef;


/*******************************************************************************
 *  Power IO expander GPIO configs go here
 ******************************************************************************/
#define IOE_OUT_CTL2            IOE_1
#define IOE_OUT_CTL3            IOE_2
#define IOE_OUT_ICHG_SEL        IOE_3
#define IOE_OUT_EX_CHG_CTL      IOE_4
#define IOE_OUT_BAT_CHG_EN      IOE_5
#define IOE_OUT_AMP_ON          IOE_6
#define IOE_OUT_BOOST_EN        IOE_7

/*******************************************************************************
 *  Audio IO expander GPIO configs go here
 ******************************************************************************/
#define IOE_IN_WF_FAULTZ        IOE_8
#define IOE_IN_TW_FAULT         IOE_9
#define IOE_OUT_TW_MUTE         IOE_10
#define IOE_OUT_WF_MUTE         IOE_11


/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* For ADC PIN */
#ifdef HAS_HW_VERSION_TAG
#define HW_VERSION_ADC_PIN      ADC_PIN9
#endif
#define NUM_OF_ADC_POWER_PIN            1
#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_POWER_PIN)

#define NUM_OF_ROTATER_PAIR             1  /* 1 Rotater contain both Clock-wise and Counter-Clock-wise events */
#define NUM_OF_GPIO_KEY                 3
#define NUM_OF_ALL_KEY                  (3/*3 rotate keyboard*/ + NUM_OF_GPIO_KEY)

#define KEYBOARD_NUMBER                 4

#define IO_EXPANDER_NUM                 (3)
/*******************************************************************************
 *  I2C config
 ******************************************************************************/
#define I2C1_CLK_KHZ (350)
#define I2C2_CLK_KHZ (100)

/*******************************************************************************
 *  Led configs go here
 ******************************************************************************/
typedef enum
{
    LED_MIN = 0,
    LED_PWR = LED_MIN,
    LED_BT,
    LED_AUX,
    LED_RCA,
    LED_VOL_0,
    LED_VOL_1,
    LED_VOL_2,
    LED_VOL_3,
    LED_VOL_4,
    LED_VOL_5,
    LED_VOL_6,
    LED_VOL_7,
    LED_VOL_8,
    LED_VOL_9,
    LED_VOL_10,

    LED_BAS_0,
    LED_BAS_1,
    LED_BAS_2,
    LED_BAS_3,
    LED_BAS_4,
    LED_BAS_5,
    LED_BAS_6,
    LED_BAS_7,
    LED_BAS_8,
    LED_BAS_9,
    LED_BAS_10,

    LED_TRE_0,
    LED_TRE_1,
    LED_TRE_2,
    LED_TRE_3,
    LED_TRE_4,
    LED_TRE_5,
    LED_TRE_6,
    LED_TRE_7,
    LED_TRE_8,
    LED_TRE_9,
    LED_TRE_10,

    LED_MAX
} eLed;

#define NUM_OF_IOEXPANDER_LED  LED_MAX


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
void I2C1_LowLevel_Init();
void I2C2_LowLevel_Init(void);

void I2C1_GPIO_Deinit(void);
void I2C1_GPIO_ReInit(void);

#ifdef HAS_MCO
void MCO_Init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
