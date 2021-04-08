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
#define GPIO_IN_STB_SW         GPIO_0   //PC13  Standby button

// PCM9211 5V : 1->ON, 0->OFF
#define GPIO_OUT_PCM9211_POWER      GPIO_1   // PA1
#define GPIO_OUT_PCM3V3_POWER       GPIO_2       // PC6:1->ON, 0->OFF

// amplifier power : 1->ON, 0->OFF
#define GPIO_OUT_AMP_POWER      GPIO_3       // PA6

//Audio
#define GPIO_OUT_1V5_POWER      GPIO_4       // PC7:1->ON, 0->OFF
#define GPIO_OUT_1V1_POWER      GPIO_5       // PA8:1->ON, 0->OFF
#define GPIO_OUT_1V2_POWER      GPIO_6       // PC9:1->ON, 0->OFF

// system 3.3V
#define GPIO_OUT_SYSPWR_ON      GPIO_7       // PA12:1->ON, 0->OFF

// wifi module 5V
#define GPIO_OUT_WIFI_POWER     GPIO_8       // PA11:1->ON, 0->OFF

// IR receiver
#define GPIO_OUT_IR_RX          GPIO_9      //PC3

//Other
#define GPIO_OUT_IOEXP_RST      GPIO_20      //PC2


#define GPIO_OUT_AMP_SDZ       GPIO_28
#define GPIO_OUT_DSP_RST_N     GPIO_29
#define GPIO_IN_DSP_TUNE     GPIO_27


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
#define ADC_KEY_BUTTON0     ADC_PIN10   // PC0
#define ADC_KEY_BUTTON1     ADC_PIN11   // PC1


/*******************************************************************************
 *  ADC configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ALL_ENABLED_ADC_PIN      2  //array size of attrADCPins[]


/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* Number of key event (1 adc pin may have multiple events) */
#define NUM_OF_ADC_KEY               4
#define NUM_OF_GPIO_KEY              1
#define NUM_OF_IR_KEY                11
#define NUM_OF_ALL_KEY               (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY + NUM_OF_IR_KEY)

#define KEYBOARD_NUMBER              3


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

#ifdef HAS_HW_SPI_DEVICE
void Spi1_LowLevel_Init(void);
void Spi2_LowLevel_Init(void);
#endif

#ifdef HAS_SWi2c_DEVICE
void SWi2c1_LowLevel_init(void);
void SWi2c2_LowLevel_init(void);
#endif

#ifdef HAS_PCM9211_CODEC
/* config the reset GPIO pin */
void Pcm9211_ResetSetup(int high);
void Pcm9211_ResetGPIOInit(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
