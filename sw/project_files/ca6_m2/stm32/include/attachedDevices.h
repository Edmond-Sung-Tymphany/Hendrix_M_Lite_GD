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

/*******************************************************************************
 *  Pin definition
 ******************************************************************************/
/* GPIO pin definition
 */
//KEY
#define GPIO_IN_VOL_UP      GPIO_14  //PC13, Volume up button
#define GPIO_IN_VOL_DOWN    GPIO_15  //PC14, Volume down button

// Power
#define GPIO_OUT_SYSPWR_ON     GPIO_2


//Audio
#define GPIO_IN_AUDIO_JACK_DET  GPIO_3  //PA0
#define GPIO_OUT_AMP_SDZ        GPIO_4  //PB13
#define GPIO_OUT_AMP_MUTE       GPIO_6  //PB15
#define GPIO_OUT_DSP_RST_N      GPIO_7  //PB2
#define GPIO_IN_DSP_TUNE        GPIO_8  //PB0, DSP get in EQ tuning mode,MCU need disable DSP IIC
#define GPIO_IN_CONF           GPIO_16  //PB14, Configure button

//ASE-TK
#define GPIO_OUT_BOOT_STATUS_SW     GPIO_9  //PB8, CONF_SW
#define GPIO_OUT_ASE_RST_N     GPIO_10  //PA11, ASE-TK Reset N
#define GPIO_OUT_ASE_SYS_EN    GPIO_11 //PA12, ASE-TK System Enable
#define GPIO_IN_ASE_REQ_PDN    GPIO_12 //PA8,  ASE-TK require Power Down


//Other
#define GPIO_OUT_IOEXP_RST     GPIO_20 //PB12


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
#define ADC_TW_NTC        ADC_PIN13   //PC3, ADC_IN13, Tweeter NTC
#define ADC_AMP1_NTC      ADC_PIN4    //PA4, ADC_IN4, amplifer 1 NTC
#define ADC_AMP2_NTC      ADC_PIN5    //PA5, ADC_IN5, amplifer 2 NTC

#define ADC_HW_VER        ADC_PIN14   //PC4, ADC_IN14, hardware version




/*******************************************************************************
 *  ADC configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ALL_ENABLED_ADC_PIN      5  //array size of attrADCPins[]




/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* Number of key event (1 adc pin may have multiple events) */
#define NUM_OF_ADC_KEY               0
#define NUM_OF_GPIO_KEY              3
#define NUM_OF_ALL_KEY               (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY)

#define KEYBOARD_NUMBER              1


/*******************************************************************************
 *  Led configs go here
 ******************************************************************************/
typedef enum
{
    LED_MIN = 0,
    LED0_RED = LED_MIN,
    LED1_GREEN,
    LED2_BLUE,
    LED3_RED,
    LED4_GREEN,
    LED5_BLUE,
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
