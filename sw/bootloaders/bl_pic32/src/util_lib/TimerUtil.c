/**
 * @file      TimerUtil.c
 * @brief     Time opertaion
 * @author    Gavin Lee
 * @date      10-Mar-2014
 * @copyright Tymphany Ltd.
 */

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "assert.h"
#include "Bootloader.h"
#include "hwsetup.h" //DISABLE_INTERRUPT
#include "TimerUtil.h"
#include "util.h"
#include "ui.h"


/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
static volatile uint32 timer_ms= 0;



/*****************************************************************************
 * Function Implemenation                                                    *
 *****************************************************************************/
/* It will overflow every 49.71 days
 */
inline uint32 TimerUtil_getTimeMs()
{
    uint32 ret;
    DISABLE_INTERRUPT();
    ret= timer_ms;
    ENABLE_INTERRUPT();;
    return ret;
}


/* Get core time
 * Note that the core timer will overflow every 214.748 seconds. (under clock frequency 40MHz)
 */
uint32 GetCoreTimeMs(void)
{
    uint32 ms= GetSystemTimeUs()/1000;
    return ms;
}



void TimerUtil_resetTime(void)
{
    DISABLE_INTERRUPT();
    timer_ms = 0;
    ENABLE_INTERRUPT();
}



/**********************************************************************************
Function Name : timer3_init
Description   : Initializes Timer 3 in input capture mode with the trigger input
 *              on port B0.  
ParameterS    : none
Return value  : none
*******************************************************************************/
void timer3_init(void)
{
    T2CONbits.T32=0;        // Odd numbered and even numbered timers form a separate 16-bit timer
    T3CON = 0x0000;
    TMR3 = 0x00; //clear timer3
    {
        /*compute the right period register value */
        int32 periodRegValue = (PERIPHERAL_CLOCK / TIMER3_PRESCALER) / TIMER3_TARGET_FREQ;
        BOOL bLessthan16b = !(periodRegValue & 0xffff0000);
        assert(bLessthan16b) //the value must be within 16bits, check targetFreq and prescaler if assert happens
        PR3 = periodRegValue;
    }

    //Set _TIMER_3_VECTOR:
    IPC3bits.T3IP   = 3;  //Interrupt Priority (IPL3)
    T3CONbits.TON   = 0;  //stop timer3
    T3CONbits.SIDL  = 0;  //Continue operation even in Idle mode
    T3CONbits.TCKPS = 0;  //set timer3 clk to the same as peripheral bus clock
    T3CONbits.TSIDL = 0;  //still working in IDLE mode
    T3CONbits.TCKPS = 0;  //set timer3 clk to the same as peripheral bus clock
    T3CONbits.TGATE = 0;  //Disable gated time accumulation 
    T3CONbits.TCS   = 0;  //Ineternal peripheral clock
    
    IFS0bits.T3IF = 0;    //clear the Timer3 Interrupt Request Flag bit
    T3CONbits.TON = 1;    //Enable compare time base
    IEC0bits.T3IE = 1;    //Enable timer3 interrupt
}

void timer3_deinit(void)
{
    T3CONbits.TON = 0;  //stop timer3
    IEC0bits.T3IE = 0;  //timer3 interrupt disable
}

inline void timer3_interrupt_handler(void)   // 1ms interval
{
    IFS0bits.T3IF = 0;
    //mLED_Blue_Tgl();
    DISABLE_INTERRUPT();
    timer_ms++;
    ENABLE_INTERRUPT();

    update_ui(timer_ms);
}


//Will have no function when fill into exception
void TimerUtil_delay_ms(uint32 delay_ms)
{
    uint32 start_ms= TimerUtil_getTimeMs();
    while( (TimerUtil_getTimeMs()-start_ms) < delay_ms ) {
        ;
    }
}

/********************************************************************
* Function:     TimerUtil_delay_us()
*
* Precondition:
*
* Input:         Micro second
*
* Output:        None.
*
* Side Effects:    Uses Core timer. This may affect other functions using core timers.
                For example, core timer interrupt may not work, or may loose precision.
*
* Overview:     Provides Delay in microsecond.
*
*
* Note:             None.
********************************************************************/
void TimerUtil_delay_us(uint32 time_us)
{
    uint32 targetCount;
    uint32 bakupCount;
    uint8 loop = 0;
    // Assert "us" not zero. This must be caught during debug phase.
    assert(time_us!=0);
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