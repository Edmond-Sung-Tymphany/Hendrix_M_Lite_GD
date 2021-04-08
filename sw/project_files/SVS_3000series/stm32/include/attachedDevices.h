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

#define DB_INDEX_OF_LP          5
#define DB_INDEX_OF_PEQ1        8
#define DB_INDEX_OF_PEQ2        12
#define DB_INDEX_OF_PEQ3        16
#define DB_INDEX_OF_RGC         20
#define DB_INDEX_OF_VOL         22
#define DB_INDEX_OF_PHASE       23
#define DB_INDEX_OF_POLARITY    24
#define DB_INDEX_OF_TUNNING     25

#define VALUE_MAGNIFICATION             10


/*******************************************************************************
 *  General settings configs go here
 ******************************************************************************/
/* Normally: V */
#define ADC_THRESHOLD_LEVEL_1   0
#define ADC_THRESHOLD_LEVEL_2   50      /* */

#define ADC_THRESHOLD_LEVEL_3   124     /* 0.10*/
#define ADC_THRESHOLD_LEVEL_4   620     /* 0.5*/

/* Normally: V */
#define ADC_THRESHOLD_LEVEL_5   744     /* 0.6V */
#define ADC_THRESHOLD_LEVEL_6   1240    /* 1.0V */

/* Normally: V */
#define ADC_THRESHOLD_LEVEL_7   1489    /* 1.2V */
#define ADC_THRESHOLD_LEVEL_8   1985    /* 1.6V */

/* Normally: V */
#define ADC_THRESHOLD_LEVEL_9   2233    /* 1.8V */
#define ADC_THRESHOLD_LEVEL_10  2730    /* 2.2V */

/* Normally: V */
#define ADC_THRESHOLD_LEVEL_11  2978    /* 2.4V */
#define ADC_THRESHOLD_LEVEL_12  3475    /* 2.8V */

/*******************************************************************************
 *  Pin definition
 ******************************************************************************/
/* GPIO pin definition
 */

//POWER
#define GPIO_OUT_POWER_CTRL         (GPIO_0)
#define GPIO_IN_CHARGE_OUT_OC       (GPIO_1)
//Audio
#define GPIO_OUT_AMP_MUTE           (GPIO_2)
#define GPIO_OUT_AMP_STDBY          (GPIO_3)
#define GPIO_IN_TRIGGER_IN          (GPIO_4)
#define GPIO_IN_JACK_DET            (GPIO_5)
//DSP
//Other
#define GPIO_OUT_IOEXP_RST          (GPIO_6)
#define GPIO_OUT_LED_PLUS_MINUS     (GPIO_7)
#define GPIO_OUT_LED_LPF            (GPIO_8)
#define GPIO_OUT_LED_PHASE          (GPIO_9)
#define GPIO_OUT_LED_RESET          (GPIO_11)


#ifdef USING_BD2242G_CTRL
#define GPIO_BD2242G_CTRL           (GPIO_10)
#endif
// ADC
#define ADC_AMP_OTP     ADC_PIN5    //PA5, ADC_IN5, AMP NTC
#define ADC_AC_SENSE    ADC_PIN7
#define ADC_KEY         ADC_PIN6
#define ADC_JACK        ADC_PIN9    // this is for trigger cable

/*******************************************************************************
 *  DSP
 ******************************************************************************/
#define DSP_I2C_ADDR_ES_EVT   0x76  //old sample: ES1, EVT
#define DSP_I2C_ADDR_DVT_MP   0x70  //new sample: DVT, MP



/*******************************************************************************
 *  ADC configs go here
 ******************************************************************************/

/* For ADC PIN */
#define NUM_OF_ALL_ENABLED_ADC_PIN   5  //array size of attrADCPins[]




/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* Number of key event (1 adc pin may have multiple events) */
#define NUM_OF_ADC_KEY_PIN      1
#define NUM_OF_ADC_KEY          6
#define NUM_OF_GPIO_KEY         0
#define NUM_OF_ALL_KEY          (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY)

#define KEYBOARD_NUMBER         1

#define IO_EXPANDER_NUM         (1)


/*******************************************************************************
 *  Led configs go here
 ******************************************************************************/
//typedef enum
//{
//    LED_MIN = 0,
//    LED0_RED = LED_MIN,
//    LED1_GREEN,
//    LED2_BLUE,
//    LED3_RED,
//    LED4_GREEN,
//    LED5_BLUE,
//    LED_MAX
//}eLed;

typedef enum
{
    LED_MIN = 0,
    LED_BAR_MIN = LED_MIN,
    LED_BAR_1 = LED_BAR_MIN,
    LED_BAR_2,
    LED_BAR_3,
    LED_BAR_4,
    LED_BAR_5,
    LED_BAR_6,
    LED_BAR_7,
    LED_BAR_8,
    LED_BAR_9,
    LED_BAR_10,
    LED_BAR_11,
    LED_BAR_MAX = LED_BAR_11,
    LED_PLUS,
    LED_MINUS,
    LED_VOL,
    LED_STBY_BLUE,
    LED_STBY_AMBER,

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
*   USB GPIO configuration and Clock select
*/
void UsbDrv_Init(void);
void UsbDrv_DeInit(void);

/*
* I2C GPIO configuration (calls by I2CDrv_Ctor() from I2CDrv.c)
* init SDL and SDA pins according project
*/

void I2C1_LowLevel_Init(void);
void I2C2_LowLevel_Init(void);

void MCO_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
