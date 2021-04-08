/**
 *  @file      assert.h
 *  @brief     assert related function
 *  @author    Gavin
 *  @date      16-Mar-2014
 *  @copyright Tymphany Ltd.
 */
 
 
#ifndef ASSERT_H
#define ASSERT_H


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <plib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "Bootloader.h"
#include "dbgprint.h"
#include "util.h"
#include "tp_uart.h" //UART_CONSOLE
#include "uart_basic.h" //UARTTxSendString.h()
#include "hwSetup.h" //DISABLE_INTERRUPT



/*****************************************************************************
 * Function Implemenation                                                    *
 *****************************************************************************/
void delay_us(uint32 time_us)
{
    uint32 targetCount;
    uint32 bakupCount;
    uint8 loop = 0;
    
    if(time_us>0)
    {
        // backup current count of the core timer.
        bakupCount = ReadCoreTimer();

        // Core timer increments every 2 sys clock cycles.
        // Calculate the counts required to complete "us".
        targetCount = CORE_COUNT_PER_US * time_us;

        // Restart core timer.
        WriteCoreTimer(0);

        // Wait till core timer completes the count.
        while(ReadCoreTimer() < targetCount);

        // Restore count back.
        WriteCoreTimer(bakupCount + targetCount);
    }
}

void assert_handler(const char* pFile, uint32 line)
{
    DISABLE_INTERRUPT();
    
    char buf[512]= {0};
    snprintf(buf, sizeof(buf), "bootloader assert: %s:%d\r\n", pFile, line);

    //Turn off all LEDs
    mLED_Blue = 0;
    mLED_Green= 0;
    mLED_Red  = 0;

#if !(defined __DEBUG || defined NDEBUG)  // not debugger run or debug mode
    volatile bool assert_loop= TRUE;
    while(assert_loop)
#endif
    {       
        //use polling UART in exception handler
        bsp_feed_watchdog();
#ifndef NDEBUG //if debug mode
        UARTTxSendString(UART_CONSOLE, buf);
#endif
        delay_us(500*1000); //500ms
        BlinkLED_Red();
    };
    DEBUGGER_PAUSE(); // break into the debugger
}
#endif /* #ifndef ASSERT_H */
