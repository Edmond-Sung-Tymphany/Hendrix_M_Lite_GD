/**
 * @file      TimerUtil.h
 * @brief     Time opertaion
 * @author    Gavin Lee
 * @date      10-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef TIMER_UTIL_H
#define TIMER_UTIL_H

#define TIMER3_TARGET_FREQ (1000) /* 1000Hz ==> 1ms*/
#define TIMER3_PRESCALER (1)
/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "Bootloader.h"


/*****************************************************************************
 * Function Prototype                                                        *
 *****************************************************************************/
uint32 TimerUtil_getTimeMs(void);
uint32 GetCoreTimeMs(void);
void TimerUtil_resetTime(void);
void TimerUtil_delay_ms(uint32 delay_ms);
void TimerUtil_delay_us(uint32 delay_us);
void timer3_interrupt_handler(void);
void int3_init(void);
void int3_disable(void);


#endif
