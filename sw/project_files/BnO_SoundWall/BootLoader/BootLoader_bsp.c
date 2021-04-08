/*****************************************************************************
* BSP for PICDEM 2 PLUS with PIC18F452
* Last Updated for Version: 4.4.01
* Date of the Last Update:  Mar 23, 2012
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information:
* Quantum Leaps Web sites: http://www.quantum-leaps.com
*                          http://www.state-machine.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#include "stm32f0xx.h"
#include "system_stm32f0xx.h"
#include "commonTypes.h"
#include "trace.h"
#include "UartDrv.h"
#include "attacheddevices.h"
#include "memory_config.h"
#include "BootLoader_bsp.h"
#include "BootLoader_common.h"

volatile static uint32_t currTime = 0;

__weak void SysTick_Handler(void) {                       /* system clock tick ISR */
    ++currTime;
}

/**
* @brief Get the tick elapsed
* @return   uint32  the tick elapsed
*/
__weak uint32 getSysTime()
{
    return currTime;
}

void bl_BSP_init(void)
{
    SystemCoreClockUpdate();
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        /* Capture error */
        while (1);
    }
}

/******************************************************************************/
/*            Functions Prototype                                             */
/******************************************************************************/
void Bl_ExpHandler(char const Q_ROM * const Q_ROM_VAR file, int line);



/******************************************************************************/
/*            Functions Implemenation                                         */
/******************************************************************************/
void Bl_BlockingDelayMs(uint32_t ms)
{
    uint32_t tickstart = getSysTime();
    while(getSysTime() - tickstart < ms)
    {
#ifdef HAS_IWDG
        // feed the dog when blocking delay
        IWDG_ReloadCounter();
#endif
    }
/*
    uint32_t us= 1000 * ms;
    while(us--)
    {
        asm("nop");
        asm("nop");
    }
*/
}

void BSP_BlockingDelayMs(uint32_t ms)
{
    Bl_BlockingDelayMs(ms);
}

void BSP_SoftReboot(void)
{
    //Disable all interrupt, to avoid interrupt occurs before reset
    __disable_irq();

    //Reset system
    NVIC_SystemReset();
}

bool in_exp= FALSE;
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line)
{
    #define SNPRINTF_BUFFER_MAX_SIZE                (200)
    (void)file;                                   /* avoid compiler warning */
    (void)line;                                   /* avoid compiler warning */ 
    static char buffer[SNPRINTF_BUFFER_MAX_SIZE];

    /* Avoid enter nested exception 
     * When stack overflow, in_exp may be overwrote to large value (not 1 or 0), thus we should not decide
     *   if( !in-exp )
     * the better decision is 
     *   if(in_exp!=TRUE)
     */
    if(in_exp!=TRUE)
    {
        in_exp= TRUE;

        snprintf(buffer,SNPRINTF_BUFFER_MAX_SIZE,"\r\nASSERT:%s,\r\nline: %d\r\n", file, line);
        while(1)
        {      
            /* Avoid watchdog timeout on debug build, to keep print error message */
            UartDrv_Write_Blocking(TP_UART_DEV_1,(uint8 *)buffer,strlen(buffer));//print debug message in blocking mode
            
            Bl_BlockingDelayMs(1000);
        }
    }
}

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
    Bl_ExpHandler("NMI_Handler", 0);
    while (1);
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  Bl_ExpHandler("HardFault_Handler", 0);
  
  /* Go to infinite loop when Hard Fault exception occurs */
  /* when hardFault happen, MCU will stop here, trace the call stack for debug info*/
  while (1);
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
    Bl_ExpHandler("SVC_Handler", 0);
    while (1);
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
    Bl_ExpHandler("PendSV_Handler", 0);
    while (1);
}


/*..........................................................................*/
void Bl_ExpHandler(char const Q_ROM * const Q_ROM_VAR file, int line) 
{
    /* Avoid enter nested exception */
    static bool in_exp= FALSE;
    if(!in_exp)
    {
        in_exp= TRUE;
        printf("Bl_ExpHandler: %s,\r\nline: %d\r\n", file, line);
        BL_error_msg(ERR_MSG_SYSTEM_FAULT);
        bl_jumpAddr(MEMORY_ADDR_ISP);
    }
    
    for (;;) {
    }
}
