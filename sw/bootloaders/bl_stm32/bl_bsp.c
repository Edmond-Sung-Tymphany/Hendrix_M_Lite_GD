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
#include "bsp.h"
#include "bl_bsp.h"
#include "commonTypes.h"
#include "bl_common.h"
#include "trace.h"
#include "tplog.h"
#include "fep_addr.h"

/******************************************************************************/
/*            Definition                                                      */
/******************************************************************************/
/* When jump from STM32-BL to PIU/UBL, system tick is no longer work, and never leave 
 * BSP_BlockingDelayMs(). Thus we disable accurary currently, and will enable when fix
 * this bug */
//#define BL_ACCURARY_DELAY



/******************************************************************************/
/*            Functions Prototype                                             */
/******************************************************************************/
void Bl_ExpHandler(char const Q_ROM * const Q_ROM_VAR file, int line);



/******************************************************************************/
/*            Functions Implemenation                                         */
/******************************************************************************/
bool BSP_InExp(void)
{
    return FALSE;
}


void BSP_ExpBlockingDelayMs(uint32 ms)
{
    BSP_BlockingDelayMs(ms);
}

        
void BSP_BlockingDelayMs(uint32 ms)
{
#ifdef BL_ACCURARY_DELAY  
    uint32_t tickstart = getSysTime();
    while(getSysTime() - tickstart < ms)
    {
    }
#else    
    uint32 us= 1000 * ms;
    while(us--)
    {
        asm("nop");
        asm("nop");
    }
#endif    
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
        TPLOG_FATAL("Bl_ExpHandler: %s,\r\nline: %d\r\n", file, line);
        bl_jumpAddr(FEP_ADDR_ISP);
    }
    
    for (;;) {
    }
}
