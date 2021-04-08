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
#include "stm32f10x.h"
#include "bsp.h"
#include "projBsp.h"
#include "qp_port.h"

#include "commonTypes.h"
#include "trace.h"

#ifndef BSP_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif


#define INTERNAL_CLOCK  0
#define EXTERNAL_CLOCK 0x04
#define PLL_CLOCK  0x08

static void SetSysClock(void);
static void SystemCoreClockUpdate (void);
#ifdef ENABLE_ACCURATE_DELAY
static void BSP_InitAccurateTimer(void);
#endif

/* clock frequency 8M (internal clock) */
uint32_t SystemCoreClock    = 8000000;
__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

//Q_DEFINE_THIS_FILE
#define Sleep() // TODO

volatile static uint32 currTime = 0;

/* system_sleeping_status*/
static bool isSystemAwake = TRUE;


/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
static void delay(uint32 time)
{
    volatile uint8 i = ~0;
    while(time>0)
    {
        while((i--)>0);
        i = ~0;
        time--;
    }
}

/**
  * @brief  This function handles NMI exception (Non maskable interrupt)
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
    while(1)
    {
        ProjBsp_CyclePrintError("NMI_Handler\r\n");
        delay(1000);
    }
}

/**
  * @brief  This function handles Hard Fault exception, (All class of fault)
  * @param  None
  * @retval None
  */
void HardFault_Handler(unsigned int * hardfault_args)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    /* when hardFault happen, MCU will stop here, trace the call stack for debug info*/

    char g_cDataBuf[50];
    unsigned int stacked_r0,stacked_r1,stacked_r2,stacked_r3;
    unsigned int stacked_r12,stacked_lr, stacked_pc, stacked_psr;
    stacked_r0 = ((unsigned long) hardfault_args[0]);
    stacked_r1 = ((unsigned long) hardfault_args[1]);
    stacked_r2 = ((unsigned long) hardfault_args[2]);
    stacked_r3 = ((unsigned long) hardfault_args[3]);
    stacked_r12 = ((unsigned long) hardfault_args[4]);
    stacked_lr = ((unsigned long) hardfault_args[5]);
    stacked_pc = ((unsigned long) hardfault_args[6]);
    stacked_psr = ((unsigned long) hardfault_args[7]);
    while(1)
    { /* print out the CPU registers to help tracing the problem */
        sprintf(g_cDataBuf,"[Hard fault handler]\r\n");
        ProjBsp_CyclePrintError(g_cDataBuf);
        sprintf(g_cDataBuf,"SP = 0x%x\r\n", hardfault_args);   //¶ÑÕ»µØÖ·
        ProjBsp_CyclePrintError(g_cDataBuf);
        sprintf(g_cDataBuf,"R0 = 0x%x\r\n", stacked_r0);
        ProjBsp_CyclePrintError(g_cDataBuf);
        sprintf(g_cDataBuf,"R1 = 0x%x\r\n", stacked_r1);
        ProjBsp_CyclePrintError(g_cDataBuf);
        sprintf(g_cDataBuf,"R2 = 0x%x\r\n", stacked_r2);
        ProjBsp_CyclePrintError(g_cDataBuf);
        sprintf(g_cDataBuf,"R3 = 0x%x\r\n", stacked_r3);
        ProjBsp_CyclePrintError(g_cDataBuf);
        sprintf(g_cDataBuf,"R12 = 0x%x\r\n", stacked_r12);
        ProjBsp_CyclePrintError(g_cDataBuf);
        sprintf(g_cDataBuf,"LR = 0x%x\r\n", stacked_lr);
        ProjBsp_CyclePrintError(g_cDataBuf);
        sprintf(g_cDataBuf,"PC = 0x%x\r\n", stacked_pc);
        ProjBsp_CyclePrintError(g_cDataBuf);
        sprintf(g_cDataBuf,"PSR = 0x%x\r\n", stacked_psr);
        ProjBsp_CyclePrintError(g_cDataBuf);
        delay(5000);
    }
}

/**
  * @brief  This function handles MemManage exception. (Memory management)
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    /* when hardFault happen, MCU will stop here, trace the call stack for debug info*/
    while (1);
}

/**
  * @brief  This function handles Bus Fault exception. (Pre-fetch fault, memory access fault)
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    /* when hardFault happen, MCU will stop here, trace the call stack for debug info*/
    while(1)
    {
        ProjBsp_CyclePrintError("BusFault\r\n");
        delay(1000);
    }
}

/**
  * @brief  This function handles Usage Fault exception. (Undefined instruction or illegal state)
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    /* when hardFault happen, MCU will stop here, trace the call stack for debug info*/
    while(1)
    {
        ProjBsp_CyclePrintError("UsageFault\r\n");
        delay(1000);
    }
}


/**
  * @brief  This function handles SVCall exception. (System service call via SWI instruction)
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
    while (1);
}

/**
  * @brief  This function handles DebugMon exception. (Debug monitor)
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
    while (1);
}


/**
  * @brief  This function handles PendSVC exception. (Pendable request for system)
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
    while (1);
}

/**
  * @brief  This function handles system tick timer interrupt.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)                         /* system clock tick ISR */
{
    QF_TICK_X(0U, (void *)0);              /* process all armed time events */
    ++currTime;
}


/**
* Public function to check if the system is in deep sleep status
* @return[in]   TRUE/FALSE      Sleep or wakedup
*/
bool BSP_IsSystemAwake()
{
    return (isSystemAwake ? TRUE : FALSE);
}

void BSP_SetSystemAwake(bool awake)
{
    isSystemAwake = awake;
}
/* End of system_sleeping_status */

/*..........................................................................*/
void BSP_init(void)
{
    /* update the clock value "SystemCoreClock" */
    SystemCoreClockUpdate();
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        /* Capture error */
        while (1);
    }
#ifdef ENABLE_ACCURATE_DELAY
    BSP_InitAccurateTimer();
#endif
}
/*..........................................................................*/
void QF_onStartup(void)
{

}
/*..........................................................................*/
void QF_onCleanup(void)
{
}
/*..........................................................................*/
void QF_onIdle(void)        /* entered with interrupts disabled, see NOTE01 */
{
#ifdef Q_SPY
    if (PIR1bits.TXIF != 0U)                                /* TXREG empty? */
    {
        uint16_t b = QS_getByte();
        if (b != QS_EOD)                   /* next QS trace byte available? */
        {
            TXREG = (uint8_t)b; /* stick the byte to TXREG for transmission */
        }
    }
#elif defined NDEBUG
    Sleep();                        /* transition to SLEEP mode, see NOTE02 */
#endif
    QF_INT_ENABLE();                                   /* enable interrupts */
}
/*..........................................................................*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line)
{
    (void)file;                                   /* avoid compiler warning */
    (void)line;                                   /* avoid compiler warning */
    TP_PRINTF("assert is happend,\r\nthe file is (%s),\r\nline is (%d)\r\n", file, line);
    QF_INT_DISABLE();                  /* make sure interrupts are disabled */
    for (;;)
    {
    }
}

/*..........................................................................*/
/* assert function for stm library,          */
/* add Macro "USE_FULL_ASSERT" to turn it on */
/*..........................................................................*/
void assert_failed(uint8_t* file, uint32_t line)
{
    Q_onAssert((char*)file,(int)line);
}

/**
* @brief Get the tick elapsed
* @return   uint32  the tick elapsed
*/
uint32 getSysTime()
{
    return currTime;
}




void BSP_BlockingDelayUs(uint16 us)
{
#ifdef ENABLE_ACCURATE_DELAY
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
    volatile uint16 capture = TIM_GetCapture1(TIM3);
    TIM_SetCompare1(TIM3, capture + us);
    while(TIM_GetITStatus(TIM3, TIM_IT_CC1) == RESET);
#else
    while(us--)
    {
        asm("nop");
        asm("nop");
    }
#endif
}

#ifdef ENABLE_ACCURATE_DELAY

/* -----------------------------------------------------------------------
    TIM3 Configuration: Output Compare Timing Mode:

    In this example TIM3 input clock (TIM3CLK) is set to APB1 clock (PCLK1),
      => TIM3CLK = PCLK1 = SystemCoreClock = 1 MHz

    To get TIM3 counter clock at 1 MHz, the prescaler is computed as follows:
       Prescaler = (TIM3CLK / TIM3 counter clock) - 1
       Prescaler = (PCLK1 /1 MHz) - 1

    CC1 update rate = TIM3 counter clock / CCR1_Val = 1000  Hz

    Note:
     SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f0xx.c file.
     Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
     function to update SystemCoreClock variable value. Otherwise, any configuration
     based on this variable will be incorrect.
  ----------------------------------------------------------------------- */
/* no need to change these Macro */
#define TIM3_CLK  1000000  //use 1M, 1us each count
#define CCR1_VAL  1000

static void BSP_InitAccurateTimer()
{
    /* TIM3 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    /* Compute the prescaler value */
    uint16 PrescalerValue = (uint16) (SystemCoreClock  / TIM3_CLK) - 1;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /* Prescaler configuration */
    TIM_PrescalerConfig(TIM3, PrescalerValue, TIM_PSCReloadMode_Immediate);

    /* Output Compare Timing Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = CCR1_VAL;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM3, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);

    /* TIM Interrupts enable */
    TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

    /* TIM3 enable counter */
    TIM_Cmd(TIM3, ENABLE);
}

#endif

/***************************************************************************
*
* watchdog place holder
*
****************************************************************************/

/* Watchdog */
void BSP_EnableWatchdog(void)
{
}

void BSP_DisableWatchdog(void)
{
}

void BSP_FeedWatchdog(void)
{
}


/***************************************************************************
*
* Clock setting for STM32, move here from STM library.
*
****************************************************************************/
/**
  * @brief  Setup the microcontroller system.
  *         Initialize the Embedded Flash Interface, the PLL and update the
  *         SystemCoreClock variable.
  * @param  None
  * @retval None
  */
void SystemInit (void)
{
    /*need to clean up the magic number below later*/
    /* Set HSION bit */
    RCC->CR |= (uint32_t)0x00000001;

    /* Reset SW[1:0], HPRE[3:0], PPRE[2:0], ADCPRE and MCOSEL[2:0] bits */
    RCC->CFGR &= (uint32_t)0xF8FFB80C;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Reset PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
    RCC->CFGR &= (uint32_t)0xFFC0FFFF;

    /* Reset PREDIV1[3:0] bits */
    RCC->CFGR2 &= (uint32_t)0xFFFFFFF0;

    /* Reset USARTSW[1:0], I2CSW, CECSW and ADCSW bits */
    //RCC->CFGR3 &= (uint32_t)0xFFFFFEAC;

    /* Reset HSI14 bit */
    //RCC->CR2 &= (uint32_t)0xFFFFFFFE;

    /* Disable all interrupts */
    RCC->CIR = 0x00000000;

    /* Configure the System clock frequency, AHB/APBx prescalers and Flash settings */
    SetSysClock();
}

/**
  * @brief  Update SystemCoreClock according to Clock Register Values
  *         The SystemCoreClock variable contains the core clock (HCLK), it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *
  * @note   Each time the core clock (HCLK) changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.
  *
  * @note   - The system frequency computed by this function is not the real
  *           frequency in the chip. It is calculated based on the predefined
  *           constant and the selected clock source:
  *
  *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(*)
  *
  *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(**)
  *
  *           - If SYSCLK source is PLL, SystemCoreClock will contain the HSE_VALUE(**)
  *             or HSI_VALUE(*) multiplied/divided by the PLL factors.
  *
  *         (*) HSI_VALUE is a constant defined in stm32f0xx.h file (default value
  *             8 MHz) but the real value may vary depending on the variations
  *             in voltage and temperature.
  *
  *         (**) HSE_VALUE is a constant defined in stm32f0xx.h file (default value
  *              8 MHz), user has to ensure that HSE_VALUE is same as the real
  *              frequency of the crystal used. Otherwise, this function may
  *              have wrong result.
  *
  *         - The result of this function could be not correct when using fractional
  *           value for HSE crystal.
  * @param  None
  * @retval None
  */

static void SystemCoreClockUpdate (void)
{
    uint32_t tmp = 0, pllmull = 0, pllsource = 0, prediv1factor = 0;

    /* Get SYSCLK source -------------------------------------------------------*/
    tmp = RCC->CFGR & RCC_CFGR_SWS;

    switch (tmp)
    {
        case INTERNAL_CLOCK:  /* HSI used as system clock */
            SystemCoreClock = HSI_VALUE;
            break;
        case EXTERNAL_CLOCK:  /* HSE used as system clock */
            SystemCoreClock = HSE_VALUE;
            break;
        case PLL_CLOCK:  /* PLL used as system clock */
            /* Get PLL clock source and multiplication factor ----------------------*/
            pllmull = RCC->CFGR & RCC_CFGR_PLLMULL;
            pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
            pllmull = ( pllmull >> 18) + 2;

            if (pllsource == 0x00)
            {
                /* HSI oscillator clock divided by 2 selected as PLL clock entry */
                SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
            }
            else
            {
                prediv1factor = (RCC->CFGR2 & RCC_CFGR2_PREDIV1) + 1;
                /* HSE oscillator clock selected as PREDIV1 clock entry */
                SystemCoreClock = (HSE_VALUE / prediv1factor) * pllmull;
            }
            break;
        default: /* HSI used as system clock */
            SystemCoreClock = HSI_VALUE;
            break;
    }
    /* Compute HCLK clock frequency ----------------*/
    /* Get HCLK prescaler */
    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    /* HCLK clock frequency */
    SystemCoreClock >>= tmp;
}

/**
  * @brief  Configures the System clock frequency, AHB/APBx prescalers and Flash
  *         settings.
  * @note   This function should be called only once the RCC clock configuration
  *         is reset to the default reset state (done in SystemInit() function).
  * @param  None
  * @retval None
  */
static void SetSysClock(void)
{
    __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

    /******************************************************************************/
    /*            PLL (clocked by HSE) used as System clock source                */
    /******************************************************************************/

    /* SYSCLK, HCLK, PCLK configuration ----------------------------------------*/
    /* Enable HSE */
#if defined(EXTERNAL_HIGH_SPEED_CLOCK)
    RCC->CR |= ((uint32_t)RCC_CR_HSION | (uint32_t)RCC_CR_HSEON);
#elif defined(EXTERNAL_AND_INTERNAL_HIGH_SPEED_CLOCK)
    RCC->CR |= ((uint32_t)RCC_CR_HSION);
#endif
}

/*--------------------------------------------------------------------------*/
#ifdef Q_SPY
#define QS_BUF_SIZE        200U
#define BAUD_RATE          19200U
QSTimeCtr BSP_tickTime;
uint8_t QS_onStartup(void const *arg)
{

}
/*..........................................................................*/
void QS_onCleanup(void)
{
}
/*..........................................................................*/
void QS_onFlush(void)
{
}
/*..........................................................................*/
/* NOTE: invoked within a critical section (inetrrupts disabled) */
QSTimeCtr QS_onGetTime(void)
{
}
#endif                                                             /* Q_SPY */
/*--------------------------------------------------------------------------*/
/*****************************************************************************
* NOTE01:
* The callback function QF_onIdle() is called with interrupts disabled,
* because the idle condition can be invalidated by any enabled interrupt that
* would post events. The QF_onIdle() function *must* enable interrupts
* internally
*
* NOTE02:
* The low-power mode must be entered with interrupts disabled to avoid
* an interrupt to preempt the transition to the low-power mode. According to
* the "PIC18FXX2 Data Sheet", the Sleep mode can be exited even if the global
* inetrrupt enable flag is cleared (INTCON<7> == 0). This allows for
* an atomic transition to the SLEEP mode.
*
* Selected interrupts, such as Timer1 interrupt with external clock (not
* synchronized to the CPU clock) can wake up the CPU from the SLEEP mode.
* after waking up, the CPU does not service the interrupt immediately,
* because interrupts are still disabled. The interrupt gets serviced only
* after the CPU unlocks interrupts.
*/
