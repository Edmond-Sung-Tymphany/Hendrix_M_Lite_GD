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
#define GPIO_IN_B1_SW          GPIO_0   // PC13

// Power
#define GPIO_OUT_SYSPWR_ON     GPIO_2

//ASE-TK
#define GPIO_OUT_BOOT_STATUS_SW     GPIO_8  //PC14, CONF_SW
#define GPIO_OUT_ASE_RST_N     GPIO_9  //PA11, ASE-TK Reset N
#define GPIO_OUT_ASE_SYS_EN    GPIO_10 //PA12, ASE-TK System Enable
#define GPIO_IN_ASE_REQ_PDN    GPIO_11 //PA8,  ASE-TK require Power Down


//Other
#define GPIO_OUT_IOEXP_RST     GPIO_20 //PB12


/* ADC pin definition
 */
#define ADC_HW_VER             ADC_PIN1 //PC4, hardware version

/*******************************************************************************
 *  Power configs go here
 ******************************************************************************/


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
    LED1_R = LED_MIN,
    LED1_G,
    LED1_B,
    LED2_R,
    LED2_G,
    LED2_B,
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

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
