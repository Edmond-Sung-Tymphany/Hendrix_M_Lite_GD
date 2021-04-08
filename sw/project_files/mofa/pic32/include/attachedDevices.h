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
#include "attachedDevicesMcu.h" //PRODUCT_VERSION_MCU

/*******************************************************************************
 *  General settings configs go here
 ******************************************************************************/
// SAM will check PRODUCT_VERSION_NUM to determine upgrade or not.
#define PRODUCT_VERSION "MOFA v" PRODUCT_VERSION_MCU " "


/*******************************************************************************
 *  Allplay configs go here
 ******************************************************************************/
/* MCU GPIO/SSI -> SAM GPIO 13 (This is PB124 S2 aka factory reset */
#define     SAM_FACTORY_RESET_IO        TRISEbits.TRISE5
#define     SAM_FACTORY_RESET_HIGH      do{ LATESET = 1<<5; } while(0)
#define     SAM_FACTORY_RESET_LOW       do{ LATBCLR = 1<<5; } while(0)


/*******************************************************************************
 *  Power configs go here 
 ******************************************************************************/


/* MCU_SAM_RESET PIC32 RB12 - >SAM J11 Pin 16 (This is the SAM processor reset) */
#define     SAM_RESET_PIN_IO            TRISBbits.TRISB12
#define     SAM_RESET_PIN_PORT          PORTBbits.RB12
#define     SAM_RESET_PIN_LAT           LATBbits.LATB12
#define     SAM_RESET_PIN_ANSEL         ANSELBbits.ANSB12

#define     RESET_PIN_IO                TRISBbits.TRISB2
#define     RESET_PIN_HIGH              do{ LATBSET = 1<<2; } while(0)
#define     RESET_PIN_LOW               do{ LATBCLR = 1<<2; } while(0)

#define     SAM3V3_EN_PIN_IO             TRISGbits.TRISG8
#define     SAM3V3_EN_PIN_HIGH           do{ LATGSET = 1<<8; } while(0)
#define     SAM3V3_EN_PIN_LOW            do{ LATGCLR = 1<<8; } while(0)

#define     DC3V3_EN_PIN_IO             TRISBbits.TRISB6
#define     DC3V3_EN_PIN_HIGH           do{ LATBSET = 1<<6; } while(0)
#define     DC3V3_EN_PIN_LOW            do{ LATBCLR = 1<<6; } while(0)

#define     I2S_CLK_EN_PIN_IO           TRISGbits.TRISG7
#define     I2S_CLK_EN_PIN_HIGH         do{ LATGSET = 1<<7; } while(0)
#define     I2S_CLK_EN_PIN_LOW          do{ LATGCLR = 1<<7; } while(0)

#define     NUM_OF_UARTS                2
#define     _UART_DEV_2

/*******************************************************************************
 *  DSP configs go here
 ******************************************************************************/
#define PRODUCT_VERSION_DSP "2.5"

#define     SDZ_PIN_ON                  //do{LATBSET=1<<13;  TRISBbits.TRISB13=0;}while(0);
#define     SDZ_PIN_OFF                 //do{LATBCLR=1<<13;  TRISBbits.TRISB13=0;}while(0);

/* The amplifier chip(TAS5613A) used in MOFA do not have mute function. */
#define     AMP_UNMUTE             //do{TRISBbits.TRISB7=1;}while(0)  //input
#define     AMP_MUTE               //do{LATBCLR=1<<7;  TRISBbits.TRISB7=0;}while(0) //output low
#define     AMP_RESET_OFF          do{TRISBbits.TRISB7=1;}while(0)  //input
#define     AMP_RESET_ON           do{LATBCLR=1<<7;  TRISBbits.TRISB7=0;}while(0) //output low

#define     RCA_OUT_UNMUTE         do{LATECLR=1<<0;  TRISEbits.TRISE0=0;}while(0) //output low
#define     RCA_OUT_MUTE           do{LATESET=1<<0;  TRISEbits.TRISE0=0;}while(0) //output high

extern tI2CDevice polk_allplay_DSP;

#define    SET_I2C1_SCL_INPUT     TRISDbits.TRISD10 = 1
#define    SET_I2C1_SDA_INPUT     TRISDbits.TRISD9 = 1

#define    SET_I2C1_SCL_OUTPUT    TRISDbits.TRISD10 = 0
#define    SET_I2C1_SDA_OUTPUT    TRISDbits.TRISD9 = 0

/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ADC_KEY_PIN              1
#define NUM_OF_ADC_POWER_PIN            0
#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_KEY_PIN + NUM_OF_ADC_POWER_PIN)
/* For GPIO PIN */
#define NUM_OF_GPIO_KEY_PIN             2

#define NUM_OF_ADC_KEY                  0
#define NUM_OF_KNOB_KEY                 2
#define NUM_OF_GPIO_KEY                 2
#define NUM_OF_ALL_KEY                  (NUM_OF_ADC_KEY + NUM_OF_KNOB_KEY + NUM_OF_GPIO_KEY)

#define KEYBOARD_NUMBER                 2

#define NUM_OF_IR_KEY                   7  //For new IR controller

/****** Knob detection related ******/
/*  wave form at knob input adc when the knob is rotated in anti-clockwise and clockwise
 *           ----  |  ------           knob_state = high, ADC = 1023,  by experiment
 *           |     |        |
 *           |     |        |
 *      ------     |        ------|    knob_state = mid,  ADC = 845, by experiment
 *      |          |              |
 * -----|          |              |    knob_state = low,  ADC = 684, by experiment
 * ------clockwise-|----anti-clockwise---------
 */
#define ADC_KNOB_MID_TH     750     // adc theshold above which the knob state is not disguished as LOW
#define ADC_KNOB_HIGH_TH    950     // adc theshold above which the knob state is HIGH
/*******************************************************************************
 *  DMA configs go here
 ******************************************************************************/
/* DMA has 4 channels */
#define NUM_OF_AVAILABLE_DMA_CHANNEL              4
//#define DMA_DRV_ENABLE
/*******************************************************************************
 *  Led configs go here
 ******************************************************************************/
typedef enum
{
    LED_MIN = 0,
    LED_RED = LED_MIN,
    LED_GREEN,
    LED_BLUE,
    LED_MAX
}eLed;
#define NUM_OF_PWM_LED                  LED_MAX

extern const tDevice * const devices[];
extern const uint16 NUM_OF_ATTACHED_DEVICES;

#define mLED_Red              LATDbits.LATD0
#define mLED_Blue             LATCbits.LATC14
#define mLED_Green            LATCbits.LATC13
#define mLED_Red_Tgl()        mLED_Red   = !mLED_Red
#define mLED_Blue_Tgl()       mLED_Blue  = !mLED_Blue  // Yellow for USB Starter Kit III
#define mLED_Green_Tgl()      mLED_Green = !mLED_Green

/* temp place for version line */
#define SOFTWARE_VERSION_STRING PRODUCT_VERSION PLATFORM_VERSION PRODUCT_VERSION_DSP

//#define HW_I2C_DRV_LIB

/*******************************************************************************
 *  UART configs go here
 ******************************************************************************/
/* Enabling UART_HAS_DMA_SUPPORT requires to enabel DMA_DRV_ENABLE first */
//#define UART_HAS_DMA_SUPPORT

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

/**
* I2C Driver hardware pin initialization
* @param[in]    channel              i2C channel (ref: enum eI2C_Channel)
*/
void I2cDrv_Init(eI2C_Channel channel);

/**
* I2C Driver hardware pin de-initialization
* @param[in]    channel              i2C channel (ref: enum eI2C_Channel)
*/
void I2cDrv_DeInit(eI2C_Channel channel);

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
