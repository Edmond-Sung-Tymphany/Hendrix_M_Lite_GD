/**
 * @file      bsp_UsbStarterKit.c
 * @brief     Implement BSP for USB Startker Kit III
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */


/*****************************************************************************
 * Include                                                                    *
 *****************************************************************************/
#include <p32xxxx.h>
#include <stdlib.h>
#include <plib.h>
#include "BootLoader.h"
#include "HardwareProfile.h"



/*****************************************************************************
 * Configuration Bits                                                        *
 *****************************************************************************/
#pragma config FNOSC    = PRIPLL        // Oscillator Selection
#pragma config FPLLIDIV = DIV_2         // PLL Input Divider (PIC32 Starter Kit: use divide by 2 only)
#pragma config FPLLMUL  = MUL_20        // PLL Multiplier
#pragma config FPLLODIV = DIV_1         // PLL Output Divider
#pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
#pragma config FWDTEN   = OFF           // Watchdog Timer
#pragma config WDTPS    = PS1           // Watchdog Timer Postscale
#pragma config FCKSM    = CSDCMD        // Clock Switching & Fail Safe Clock Monitor
#pragma config OSCIOFNC = OFF           // CLKO Enable
#pragma config POSCMOD  = HS            // Primary Oscillator - note need high speed (HS) mode for a UART @ 115200
#pragma config IESO     = OFF           // Internal/External Switch-over
#pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable
#pragma config CP       = OFF           // Code Protect
#pragma config BWP      = OFF           // Boot Flash Write Protect
#pragma config PWP      = OFF           // Program Flash Write Protect
#pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select

#ifdef NDEBUG //release mode
  #pragma config DEBUG    = OFF           // Debugger Disable for Starter Kit
#else // debug mode
  #pragma config DEBUG    = ON            // Debugger Enabled for Starter Kit
#endif



/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
extern BOOL RunApplication;



/*****************************************************************************
 * Function Implemenation                                                    *
 *****************************************************************************/
void bsp_init(void)
{
    // configure IOPORTS PORTD.RD0, RD1, RD2 as outputs (LED Green, Yellow, Red)
    PORTSetPinsDigitalOut(IOPORT_D, BIT_0 | BIT_1 | BIT_2);

    // initialize the port pins states = output low
    PORTClearBits(IOPORT_D, BIT_0 | BIT_1 | BIT_2);   //red, yellow, green LED

#ifdef BL_FORCE_UPGRADE
    // PORTD.RD6, RD7 as inputs (SW1)
    PORTSetPinsDigitalIn(IOPORT_D, BIT_6);

    // Enable change notice, enable discrete pins and weak pullups
    mCNDOpen(CONFIG, PINS, PULLUPS);

    // Read the port to clear any mismatch on change notice pins
    dummy = mPORTDRead();

    // Config change notice interrupt flag
    ConfigIntCND(INTERRUPT);
#endif
    
    // Ok now to enamInitAllLEDsble multi-vector interrupts
    INTEnableSystemMultiVectoredInt();
    INTCONSET = _INTCON_MVEC_MASK;   /* configure multi-vectored interrupts */
}



void bsp_destroy(void)
{
    // Clear interrupt
#ifdef BL_FORCE_UPGRADE
    ConfigIntCND(0);
#endif
}


#ifdef BL_FORCE_UPGRADE
void __ISR(_CHANGE_NOTICE_VECTOR, ipl2) ChangeNotice_Handler(void) {
    // Clear the mismatch condition first
    dummy = PORTReadBits(IOPORT_D, BIT_6);

    // Then clear the interrupt flag
    mCNDClearIntFlag();
	
	// Notice to leave bootloader
    RunApplication= TRUE;
}
#endif /* #ifdef BL_FORCE_UPGRADE */