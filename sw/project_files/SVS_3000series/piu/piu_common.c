#include "stm32f0xx.h"
#include "commonTypes.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_syscfg.h"
#include "piu_common.h"
#include "attachedDevices.h"
#include "GpioDrv.h"
#include "IoExpanderDrv.h"
#include "piu_common.h"
#include "IoExpanderLedDrv.config"

static cGpioDrv audioGpioDrv;
static cIoExpanderDrv ioeDrv;
static tIoeLedDevice *pIoeLedConfig;
static cGpioDrv    powerGpioDrv;

#define LED_BRIGHTNESS_100  (15)    // should be same as MainApp_util.h

typedef  void (*pFunction)(void);

#if   (defined ( __CC_ARM ))
  __IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))
#pragma location = 0x20000000
  __IO uint32_t VectorTable[48];
#elif defined   (  __GNUC__  )
  __IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#elif defined ( __TASKING__ )
  __IO uint32_t VectorTable[48] __at(0x20000000);
#endif

void remappingVectorTable(uint32_t address)
{
    uint32_t i;

    if( (*(__IO uint32_t*)(address) & 0x2FFE0000) != ADDR_SRAM_MEMORY)
        return ;

    /* Copy the vector table from the Flash (mapped at the base of the application
         load address 0x08003000) to the base address of the SRAM at 0x20000000. */

    /* that is a very hide problem, we need to pay attetion
       1, once reset, the PC register will point to the 0x0800000, the first 48
          words in the internal FlASH are vector table.
       2, but this vector table will not be place at the SDRAM(0x2000000),
          that is address 0x20000000 stores other things, not the vector table,
          example, see map.file, you can see this address stores controller.o
       3, if we use follow statement, it will change the the SRAM content,
          example, it will change the content in controller.o. it will crash.
    */

    /* wrong operation, but please keep it here to reminder.
    for(i = 0; i < 48; i++)
    {
        *(__IO uint32_t*)(ADDR_SRAM_MEMORY + ((uint32_t)i << 2)) = *(__IO uint32_t*)(address + ( (uint32_t)i << 2));
    }
    */

    for(i = 0; i < 48; i++)
    {
      VectorTable[i] = *(__IO uint32_t*)(address + (i << 2));
    }

    /* Enable the SYSCFG peripheral clock*/
    /* Note original STM32 bootlaoder sample call RCC_APB2PeriphResetCmd() to start SYSCFG
     * But it is a bug, RCC_APB2PeriphResetCmd() disable SYSCFG, and EXTI have problem.
     */
    //RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Remap SRAM at 0x00000000 */
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);

}

/**
  * @brief  bootloader jump function
  * @param  None
  * @retval None
  */
void jump2Address(uint32_t address)
{
    pFunction jump2function;
    uint32_t jump2addr;

    if((*(__IO uint32_t*)(address) & 0x2FFE0000) != ADDR_SRAM_MEMORY)
        return;

    RCC_DeInit();
    USART_DeInit(USART1);
    USART_DeInit(USART2);

    SysTick->CTRL   = 0;  // reset the Systick Timer
    SysTick->LOAD   = 0;
    SysTick->VAL    = 0;

    /*
        Don't use follow sentence, it will stop the USB interrupt after entering USB DFU process,
        its driver will install fail.
    */

    //__set_PRIMASK(1);	// Disable interrupts
    //__disable_irq();

    /* jump to the point address, but first word stores stack pointer,
       second word stores the vector table, and the first one of the vector table
       is reset vector.
     */
    jump2addr = *(__IO uint32_t*)(address + 4);
    jump2function = (pFunction)jump2addr;

    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) address);

    /*start jumping*/
    jump2function();
}

void amplifier_Initial(void)
{
    audioGpioDrv.gpioConfig = (tGPIODevice*)getDevicebyIdAndType(AUDIO_DEV_ID, GPIO_DEV_TYPE, NULL);
    GpioDrv_Ctor(&audioGpioDrv, audioGpioDrv.gpioConfig);

    /* mute and shutdown amplifier */
    GpioDrv_SetBit(&audioGpioDrv, GPIO_OUT_AMP_MUTE);
    GpioDrv_ClearBit(&audioGpioDrv, GPIO_OUT_AMP_STDBY);

    /* Close some power control pin*/
    powerGpioDrv.gpioConfig = (tGPIODevice*)getDevicebyIdAndType(POWER_DEV_ID, GPIO_DEV_TYPE, NULL);
    GpioDrv_Ctor(&powerGpioDrv, powerGpioDrv.gpioConfig);
    GpioDrv_ClearBit(&powerGpioDrv, GPIO_BD2242G_CTRL);
}

void ioExpander_Initial(void)
{
	pIoeLedConfig = (tIoeLedDevice*) getDevicebyIdAndType(LED_DEV_ID, IO_EXPANDER_DEV_TYPE, NULL);
    IoExpanderDrv_Ctor_aw9523(&ioeDrv, pIoeLedConfig);
}

void ioExpander_SetLeds(ledMask leds, bool on)
{
	eLed i;
	ledMask mask;
	uint8 port, pin;
    tIoExpanderLedMap *pLedMap;
    uint8 brightness = 0;

    pLedMap = pIoeLedConfig->pIoExpanderLedMap;

	for (i = LED_MIN; i < LED_MAX; i++)
	{
		mask = 1 << i;
		if (mask & leds)
		{
			port = pLedMap[i].port;
			pin = pLedMap[i].pin;

			if (on)
			{
                brightness = LED_BRIGHTNESS_100 * IO_EXPANDER_LED_BRIGHTNESS_RATE / 100;
			}
            else
            {
                brightness = 0;
            }

            IoExpanderDrv_SetBrightness_aw9523(&ioeDrv, port, pin, brightness);
		}
	}
}
