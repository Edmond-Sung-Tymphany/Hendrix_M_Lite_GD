/**
  ******************************************************************************
  * @file    gd32f1x0_conf.h 
  * @author  
  * @version V0.1.0
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GD32f1x0_CONF_H
#define __GD32f1x0_CONF_H

/* Includes ------------------------------------------------------------------*/
/* Comment the line below to disable peripheral header file inclusion */
#include "gd32f1x0_adc.h"
#include "gd32f1x0_cec.h"
#include "gd32f1x0_crc.h"
#include "gd32f1x0_comp.h"
#include "gd32f1x0_dac.h"
#include "gd32f1x0_dbgmcu.h"
#include "gd32f1x0_dma.h"
#include "gd32f1x0_exti.h"
#include "gd32f1x0_flash.h"
#include "gd32f1x0_gpio.h"
#include "gd32f1x0_syscfg.h"
#include "gd32f1x0_i2c.h"
#include "gd32f1x0_iwdg.h"
#include "gd32f1x0_pwr.h"
#include "gd32f1x0_rcc.h"
#include "gd32f1x0_rtc.h"	
#include "gd32f1x0_spi.h"
#include "gd32f1x0_tim.h"
#include "gd32f1x0_usart.h"
#include "gd32f1x0_wwdg.h"
#include "gd32f1x0_misc.h"  /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line below to expanse the "assert_param" macro in the 
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function which reports 
  *         the name of the source file and the source line number of the call 
  *         that failed. If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

#endif /* __GD32f1x0_CONF_H */

/************************ (C) COPYRIGHT*****END OF FILE****/
