/*****************************************************************************
*  @file      main.c
*  @brief     Contains main and is a simple entry point into the SW
*  @author    Christopher Alexander
*  @date      11-Nov-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/
#include "stm32f0xx.h"
#include "controller.h"
#include "bsp.h"
#include "product.config"
#include "fep_addr.h"

#if   (defined ( __CC_ARM ))
  __IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))
#pragma location = 0x20000000
  __no_init __IO uint32_t VectorTable[48];
#elif defined   (  __GNUC__  )
  __IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#elif defined ( __TASKING__ )
  __IO uint32_t VectorTable[48] __at(0x20000000);
#endif

/*..........................................................................*/

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    uint32 i = 0;

    /*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f0xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f0xx.c file
     */

    /* Relocate by software the vector table to the internal SRAM at 0x20000000 ***/

    /* Copy the vector table from the Flash (mapped at the base of the application
     load address 0x08003000) to the base address of the SRAM at 0x20000000. */
    for(i = 0; i < 48; i++)
    {
        VectorTable[i] = *(__IO uint32_t*)(FEP_ADDR_FIRMWARE + (i<<2));
    }

    /* Enable the SYSCFG peripheral clock*/
    /* Note original STM32 bootlaoder sample call RCC_APB2PeriphResetCmd() to start SYSCFG
     * But it is a bug, RCC_APB2PeriphResetCmd() disable SYSCFG, and EXTI have problem.
     */
    //RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Remap SRAM at 0x00000000 */
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);

    /* let 'er rip */
    BSP_init();

    return Controller_Ctor(NORMAL_MODE);
}
