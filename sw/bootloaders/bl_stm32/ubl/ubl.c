/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  UBL - Upgradable Boot-Loader
                  -------------------------

                  SW Module Document




@file        ubl.c
@brief       Implemented a boot-loader image which allowed to be upgraded
@author      Wesley Lee
@date        30-Nov-2015
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2015-11     Wesley Lee
DESCRIPTION: First Draft.
SCO/ERROR  :
-------------------------------------------------------------------------------
*/

#include "stm32f0xx.h"
#include "ubl.h"
#include "tplog.h"

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

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void UBL_fwErrorHandler()
{
    UBL_cust_action_before_stbl();
    bl_jumpAddr(FEP_ADDR_ISP);
}

__weak void UBL_cust_init(void)
{
    // projects should have the own implementation
}

__weak void UBL_ui(void)
{
    // projects should have the own implementation on UI
}

/*****************************************************************************************************************
 *
 * public functions
 *
 *****************************************************************************************************************/
int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;
    uint32 firmware_checksum = *(uint32*)FEP_ADDR_FIRMWARE_CHECKSUM;
    uint32 fw_calc = bl_calcChecksum((uint32*) FEP_ADDR_FIRMWARE, (uint32*) FEP_ADDR_FIRMWARE_END);
    eFepStblStatus stbl_status = bl_getStblStatus();
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
        VectorTable[i] = *(__IO uint32_t*)(FEP_ADDR_UBL + (i<<2));
    }
    
    /* Enable the SYSCFG peripheral clock*/
    /* Note original STM32 bootlaoder sample call RCC_APB2PeriphResetCmd() to start SYSCFG
     * But it is a bug, RCC_APB2PeriphResetCmd() disable SYSCFG, and EXTI have problem.
     */
    //RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Remap SRAM at 0x00000000 */
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);

    /* We do not enable BL_ACCURARY_DELAY, thus do not need tick timer */ 
    //bl_BSP_init(); 
    
    /* Enable interrupt here cause FW sys_tick interrupt do not work.
     * before fix this bug, the workaround is disable UBL interrupt.
     */
    //__enable_interrupt();
    
    //customized init
    UBL_cust_init();  

    //Write version if stored version is wrong
    bl_writeVersion((void*)FEP_ADDR_UBL_VER, BL_MAJOR_VERSION, BL_MINOR_VERSION1, BL_MINOR_VERSION2, BL_MINOR_VERSION3);

#ifdef BRINGUP_DEBUG
    TPLOG_INFO("\n\rUBL jump to APP...");
    bl_jumpAddr(FEP_ADDR_FIRMWARE);
#else
    
    TPLOG_INFO("UBL: STBL status = 0x%X", stbl_status);
    switch (stbl_status)
    {
    case FEP_STBL_NORMAL:
        if ( fw_calc == firmware_checksum )
        {
            TPLOG_INFO("UBL: jump to Application\r\n\r\n");
            bl_jumpAddr(FEP_ADDR_FIRMWARE);
        }
        else
        {
            TPLOG_ERROR("Application checksum error: %x [%x]", fw_calc, firmware_checksum);
            bl_setStblStatus(FEP_STBL_ERROR_FIRMWARE);
        }
        // fall through to ISP
    case FEP_STBL_NEW:          // unexpected, handled in PIU
    case FEP_STBL_UPGRADE_UBL:
    case FEP_STBL_UPGRADE_FIRMWARE:
    case FEP_STBL_ERROR_UBL:
    case FEP_STBL_ERROR_FIRMWARE:
    default:
        TPLOG_ERROR("UBL: Enetering ISP\r\n\r\n");
        UBL_cust_before_stbl();
        bl_jumpAddr(FEP_ADDR_ISP);
        break;
    }
#endif

    return 1;   // never reach here
}


