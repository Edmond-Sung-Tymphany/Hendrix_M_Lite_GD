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

/*******************************************************************************
 *  Pin definition
 ******************************************************************************/
/* GPIO pin definition
 */

typedef enum {
    //KEY
    GPIO_IN_STB_SW = GPIO_0,    //PC13  Standby button
    GPIO_IN_RESET_SW,           //PC14, Reset button
    GPIO_IN_CONN_SW,            //PB4, Connect button
    GPIO_IN_VOL_UP_SW,          //PB3, Volume up button
    GPIO_IN_VOL_DOWN_SW,        //PD2, Volume down button

    //Audio
    GPIO_IN_AUXIN_JACK_DET,     //PA0
    GPIO_OUT_AMP_SDZ,           //PB13
    GPIO_OUT_AMP_MUTE,          //PB14
    GPIO_IN_AMP_FAULTZ,         //PB14, High:normal operation, Low:fault condition(Over-temp, DC Detect)

    //DSP
    GPIO_OUT_DSP_VCC_ON,        //PA15
    GPIO_OUT_DSP_I2S_ENABLE_EVT2_REWORK_N,  //PC6. It is old pin which use only on EVT2-Rework device. It is already ship to customer, thus back-compatible for it.
    GPIO_OUT_DSP_I2S_ENABLE_N,  //PB15
    GPIO_IN_DSP_TUNE,           //PB0, DSP get in EQ tuning mode,MCU need disable DSP IIC

    //ASE-TK
    GPIO_OUT_BOOT_STATUS_SW,    //PA8,  CONF_SW
    GPIO_OUT_ASE_RST_N,         //PA11, ASE-TK Reset N
    GPIO_OUT_ASE_SYS_EN,        //PA12, ASE-TK System Enable
    GPIO_IN_ASE_REQ_PDN,        //PA8,  ASE-TK require Power Down

    //Other
    GPIO_OUT_IOEXP_RST,         //PB12
} eGpioPinDef;

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
#define ADC_AMP_NTC       ADC_PIN4    //PA4, ADC_IN4, AMP NTC
#define ADC_DSP_NTC       ADC_PIN9    //PB1, ADC_IN9, DSP NTC (will support on EVT2)
#define ADC_HW_VER        ADC_PIN14   //PC4, ADC_IN14, hardware version



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
#define NUM_OF_ADC_KEY               0
#define NUM_OF_GPIO_KEY              5
#define NUM_OF_ALL_KEY               (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY)

#define KEYBOARD_NUMBER              1

#define IO_EXPANDER_NUM                 (1)


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
    LED1_GREEN = LED_MIN,
    LED2_BLUE,
    LED0_RED,
    LED4_GREEN,
    LED5_BLUE,
    LED3_RED,
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
