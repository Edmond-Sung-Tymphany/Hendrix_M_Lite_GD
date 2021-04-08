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
#include "projBsp.h"
#include "qp_port.h"
#include "object_ids.h"
#include "UartDrv.h"
#include "commonTypes.h"
#include "trace.h"

#if defined(TP_AUDIO_V1) 
#include "deviceTypes_v1.h" 
#else 
#include "deviceTypes_v2.h" 
#endif

#ifdef HAS_DYNAMIC_MEMORY_CONTROL
#include "Setting_id.h"
#include "SettingSrv.h"
#endif

#ifndef BSP_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif


#define INTERNAL_CLOCK  0
#define EXTERNAL_CLOCK 0x04
#define PLL_CLOCK  0x08

static void SetSysClock(void);
static void SystemCoreClockUpdate (void);
//static void BSP_InitDelayTimer(void);
#ifdef ENABLE_STM_READ_PROTECTION
static void SystemEnableStmReadProtection(void);
#endif

/* clock frequency 8M (internal clock) */
uint32_t SystemCoreClock    = 8000000;
__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

//Q_DEFINE_THIS_FILE
#define Sleep() // TODO

volatile static uint32 currTime = 0;

void SysTick_Handler(void);

/* system_sleeping_status*/
static bool isSystemAwake = TRUE;

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
    while (1);
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Execute assert handler and print error message
   */
  Q_onAssert("HardFault_Handler", 0);
  
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
    while (1);
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
    while (1);
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
void SysTick_Handler(void) {                       /* system clock tick ISR */
    QF_TICK_X(0U, (void *)0);              /* process all armed time events */
    ++currTime;
}
/*..........................................................................*/
void BSP_init_clock(void)
{
    /* update the clock value "SystemCoreClock" */
   SystemCoreClockUpdate();
   if (SysTick_Config(SystemCoreClock / 1000))
   { 
      /* Capture error */ 
      while (1);
   }
}
/*..........................................................................*/
void BSP_init(void)
{
   BSP_init_clock();
   //BSP_InitDelayTimer();

#ifdef ENABLE_STM_READ_PROTECTION
    SystemEnableStmReadProtection();
#endif

}
/*..........................................................................*/
void QF_onStartup(void)
{
   
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
void QF_onIdle(void) {      /* entered with interrupts disabled, see NOTE01 */
#ifdef Q_SPY
    if (PIR1bits.TXIF != 0U) {                              /* TXREG empty? */
        uint16_t b = QS_getByte();
        if (b != QS_EOD) {                 /* next QS trace byte available? */
            TXREG = (uint8_t)b; /* stick the byte to TXREG for transmission */
        }
    }
#elif defined NDEBUG
    Sleep();                        /* transition to SLEEP mode, see NOTE02 */
#endif
    QF_INT_ENABLE();                                   /* enable interrupts */
}
/*..........................................................................*/
/* in_exp can not be bool type, becuase bool type will assign to either 1 or 0
 * when stack overflow occurs, it will always treated as TRUE.
 */
static uint32 in_exp= FALSE;

bool BSP_InExp()
{
    /* Avoid enter nested exception 
     * When stack overflow, in_exp may be overwrote to large value (not 1 or 0), thus we should not decide
     *   if( !in-exp )
     * the better decision is 
     *   if(in_exp!=TRUE)
     */
    if(in_exp==TRUE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }    
}


void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
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
    if( !BSP_InExp() )
    {
        in_exp= TRUE;
        __disable_interrupt();

#ifdef PROJECT_SPEC_ASSERT_HANDLER
        ProjBsp_AssertHandler(file, line);
#endif

#ifdef NDEBUG
        //TODO: print error one time
        BSP_SoftReboot();
#else
        tUARTDevice *pConfig = (tUARTDevice *)getDevicebyId(DEBUG_DEV_ID, NULL); /*get the uart ID */
        if(NULL != pConfig)
        {
            snprintf(buffer,SNPRINTF_BUFFER_MAX_SIZE,"\r\nASSERT: %s,\r\nline: %d\r\n", file, line);
            while(1)
            {      
                /* Avoid watchdog timeout on debug build, to keep print error message */
#ifdef HAS_IWDG
                IWDG_ReloadCounter();
#endif
                UartDrv_Write_Blocking(pConfig->uartId,(uint8 *)buffer,strlen(buffer));//print debug message in blocking mode

                BSP_ExpBlockingDelayMs(1000);
            }
        }
#endif
    }
    for (;;) 
    {
#ifdef HAS_IWDG
        IWDG_ReloadCounter();
#endif
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

#define MILLISECONDS_IN_DAY (24*60*60*1000)
#define MILLISECONDS_IN_HOUR   (60*60*1000)
#define MILLISECONDS_IN_MINUTE    (60*1000)
#define MILLISECONDS_IN_SECOND       (1000)

void getSysTimePretty(uint8* day, uint8* hour, uint8* minute, uint8* second, uint16* millisecond)
{
    *day = currTime / MILLISECONDS_IN_DAY;
    *hour = (currTime - (*day * MILLISECONDS_IN_DAY)) / MILLISECONDS_IN_HOUR;
    *minute = (currTime - (*day * MILLISECONDS_IN_DAY) - (*hour * MILLISECONDS_IN_HOUR)) / MILLISECONDS_IN_MINUTE;
    *second = (currTime - (*day * MILLISECONDS_IN_DAY) - (*hour * MILLISECONDS_IN_HOUR) - (*minute * MILLISECONDS_IN_MINUTE)) / MILLISECONDS_IN_SECOND;
    *millisecond = (currTime - (*day * MILLISECONDS_IN_DAY) - (*hour * MILLISECONDS_IN_HOUR) - (*minute * MILLISECONDS_IN_MINUTE) - (*second * MILLISECONDS_IN_SECOND));
}

void BSP_SoftReboot(void)
{
    //Disable all interrupt, to avoid interrupt occurs before reset
    __disable_irq();

    //Reset system
    NVIC_SystemReset();
}


void BSP_BlockingDelayMs(uint32 ms)
{
    uint32_t tickstart = getSysTime();
    while(getSysTime() - tickstart < ms)
    {
#ifdef HAS_IWDG
        // feed the dog when blocking delay
        IWDG_ReloadCounter();
#endif
    }
}


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
//#define TIM3_CLK  1000000  //use 1M, 1us each count
//#define CCR1_VAL  1000     
//
//static void BSP_InitDelayTimer()
//{
//    /* TIM3 clock enable */
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//    
//    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//    TIM_OCInitTypeDef  TIM_OCInitStructure;
//    
//    /* Compute the prescaler value */
//    uint16 PrescalerValue = (uint16) (SystemCoreClock  / TIM3_CLK) - 1;
//
//    /* Time base configuration */
//    TIM_TimeBaseStructure.TIM_Period = 65535;
//    TIM_TimeBaseStructure.TIM_Prescaler = 0;
//    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//
//    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
//
//    /* Prescaler configuration */
//    TIM_PrescalerConfig(TIM3, PrescalerValue, TIM_PSCReloadMode_Immediate);
//
//    /* Output Compare Timing Mode configuration: Channel1 */
//    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
//    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//    TIM_OCInitStructure.TIM_Pulse = CCR1_VAL;
//    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
//
//    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
//
//    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);
//
//    /* TIM Interrupts enable */
//    TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
//
//    /* TIM3 enable counter */
//    TIM_Cmd(TIM3, ENABLE);
//}

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
  RCC->CFGR3 &= (uint32_t)0xFFFFFEAC;

  /* Reset HSI14 bit */
  RCC->CR2 &= (uint32_t)0xFFFFFFFE;

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
  /* if you are using default HSI, you can define an empty function in your projBsp.c*/
  ProjBsp_SysClkUpdate();
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

extern int CSTACK$$Base;            /* symbol created by the IAR linker */
extern int CSTACK$$Limit;
#define STACK_MAGIC_NUM  0x1234ABCD
int stackMagicNumberFill(void)
{
  const char *pStackBase  = (char *)&CSTACK$$Base;
  const char *pStackLimit = (char *)&CSTACK$$Limit;
  const uint32 stackSize  = pStackLimit - pStackBase;
  const int32 sp= __get_MSP();
  const char *pWriteEnd= (char*)( (sp-128)/16*16 ); //we can not fill with sp directly, becuase sp may used by current function
  
  //Assert Check
  ASSERT( pWriteEnd > pStackBase );
  ASSERT( stackSize > 16 ); //Ensure stack have enough size to fill magic number
  ASSERT( stackSize % 16 == 0); //We treat 16bytes as a section to check magic number, thus stack size must be multiple by 16
  
  //Write magic number
  char *p;
  for(p=(char*)pStackBase ; p<=(char*)pWriteEnd ; p+=4)
  {
      *(uint32*)p= STACK_MAGIC_NUM;
  }
  
  //uint32 stack_usage= stackMaxUsage();
  
  return 0;
}


/* return TRUE if stack overflow
 */
bool stackOverflowCheck(void)
{
    char *pStackBase= (char *)&CSTACK$$Base;
    static uint32 buf[4]={STACK_MAGIC_NUM, STACK_MAGIC_NUM, STACK_MAGIC_NUM, STACK_MAGIC_NUM};
    bool rtn= memcmp(buf, pStackBase, 4); //compare 16 bytes on top of stack
    
    /* memcmp() return 0 => compare the same => not overflow
     * memcmp() return non-0 => compare different => overflow
     */
    return rtn;
}


/* return stack size (byte)
 */
uint32 stackSize(void)
{
    const char *pStackBase  = (char *)&CSTACK$$Base;
    const char *pStackLimit = (char *)&CSTACK$$Limit;
    const uint32 stackSize  = pStackLimit - pStackBase;
    return stackSize;
}


/* return stack usage (byte)
 */
uint32 stackMaxUsage(void)
{
  const char *pStackBase  = (char *)&CSTACK$$Base;
  const char *pStackLimit = (char *)&CSTACK$$Limit;
  const char* sp          = (char*)__get_MSP();
  const uint32 stackSize  = (char*)pStackLimit - (char*)pStackBase;
  
  uint32 stack_usage = pStackLimit - sp;
  char *p;
  for(p=(char*)pStackBase ; p<sp ; p+=4)
  {
      if( (*(uint32*)p) != STACK_MAGIC_NUM )
      {
          stack_usage= pStackLimit - p;
          break;
      }
  }
  
  ASSERT(stack_usage <= stackSize);
  return stack_usage;
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
#if defined(EXTERNAL_AND_INTERNAL_HIGH_SPEED_CLOCK)
  RCC->CR |= ((uint32_t)RCC_CR_HSION | (uint32_t)RCC_CR_HSEON);
#elif defined(INTERNAL_HIGH_SPEED_CLOCK)
  RCC->CR |= ((uint32_t)RCC_CR_HSION);
#elif defined(EXTERNAL_HIGH_SPEED_CLOCK)
  RCC->CR |= ((uint32_t)RCC_CR_HSEON);
#endif

}

#ifdef ENABLE_STM_READ_PROTECTION
/**
  * @brief  check the read protection option byte and set the read protection to LEVEL_1
  * @note:  don't enable it in debug mode, otherwise you have to use
  *         ST-LINK Utility to write the read protection byte to LEVEL_0.
  * @param  None
  * @retval None
  */
static void SystemEnableStmReadProtection(void)
{
    FlagStatus rdp_set; 
    uint8_t ob_rdp;

    rdp_set = FLASH_OB_GetRDP();

    if( ! rdp_set )
    {
        ob_rdp = OB_RDP_Level_1;

        FLASH_Unlock();
        FLASH_OB_Unlock();

        //FLASH_OB_WRPConfig();
        FLASH_OB_RDPConfig(ob_rdp);
        //FLASH_OB_UserConfig();
        //FLASH_OB_BORConfig();

        FLASH_OB_Launch();
        FLASH_OB_Lock();
        FLASH_Lock();
    }
}
#endif


/*--------------------------------------------------------------------------*/
#ifdef Q_SPY
#define QS_BUF_SIZE        200U
#define BAUD_RATE          19200U
QSTimeCtr BSP_tickTime;
uint8_t QS_onStartup(void const *arg) {
   
}
/*..........................................................................*/
void QS_onCleanup(void) {
}
/*..........................................................................*/
void QS_onFlush(void) {
}
/*..........................................................................*/
/* NOTE: invoked within a critical section (inetrrupts disabled) */
QSTimeCtr QS_onGetTime(void) {
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

void DynamicAnalysis()
{
#ifdef HAS_DYNAMIC_MEMORY_CONTROL
    const uint32 margin_percent_warning= HAS_DYNAMIC_MEMORY_CONTROL_WARNING_PERCENTAGE;
    uint16 margin;
    uint32 margin_percent;
    //uint8  day;
    //uint8  hour;
    //uint8  minute;
    //uint8  second;
    //uint16 millisecond;
    TP_PRINTF("\r\n\r\n");
    TP_PRINTF("===DynamicAnalysis===Queues=======Remain======\r\n");
#ifdef HAS_DEBUG
    margin = QF_getQueueMin(DEBUG_SRV_ID);
    margin_percent= margin*100/DBG_SRV_EVENT_Q_SIZE;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_QUEUE_MIN_DEBUG_SRV) > (uint32)margin)
    {
        Setting_Set(SETID_QUEUE_MIN_DEBUG_SRV, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| DEBUG_SRV_ID      [%3d][%3d]   %3d%%  %s\r\n", margin,  DBG_SRV_EVENT_Q_SIZE,  margin*100/DBG_SRV_EVENT_Q_SIZE, (margin_percent<=margin_percent_warning)?"==>WARNING":"");
#endif
#if defined(HAS_SETTING) && defined(SETTING_HAS_ROM_DATA)
    margin = QF_getQueueMin(SETTING_SRV_ID);
    margin_percent= margin*100/SETT_SRV_EVENT_Q_SIZE;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_QUEUE_MIN_SETTING_SRV) > (uint32)margin)
    {
        Setting_Set(SETID_QUEUE_MIN_SETTING_SRV, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| SETTING_SRV_ID    [%3d][%3d]   %3d%%  %s\r\n", margin, SETT_SRV_EVENT_Q_SIZE,  margin*100/SETT_SRV_EVENT_Q_SIZE, (margin_percent<=margin_percent_warning)?"==>WARNING":"");
#endif
#ifdef HAS_AUDIO_CONTROL
    margin = QF_getQueueMin(AUDIO_SRV_ID);
    margin_percent= margin*100/AUDIO_SRV_EVENT_Q_SIZE;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_QUEUE_MIN_AUDIO_SRV) > (uint32)margin)
    {
        Setting_Set(SETID_QUEUE_MIN_AUDIO_SRV, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| AUDIO_SRV_ID      [%3d][%3d]   %3d%%  %s\r\n", margin, AUDIO_SRV_EVENT_Q_SIZE,  margin*100/AUDIO_SRV_EVENT_Q_SIZE, (margin_percent<=margin_percent_warning)?"==>WARNING":"");
#endif
#ifdef HAS_LEDS
    margin = QF_getQueueMin(LED_SRV_ID);
    margin_percent= margin*100/LED_SRV_EVENT_Q_SIZE;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_QUEUE_MIN_LEDS_SRV) > (uint32)margin)
    {
        Setting_Set(SETID_QUEUE_MIN_LEDS_SRV, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| LED_SRV_ID        [%3d][%3d]   %3d%%  %s\r\n", margin, LED_SRV_EVENT_Q_SIZE,  margin*100/LED_SRV_EVENT_Q_SIZE, (margin_percent<=margin_percent_warning)?"==>WARNING":"");
#endif
#ifdef HAS_BLUETOOTH
    margin = QF_getQueueMin(BT_SRV_ID);
    margin_percent= margin*100/KEY_SRV_EVENT_Q_SIZE;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_QUEUE_MIN_BLUETOOTH_SRV) > (uint32)margin)
    {
        Setting_Set(SETID_QUEUE_MIN_BLUETOOTH_SRV, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| BT_SRV_ID         [%3d][%3d]   %3d%%  %s\r\n", margin, BT_SRV_EVENT_Q_SIZE);
#endif
#ifdef HAS_KEYS
    margin = QF_getQueueMin(KEY_SRV_ID);
    if (*(uint32*)Setting_Get(SETID_QUEUE_MIN_KEYS_SRV) > (uint32)margin)
    {
        Setting_Set(SETID_QUEUE_MIN_KEYS_SRV, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| KEY_SRV_ID        [%3d][%3d]   %3d%%  %s\r\n", margin, KEY_SRV_EVENT_Q_SIZE,  margin*100/KEY_SRV_EVENT_Q_SIZE, (margin_percent<=margin_percent_warning)?"==>WARNING":"");
#endif
#ifdef HAS_POWER_CONTROL
    margin = QF_getQueueMin(POWER_SRV_ID);
    margin_percent= margin*100/POWER_SRV_EVENT_Q_SIZE;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_QUEUE_MIN_POWER_SRV) > (uint32)margin)
    {
        Setting_Set(SETID_QUEUE_MIN_POWER_SRV, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| POWER_SRV_ID      [%3d][%3d]   %3d%%  %s\r\n", margin, POWER_SRV_EVENT_Q_SIZE,  margin*100/POWER_SRV_EVENT_Q_SIZE, (margin_percent<=margin_percent_warning)?"==>WARNING":"");
#endif
#if defined(HAS_ASE_TK) || defined(HAS_ASE_NG)
    margin = QF_getQueueMin(ASETK_SRV_ID);
    margin_percent= margin*100/ASETK_SRV_EVENT_Q_SIZE;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_QUEUE_MIN_ASE_TK_SRV) > (uint32)margin)
    {
        Setting_Set(SETID_QUEUE_MIN_ASE_TK_SRV, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| ASETK_SRV_ID      [%3d][%3d]   %3d%%  %s\r\n", margin, ASETK_SRV_EVENT_Q_SIZE,  margin*100/ASETK_SRV_EVENT_Q_SIZE, (margin_percent<=margin_percent_warning)?"==>WARNING":"");
#endif
    margin = QF_getQueueMin(MAIN_APP_ID);
    margin_percent= margin*100/MAINAPP_EVENT_Q_SIZE;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_QUEUE_MIN_DEBUG_SRV) > (uint32)margin)
    {
        Setting_Set(SETID_QUEUE_MIN_DEBUG_SRV, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| MAIN_APP_ID       [%3d][%3d]   %3d%%  %s\r\n", margin, MAINAPP_EVENT_Q_SIZE,  margin*100/MAINAPP_EVENT_Q_SIZE, (margin_percent<=margin_percent_warning)?"==>WARNING":"");

    TP_PRINTF("===DynamicAnalysis===Pools========Remain======\r\n");
    margin = QF_getPoolMin(1);
    margin_percent= margin*100/NUM_OF_SMALL_EVENTS;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_POOL_MIN_SMALL) > (uint32)margin)
    {
        Setting_Set(SETID_POOL_MIN_SMALL, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| PollSmall         [%3d][%3d]   %3d%%  %s\r\n", margin, NUM_OF_SMALL_EVENTS,  margin*100/NUM_OF_SMALL_EVENTS, (margin_percent<=margin_percent_warning)?"==>WARNING":"");

    margin = QF_getPoolMin(2);
    margin_percent= margin*100/NUM_OF_MEDIUM_EVENTS;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_POOL_MIN_MEDIUM) > (uint32)margin)
    {
        Setting_Set(SETID_POOL_MIN_MEDIUM, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| PollMedium        [%3d][%3d]   %3d%%  %s\r\n", margin, NUM_OF_MEDIUM_EVENTS,  margin*100/NUM_OF_MEDIUM_EVENTS, (margin_percent<=margin_percent_warning)?"==>WARNING":"");

    margin = QF_getPoolMin(3);
    margin_percent= margin*100/NUM_OF_LARGE_EVENTS;
    ASSERT(margin_percent>margin_percent_warning); //early assert on debug build, the remind size is near empty
    if (*(uint32*)Setting_Get(SETID_POOL_MIN_LARGE) > (uint32)margin)
    {
        Setting_Set(SETID_POOL_MIN_LARGE, &margin);
    }
    ASSERT(margin > 0);
    TP_PRINTF("|| PollLarge         [%3d][%3d]   %3d%%  %s\r\n", margin, NUM_OF_LARGE_EVENTS,  margin*100/NUM_OF_LARGE_EVENTS, (margin_percent<=margin_percent_warning)?"==>WARNING":"");
    TP_PRINTF("==============================================\r\n\r\n");
    //getSysTimePretty(&day,&hour,&minute,&second,&millisecond);
    //TP_PRINTF("Time [%d:%02d:%02d:%02d.%03d]\r\n", day, hour, minute, second, millisecond);
#endif
}

/*
    t_IWDG = t_LSI(ms) * 4 * (IWDG_RLR[11:0]+1) * 2 ^ (IWDG_PR[2:0]) = [125 us..32.8s]

    Example:

    LSI clock 40kHz => t_LSI(ms) = 1/40000 = 0.025
    reload = 0x0FFF => (IWDG_RLR[11:0]+1)  = 4096
    prescale = 6    => 2 ^ (IWDG_PR[2:0])  = 64

    t_IWDG = 26.2s
*/
void IwdgInit(uint8_t prescaler, uint16_t reload)
{
#ifdef HAS_IWDG
    IWDG_Enable();
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(prescaler);
    IWDG_SetReload(reload);
    while (SET == IWDG_GetFlagStatus(IWDG_FLAG_PVU));
    while (SET == IWDG_GetFlagStatus(IWDG_FLAG_RVU));
    IWDG_ReloadCounter();

    /* Pause WDOG when use IAR to online debug */
    DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);
#endif
}
void IwdgReloadActiveMode()
{
#ifdef HAS_IWDG
    IWDG_ReloadCounter();
#endif
}
void IwdgReloadOffMode()
{
#ifdef HAS_IWDG
    IWDG_ReloadCounter();
#endif
}

