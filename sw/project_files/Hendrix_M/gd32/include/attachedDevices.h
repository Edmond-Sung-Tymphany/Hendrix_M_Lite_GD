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
#include "bsp.h"


/*******************************************************************************
 *  General settings configs go here
 ******************************************************************************/
// SAM will check PRODUCT_VERSION_NUM to determine upgrade or not.
#define PRODUCT_VERSION "Hendrix"

/*******************************************************************************
 *  DSP configs go here
 ******************************************************************************/
#define DSP_VERSION     "0.1.0.0"

// gpio key
#define GPIO_IN_MULTI_KEY          GPIO_1
#define GPIO_IN_BT_KEY              GPIO_2

// BT pins
#define BT_INPUT0           GPIO_3      //LED0
#define BT_INPUT1           GPIO_4      //LED1
#define BT_INPUT2           GPIO_5      //LED2
#define BT_OUTPUT1          GPIO_6
#define BT_OUTPUT2          GPIO_7
#define BT_OUTPUT3          GPIO_8
#define BT_OUTPUT4          GPIO_9
#define BT_OUTPUT5          GPIO_10

// power control
//#define GPIO_OUT_BT_RESET           GPIO_11
#define BT_RESET_PIN           GPIO_11
#define GPIO_OUT_PWR_EN          GPIO_12
#define GPIO_IN_DC_IN_DET           GPIO_14
#define GPIO_IN_POWER_KEY           GPIO_15
#define GPIO_OUT_DSP_3V3            GPIO_16
#define GPIO_OUT_EX_CHG_CTL     GPIO_17
#define GPIO_OUT_BAT_CHG_EN     GPIO_18
#define GPIO_IN_BAT_CHG_STATUS1  GPIO_19
#define GPIO_IN_BAT_CHG_STATUS2  GPIO_20
#define GPIO_IN_BAT_PG           GPIO_21
#define GPIO_OUT_BOOST_EN          GPIO_22
#define GPIO_IN_DSP_TUNING           GPIO_23

#define GPIO_OUT_IOEXP_RST1      GPIO_24
#define GPIO_OUT_IOEXP_RST2      GPIO_25  /* IO expander for LED*/

#define GPIO_OUT_TW_FAULT        GPIO_26
#define GPIO_OUT_WF_FAULT        GPIO_27
#define GPIO_CHG_SHUD            GPIO_28

#define GPIO_OUT_AMP_ON            GPIO_29
#define GPIO_OUT_TW_PWDNN          GPIO_30
#define GPIO_OUT_WF_MUTE           GPIO_31

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
#ifdef HAS_BATTERY_DETECT
#define BATTERY_ADC_PIN         ADC_PIN6
#define NUM_OF_BATTERY_ADC_PIN  1
#else
#define NUM_OF_BATTERY_ADC_PIN  0
#endif

#ifdef HAS_DC_IN
#define DC_IN_ADC_PIN           ADC_PIN8
#define NUM_OF_DC_IN_ADC_PIN    1
#else
#define NUM_OF_DC_IN_ADC_PIN    0
#endif

#ifdef HAS_BATTERY_NTC
#define BATTERY_NTC_PIN         ADC_PIN7
#define NUM_OF_BATTERY_NTC_PIN  1
#else
#define NUM_OF_BATTERY_NTC_PIN  0
#endif

#ifdef HAS_HW_VERSION_TAG
#define HW_VERSION_ADC_PIN      ADC_PIN9
#define NUM_OF_VERSION_ADC_PIN  1
#else
#define NUM_OF_VERSION_ADC_PIN  0
#endif

#define NUM_OF_ADC_KEY_PIN              3//3

#define NUM_OF_ADC_POWER_PIN    ( NUM_OF_BATTERY_ADC_PIN\
                                + NUM_OF_DC_IN_ADC_PIN\
                                + NUM_OF_BATTERY_NTC_PIN\
                                + NUM_OF_VERSION_ADC_PIN)

#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_KEY_PIN + NUM_OF_ADC_POWER_PIN)

#define NUM_OF_ADC_KEY                  0
#define NUM_OF_LINEAR_KNOB_KEY          3

#define NUM_OF_GPIO_KEY                 1

#define NUM_OF_ALL_KEY                  (NUM_OF_LINEAR_KNOB_KEY + NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY)

#define KEYBOARD_NUMBER                 2

#define IO_EXPANDER_NUM                 (1)
/*******************************************************************************
 *  I2C config
 ******************************************************************************/
#define I2C1_CLK_KHZ (350)
#define I2C2_CLK_KHZ (350)

/*******************************************************************************
 *  Led configs go here
 ******************************************************************************/
typedef enum
{
    LED_MIN = 0,
    LED_PWR_R = LED_MIN,
    LED_PWR_G,
    LED_PWR_B,
    LED_BAT_1,
    LED_BAT_2,
    LED_BAT_3,
    LED_BAT_4,
    LED_BAT_5,
    LED_BAT_6,
    LED_BAT_7,
    LED_BAT_8,
    LED_BAT_9,
    LED_BAT_10,
    LED_MAX
} eLed;

#define NUM_OF_IOEXPANDER_LED  LED_MAX

extern tIoeLedDevice ioeGpioMbSleepConfig;
extern tIoeLedDevice ledSleepConfig;


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
//#ifdef HAS_MCO
void MCO_Init(void);
//#endif


#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
