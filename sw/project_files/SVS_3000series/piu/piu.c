/**
  ******************************************************************************
  * @file    app.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-January-2014
  * @brief   This file provides all the Application firmware functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "commonTypes.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_syscfg.h"
#include "bsp.h"
#include "piu_common.h"

/* Private define ------------------------------------------------------------*/
static const uint8 PRODUCT_NAME[16]@"PRODUCT_NAME" = {TP_PRODUCT};
static const uint8 JUMPER_VER[16]@"JUMPER_VER" = {"v1.0.0"};

/* Private macro -------------------------------------------------------------*/

/**
  * @brief  Program entry point
  * @param  None
  * @retval None
  */
int main(void)
{
    uint32 bootMode;

    (void)PRODUCT_NAME;
    (void)JUMPER_VER;

    /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f072.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f0xx.c file
      */
    remappingVectorTable(ADDR_BOOTLOADER_MEMORY);

    /* intialize BSP*/
    BSP_init();

    /* config amplifier */
    amplifier_Initial();

    /* Get the Setting sections */
    bootMode = *(__IO uint32_t*)ADDR_BOOT_MODE;

    if(bootMode == BOOT_MODE_APP)
    {
        jump2Address(ADDR_APP_MEMORY);
    }
    else
    {
        /*set leds*/
        ioExpander_Initial();
        ioExpander_SetLeds(LED_MASK_LED_UPGRADING, TRUE);

        jump2Address(ADDR_SYSTEM_MEMORY);
    }

}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

