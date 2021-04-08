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
#define PRODUCT_VERSION "Polk Allplay v" PRODUCT_VERSION_MCU " "


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

#define     RESET_PIN_IO                TRISBbits.TRISB14
#define     RESET_PIN_HIGH          do{ LATBSET = 1<<14; } while(0)
#define     RESET_PIN_LOW           do{ LATBCLR = 1<<14; } while(0)

#define     DC3V3_EN_PIN_IO             TRISBbits.TRISB15
#define     DC3V3_EN_PIN_HIGH           do{ LATBSET = 1<<15; } while(0)
#define     DC3V3_EN_PIN_LOW            do{ LATBCLR = 1<<15; } while(0)

#define     NUM_OF_UARTS                2
#define     _UART_DEV_2


/*******************************************************************************
 *  DSP configs go here
 ******************************************************************************/
#define DSP_VERSION "Polk_BT_MP "

#define     SDZ_PIN_ON                  do{LATBSET=1<<13;  TRISBbits.TRISB13=0;}while(0);
#define     SDZ_PIN_OFF                 do{LATBCLR=1<<13;  TRISBbits.TRISB13=0;}while(0);

#define     AMP_MUTE_HIGH               do{LATFSET=1<<4;  TRISFbits.TRISF4=0;}while(0);
#define     AMP_MUTE_LOW                do{LATFCLR=1<<4;  TRISFbits.TRISF4=0;}while(0);

extern tI2CDevice polk_allplay_DSP;

#define    SET_I2C1_SCL_INPUT     TRISDbits.TRISD10 = 1
#define    SET_I2C1_SDA_INPUT     TRISDbits.TRISD9 = 1

#define    SET_I2C1_SCL_OUTPUT    TRISDbits.TRISD10 = 0
#define    SET_I2C1_SDA_OUTPUT    TRISDbits.TRISD9 = 0

/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ADC_KEY_PIN              2
#define NUM_OF_ADC_POWER_PIN            2
#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_KEY_PIN + NUM_OF_ADC_POWER_PIN)
/* For GPIO PIN */
#define NUM_OF_GPIO_KEY_PIN             3

#define NUM_OF_ADC_KEY                  7
#define NUM_OF_GPIO_KEY                 3
#define NUM_OF_ALL_KEY                  (NUM_OF_ADC_KEY + NUM_OF_GPIO_KEY)

#define KEYBOARD_NUMBER                 2
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

#define mLED_Red              LATDbits.LATD1
#define mLED_Blue             LATDbits.LATD5
#define mLED_Green            LATDbits.LATD4
#define mLED_Red_Tgl()        mLED_Red   = !mLED_Red
#define mLED_Blue_Tgl()       mLED_Blue  = !mLED_Blue  // Yellow for USB Starter Kit III
#define mLED_Green_Tgl()      mLED_Green = !mLED_Green

/* temp place for version line */
#define SOFTWARE_VERSION_STRING PRODUCT_VERSION PLATFORM_VERSION DSP_VERSION

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


#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
