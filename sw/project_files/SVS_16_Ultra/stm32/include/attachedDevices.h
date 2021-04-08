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
#define     RESET_PIN_HIGH          // do{ LATBSET = 1<<14; } while(0)
#define     RESET_PIN_LOW           //  do{ LATBCLR = 1<<14; } while(0)

#define     DC3V3_EN_PIN_IO             TRISBbits.TRISB15
#define     DC3V3_EN_PIN_HIGH           //  do{ LATBSET = 1<<15; } while(0)
#define     DC3V3_EN_PIN_LOW            //  do{ LATBCLR = 1<<15; } while(0)

#define     NUM_OF_UARTS                2
#define     _UART_DEV_2


/*******************************************************************************
 *  DSP configs go here
 ******************************************************************************/
#define DSP_VERSION "N/A "

#define DB_INDEX_OF_LP          5
#define DB_INDEX_OF_PEQ1        8
#define DB_INDEX_OF_PEQ2        12
#define DB_INDEX_OF_PEQ3        16
#define DB_INDEX_OF_RGC         20
#define DB_INDEX_OF_VOL         22
#define DB_INDEX_OF_PHASE       23
#define DB_INDEX_OF_POLARITY    24
#define DB_INDEX_OF_TUNNING     25
  
#define     MUTE_PIN                    GPIO_19
#define     SDZ_PIN                     GPIO_20

/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ADC_KEY_PIN              1
#define NUM_OF_ADC_AMP_PIN              2
#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_KEY_PIN + NUM_OF_ADC_AMP_PIN)
/* For GPIO PIN */
#define NUM_OF_GPIO_POWER_PIN           4
#define NUM_OF_GPIO_AMP_MUTE_PIN        1
#define NUM_OF_GPIO_DISPLAY_PIN         3

/* For Infrared keys*/
#define NUM_OF_IR_KEY                   11  //For new IR controller
  
#define NUM_OF_ADC_KEY                  4
#define NUM_OF_GPIO_KEY                 0
#define NUM_OF_ALL_KEY                  (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY + NUM_OF_IR_KEY)
 
#define KEYBOARD_NUMBER                 2



/*******************************************************************************
 *  Display and menu go here
 ******************************************************************************/
#define NUM_OF_SCREEN_DIGIT             8
#define NUM_OF_PRESET                   3
#define NUM_OF_MENU_SETT_ITEMS          26
#define NUM_OF_PRESET_ITEMS             NUM_OF_MENU_SETT_ITEMS
#define VALUE_MAGNIFICATION             10
/*******************************************************************************
 *  Led configs go here
 ******************************************************************************/

#define NUM_OF_DISPLAY_GPIO_PIN

extern const tDevice * const devices[];
extern const uint8 NUM_OF_ATTACHED_DEVICES;

#define mLED_Red              LATDbits.LATD1
#define mLED_Blue             LATDbits.LATD5
#define mLED_Green            LATDbits.LATD4
#define mLED_Red_Tgl()        mLED_Red   = !mLED_Red
#define mLED_Blue_Tgl()       mLED_Blue  = !mLED_Blue  // Yellow for USB Starter Kit III
#define mLED_Green_Tgl()      mLED_Green = !mLED_Green

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

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
