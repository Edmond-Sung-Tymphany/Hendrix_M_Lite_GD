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
#define PRODUCT_VERSION "iBT150 v"

/*******************************************************************************
 *  Allplay configs go here
 ******************************************************************************/
/* MCU GPIO/SSI -> SAM GPIO 13 (This is PB124 S2 aka factory reset */
#define     SAM_FACTORY_RESET_IO        TRISEbits.TRISE5
#define     SAM_FACTORY_RESET_HIGH     // do{ LATESET = 1<<5; } while(0)
#define     SAM_FACTORY_RESET_LOW      // do{ LATBCLR = 1<<5; } while(0)


/*******************************************************************************
 *  Power configs go here 
 ******************************************************************************/


/* MCU_SAM_RESET PIC32 RB12 - >SAM J11 Pin 16 (This is the SAM processor reset) */
#define     SAM_RESET_PIN_IO            TRISBbits.TRISB12
#define     SAM_RESET_PIN_PORT          PORTBbits.RB12
#define     SAM_RESET_PIN_LAT           LATBbits.LATB12
#define     SAM_RESET_PIN_ANSEL         ANSELBbits.ANSB12

#define     RESET_PIN_IO                TRISBbits.TRISB14
#define     RESET_PIN_HIGH             // do{ LATBSET = 1<<14; } while(0)
#define     RESET_PIN_LOW             //  do{ LATBCLR = 1<<14; } while(0)

#define     DC3V3_EN_PIN_IO             TRISBbits.TRISB15
#define     DC3V3_EN_PIN_HIGH         //  do{ LATBSET = 1<<15; } while(0)
#define     DC3V3_EN_PIN_LOW          //  do{ LATBCLR = 1<<15; } while(0)

#define     NUM_OF_UARTS                2
#define     _UART_DEV_2


/*******************************************************************************
 *  DSP configs go here
 ******************************************************************************/
#define DSP_VERSION "N/A "


  
#define     MUTE_PIN                     GPIO_19
#define     SDZ_PIN                     GPIO_20

/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ADC_KEY_PIN              2
#define NUM_OF_ADC_POWER_PIN            0
#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_KEY_PIN + NUM_OF_ADC_POWER_PIN)

#define NUM_OF_ADC_KEY                  0
#define NUM_OF_LINEAR_KNOB_KEY          2
#define NUM_OF_GPIO_KEY                 1
#define NUM_OF_ALL_KEY                  (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY + NUM_OF_LINEAR_KNOB_KEY)
 
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

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
