/*****************************************************************************
*  @file      main.c
*  @brief     Contains main and is a simple entry point into the SW
*  @author    Christopher Alexander
*  @date      11-Nov-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/
#include "controller.h"
#include "stm32f0xx.h"
#include "bsp.h"
/*..........................................................................*/

#ifdef MCU_FW_SELF_CHECKING

#include "stm32f0xx_crc.h"

#define __MEMORY_ADDR_START       (0x08000000)
#define __MEMORY_ADDR_END         (0x0800f7fb)
#define __MEMORY_ADDR_CSM         (0x0800f7fc)

static uint32 mcu_fw_crc_OK=0;

static uint32 CalcChecksum(uint32* pStart, uint32* pEnd)
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

static void McuFwSelfChecking(void)
{
    uint32 csm_calc, csm_in_rom;
    csm_in_rom = *(uint32 *)__MEMORY_ADDR_CSM;
    csm_calc = CalcChecksum((uint32 *)__MEMORY_ADDR_START, (uint32 *)__MEMORY_ADDR_END);

    if( csm_in_rom == csm_calc )
    {
        mcu_fw_crc_OK = 1;
    }
    else
    {
        mcu_fw_crc_OK = 0;
    }
}

uint32 GetMcuFwStatus(void)
{
    return mcu_fw_crc_OK;
}
#endif

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    //stackMagicNumberFill();
    
    /* initialize the Board Support Package */
    BSP_init();

#ifdef MCU_FW_SELF_CHECKING
    McuFwSelfChecking();
    if( ! mcu_fw_crc_OK )
    {
        // crc error, do nothing to identify the ERROR
        while(1);
    }
#endif

    
    /* let 'er rip */
    return Controller_Ctor(NORMAL_MODE);
}
