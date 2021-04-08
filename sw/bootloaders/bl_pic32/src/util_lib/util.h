/**
 * @file      util.h
 * @brief     Header file for BSP
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef __UTIL_H__
#define __UTIL_H__


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "Bootloader.h"

#ifdef __DEBUG //connect to debugger and run the program
    #define DEBUGGER_PAUSE() __asm volatile ("sdbbp 0")
    #define DEBUGGER_NOP()   __asm volatile ("nop")
#else
    #define DEBUGGER_PAUSE()
    #define DEBUGGER_NOP()
#endif



/*****************************************************************************
 * Macro                                                                     *
 *****************************************************************************/
#define MIN(a, b) (((a) < (b)) ? (a): (b))
#define MAX(a, b) (((a) > (b)) ? (a): (b))



/****************************************************************************
 * Function Prototype                                                       *
 ****************************************************************************/
void reboot(void);
BOOL ValidAppWrong(void);
void JumpToApp(void);


#endif /* __UTIL_H__ */
