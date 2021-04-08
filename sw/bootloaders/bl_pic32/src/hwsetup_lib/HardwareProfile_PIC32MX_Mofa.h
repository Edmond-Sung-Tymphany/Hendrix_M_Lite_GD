/**
 * @file      HardwareProfile_PIC32MX_PolkCamdenSquare.h
 * @brief     Header file for Hardware profile (Polk CamdenSquare)
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef __HARDWAREPROFILE_PIC32MX_USB_STARTER_KIT_H__
#define __HARDWAREPROFILE_PIC32MX_USB_STARTER_KIT_H__



/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "BootLoader.h" //each file must include Bootloader.h


/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/
// Maximum System frequency of 80MHz for PIC32MX3xx, PIC32MX4xx,
// PIC32MX5xx, PIC32MX6xx and PIC32MX7xx devices.
#define CPU_OSC_FREQ_HZ (40000000L)


/*******************************************************************/
/******** Application specific definitions *************************/
/*******************************************************************/
//These defintions will tell the main() function which board is
//  currently selected.  This will allow the application to add
//  the correct configuration bits as well use the correct
//  initialization functions for the board.  These defitions are only
//  required for this demo.  They are not required in
//  final application design.

// IOPORT bit masks can be found in ports.h
#define CONFIG      (CND_ON)
#define PINS        (CND7_ENABLE)
#define PULLUPS     (CND6_PULLUP_ENABLE | CND7_PULLUP_ENABLE)
#define INTERRUPT   (CHANGE_INT_ON | CHANGE_INT_PRI_2)

//LED
#define mLED_Red              LATDbits.LATD0
#define mLED_Blue             LATCbits.LATC13
#define mLED_Green            LATCbits.LATC14
#define mLED_Red_Tgl()        mLED_Red   = !mLED_Red
#define mLED_Blue_Tgl()       mLED_Blue  = !mLED_Blue  // Yellow for USB Starter Kit III
#define mLED_Green_Tgl()      mLED_Green = !mLED_Green
#define BlinkLED_Red()        (mLED_Red   = ((ReadCoreTimer() & 0x0800000) != 0))
#define BlinkLED_Blue()       (mLED_Blue  = ((ReadCoreTimer() & 0x0800000) != 0))  // Yellow for USB Starter Kit III
#define BlinkLED_Green()      (mLED_Green = ((ReadCoreTimer() & 0x0800000) != 0))
#define InitLED() do{    \
                        TRISDbits.TRISD0 = 0;  \
                        TRISCbits.TRISC13 = 0;  \
                        TRISCbits.TRISC14 = 0;  \
                        mLED_Red = 0; mLED_Blue = 0; mLED_Green = 0;\
                    }while(0)

// Error indication.
#define Error()   do{mLED_Red = 1; mLED_Blue = 1; mLED_Green = 1;} while(0);

#endif /* #ifndef __HARDWAREPROFILE_PIC32MX_USB_STARTER_KIT_H__ */
