/**
 * @file      hex.c
 * @brief     The main source file for hex file parsing
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <p32xxxx.h>
#include <plib.h>
#include <GenericTypeDefs.h>
#include "Bootloader.h"
#include "HardwareProfile.h"
#include "util.h" //DBG_PRINT
#include "hex.h"
#include "assert.h"
#include "dbgprint.h"




/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/
#define DFU_RED_LED_ON_MS  300
#define DFU_RED_LED_OFF_MS 300
#define DFU_RED_LED_PERIOD_MS (DFU_RED_LED_ON_MS + DFU_RED_LED_OFF_MS)




/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
static BOOL ui_dfu_mode= FALSE;




/*****************************************************************************
 * Function Implemenation                                                    *
 *****************************************************************************/
__inline void update_ui(uint32 time_ms) 
{
    if( ui_dfu_mode && (time_ms%DFU_RED_LED_PERIOD_MS) < DFU_RED_LED_ON_MS )
    {
        mLED_Red = 1;
        mLED_Blue = 1;
        mLED_Green = 1;
    }
    else
    {
        mLED_Red = 0;
        mLED_Blue = 0;
        mLED_Green = 0;
    }
}


void dfu_mode_enable(BOOL enable)
{
	ui_dfu_mode= enable;
}