/*
-------------------------------------------------------------------------------
TYMPHANY LTD


                  Boot-Loader common functions
                  -------------------------
                  SW Module Document

@file        bootloader_common.c
@brief       Implemented a boot-loader which will NOT be updated through OTA
@author      Viking Wang
@date        02-Nov-2016
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2015-11     Wesley Lee
DESCRIPTION: First Draft.
SCO/ERROR  :
-------------------------------------------------------------------------------
*/

#include "stm32f0xx.h"
#include "commonTypes.h"
#include "bootloader_common.h"

typedef void (*pFunction)(void);

void bl_jumpAddr(uint32 address)
{
    pFunction Jump_To_Application;
    uint32_t JumpAddress;

    // Shut down any tasks running
    RCC_DeInit();
    SysTick->CTRL   = 0;  // reset the Systick Timer
    SysTick->LOAD   = 0;
    SysTick->VAL    = 0;
//    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);     // Select HSI as system clock source

//    __set_PRIMASK(1);   // Disable interrupts

    /* Jump to user application */
    JumpAddress = *(__IO uint32_t*) (address + 4);
    Jump_To_Application = (pFunction) JumpAddress;

    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) address);
    
    /* Jump to application */
    Jump_To_Application();
}

uint32 bl_calcChecksum(uint32* pStart, uint32* pEnd)
{
    uint32* pAddr = pStart;
    uint32 checksum = 0xFFFFFFFF;

    /* Enable CRC clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
    CRC_ResetDR();

    while (pAddr < pEnd)
    {
        checksum = CRC_CalcCRC(*pAddr);
        pAddr++;
    }

    /* Disable CRC clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, DISABLE);

    return checksum;
}




