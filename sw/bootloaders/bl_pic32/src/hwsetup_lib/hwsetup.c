/**
 * @file      hwsetup.c
 * @brief     This file implements functions to initialize the system's HW (Braven & Polk product family)
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 *
 * TODO: Split this file for every project, current setting is for MOFA only
 */


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <p32xxxx.h>
#include <stdlib.h>
#include <plib.h>       //Include to use PIC32 peripheral libraries
#include "BootLoader.h" //each file must include Bootloader.h
#include "HardwareProfile.h"
#include "tp_uart.h"
#include "hwsetup.h"
#include "assert.h"
#include "dbgprint.h"
#include "pragma.h"
#include "HardwareProfile.h"


/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
static BOOL sam_initialized= FALSE;


/*****************************************************************************
 * Function Implemenation                                                    *
 *****************************************************************************/
/* fake_init==TRUE means SAM have initialized by application before.
 * Bootloader does not need initialized it, but need to destroy.
 */
void sam_init(BOOL real_init)
{    
    sam_initialized= TRUE;
    
    if(!real_init)
        return;

    DBG_PRINT("\r\n\r\n\r\n");
    DBG_PRINT("***** Initialize SAM *****\r\n");   
    // set RESET pin as a non-analog pin
    SAM_RESET_PIN_ANSEL= 0; 

    //Set pin as output
    SAM_RESET_PIN_IO = 0;
    SAM3V3_EN_PIN_IO = 0;
    
    SAM3V3_EN_PIN_OFF;  //turn off V3P3
    SAM_RESET_PIN_LAT= 0; // output low to deactivate SAM
    TimerUtil_delay_us(50*1000); //50ms
    
    SAM3V3_EN_PIN_ON;
    TimerUtil_delay_us(50*1000); //50ms

     // output high to activate SAM
    SAM_RESET_PIN_LAT = 1;
}


void sam_destroy(void)
{
    if(!sam_initialized) {
        return;
    }
    
    // output low to deactivate SAM
    SAM_RESET_PIN_LAT = 0; 

    /* Let SAM have enough time to power off */
    TimerUtil_delay_us(50*1000); //50ms

    //Turn off V3P3
    SAM3V3_EN_PIN_OFF;

    sam_initialized= FALSE;
}

void bsp_init(void)
{
    SYSTEMConfig(CPU_OSC_FREQ_HZ, SYS_CFG_ALL);

#ifndef PIC32_STARTER_KIT
    /* The JTAG is on by default on POR.  A PIC32 Starter Kit uses the JTAG, but
     * for other debug tool use, like ICD 3 and Real ICE, the JTAG should be off
     * to free up the JTAG I/O 
     * PS: Without this line, PIN27(SAM_RESET_PIN_IO) will be configured as JTAG input pin, and have wrong behavior
     */
    DDPCONbits.JTAGEN = 0;
#endif

    // configure IOPORTS PORTD.RD0, RD1, RD2 as outputs (LED Green, Yellow, Red)
    //PORTSetPinsDigitalOut(IOPORT_D, BIT_1 | BIT_5 | BIT_4); //red, blue, green LED

    // initialize the port pins states = output low
    //PORTClearBits(IOPORT_D, BIT_1 | BIT_5 | BIT_4); //red, blue, green LED

    //Turn off all LEDs
    InitLED();
    mLED_Blue = 0;
    mLED_Green= 0;
    mLED_Red  = 0;

    //init timer interrupt
    timer3_init();

    //init uart_console
#if defined(BL_MSG_PRINT)
    uart_console_init();
#endif

    //mute amplifier and RCA out
    AMP_MUTE;
    RCA_OUT_MUTE;
    //power off dsp and amplifier
    DC3V3_EN_PIN_OFF;

    // Ok now to enamInitAllLEDsble multi-vector interrupts
    INTEnableSystemMultiVectoredInt();
    INTCONSET = _INTCON_MVEC_MASK;   /* configure multi-vectored interrupts */

    bsp_enable_watchdog();
}


void bsp_destroy(void)
{
    timer3_deinit();
    
#ifdef BL_MSG_PRINT
    uart_console_wait_tx_flush(500); //wait up to 500ms
    uart_console_destroy();
#endif

    //mute amplifier and RCA out
    AMP_MUTE;
    RCA_OUT_MUTE;
    //power off dsp and amplifier
    DC3V3_EN_PIN_OFF;

    //Make sure again to disable all interrupt
    IEC0= 0;  //Disable interrupt on IEC0
    IEC1= 0;  //Disable interrupt on IEC1
}


/* Watchdog */
void bsp_enable_watchdog(void)
{
    EnableWDT();
}

void bsp_disable_watchdog(void)
{
    DisableWDT();
}

void bsp_feed_watchdog(void)
{
    ClearWDT();
}