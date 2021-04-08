/**
 * @file      HardwareProfile.h
 * @brief     Header file for general Hardware profile
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "BootLoader.h" //each file must include Bootloader.h
#include "HardwareProfile_flash.h" 



/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/
//   Part number defining Macro
#if (((__PIC32_FEATURE_SET__ >= 300) && (__PIC32_FEATURE_SET__ <= 799)))
    #define __PIC32MX3XX_7XX__
#else
    #error("Controller not supported")
#endif


// Common macros 
// Clock frequency values
// These directly influence timed events using the Tick module.  They also are used for UART and SPI baud rate generation.
#define GetSystemClock()        CPU_OSC_FREQ_HZ   // Hz
#define GetInstructionClock()   (GetSystemClock()/1)    
#define GetPeripheralClock(SystemClockHz) (CPU_OSC_FREQ_HZ/(1 << OSCCONbits.PBDIV))  // Divisor is dependent on the value of FPBDIV set(configuration bits).
#define PERIPHERAL_CLOCK        GetPeripheralClock(CPU_OSC_FREQ_HZ)
#define CORE_COUNT_PER_MS       (CPU_OSC_FREQ_HZ/2/1000)
#define CORE_COUNT_PER_US       (CPU_OSC_FREQ_HZ/2/1000000)

/* Core Timer Opertaion
 * Note that the core timer will overflow every 214.748 seconds. (under clock frequency 40MHz)
 */
#define GetSystemTimeUs()       ( ReadCoreTimer()/(CPU_OSC_FREQ_HZ/2/1000000) )
#define GetSystemTimeMs()       ( ReadCoreTimer()/(CPU_OSC_FREQ_HZ/2/1000   ) )
#define TOGGLES_PER_SEC          18
#define CORE_TICK_RATE           (CPU_OSC_FREQ_HZ/2/TOGGLES_PER_SEC)


//Tick
#define BSP_TICKS_PER_SEC    100U


// Demo board hardware profiles
#if defined(PIC32_STARTER_KIT)
    #include "HardwareProfile_PIC32MX_USB_StarterKit.h"
#elif defined(PIC32_POLK_CAMDEN_SQUARE)
    #include "HardwareProfile_PIC32MX_PolkCamdenSquare.h"
#elif defined(PIC32_MOFA)
    #include "HardwareProfile_PIC32MX_Mofa.h"
#else
    /* Note ****: User has to define board type depending on the development board.
    To do this, in the MPLAB IDE navigate to menu Project->Build Options->Project.
    Select "MPLAB PIC32 C Compiler" tab. Select categories as "General" from the dropdown list box.
    Click ADD button and define the DEMO_BOARD under "Preprocessor Macros".*/

    #error ("Demo board is either not defined or not defined properly. \
             Supported values for this macro are BOARD_EXPLORER_16/ BOARD_USB_STARTER_KIT.");
#endif


#endif  //HARDWARE_PROFILE_H
