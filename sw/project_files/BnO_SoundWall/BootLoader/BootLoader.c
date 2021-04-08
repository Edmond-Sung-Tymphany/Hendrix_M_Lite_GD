/*
-------------------------------------------------------------------------------
TYMPHANY LTD

                  Boot loader implementation.
                  ---------------------------
                  SW Module Document

@file        BootLoader.c
@brief       Implemented the customizable part of boot-loader which will NOT be updated through OTA
@author      Viking Wang
@date        02-Nov-2016
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "stm32f0xx.h"

#include "stm32f0xx_flash.h"
#include "deviceTypes.h"
#include "GpioDrv.h"
#include "UartDrv.h"
#include "Ringbuf.h"
#include "tplog.h"
#include "BootLoader_bsp.h"
#include "Bootloader_common.h"
#include "memory_config.h"
#include "SystemDrv_BTL.h"

#ifdef HAS_IOE_LED
#include "IoeLedDrv.h"
#endif

typedef struct cDbg
{
    cUartDrv    uartDrv;
    cRingBuf    txBuf;
}cDbg;
static cDbg dbg;

#define BTL_UART_BUF_SIZE   (128)
static uint8_t uartTxBuf[BTL_UART_BUF_SIZE];

static const uint32 BL_version@"BL_VERSION_SECT" =
    (BOOTLOADER_VERSION0 << 24) | (BOOTLOADER_VERSION1 << 16) |
    (BOOTLOADER_VERSION2 << 8) | (BOOTLOADER_VERSION3);


/*****************************************************************************************************************
 *
 * public functions
 *
 *****************************************************************************************************************/
void Bl_cust_init(void)
{
    tUARTDevice* pUartDevice = (tUARTDevice*)getDevicebyId(DEBUG_DEV_ID, NULL);

    // power off and shutdown the amp
    BTL_SystemDrv_Init();
    
    // Uart Debug Init
    RingBuf_Ctor(&dbg.txBuf, uartTxBuf, sizeof(uartTxBuf));
    UartDrv_Ctor(&dbg.uartDrv, pUartDevice, &dbg.txBuf, NULL);
    printf("\n\r----------------------------------------------------------------------\n\r");
    printf("\n\r BnO SoundWall Bootloader version: %s[0x%x], built on %s.\n\r", SOUNDWALL_BL_VERSION, BL_version, __DATE__);
    printf("\n\r----------------------------------------------------------------------\n\r");
    
    Bl_BlockingDelayMs(20);
}

/* led show or something else to let the user know */
void BL_error_msg(uint32_t err_msg_id)
{
#ifdef HAS_IOE_LED
    // make sure the A2B power is ON for upgrade
    BTL_SystemDrv_PowerOff();
    BTL_SystemDrv_PowerOnA2B(TRUE);
    IoeLed_Ctor();
    IoeLed_Reset();
    IoeLed_Init();
    Bl_BlockingDelayMs(50);
#endif

    switch( err_msg_id )
    {
    case ERR_MSG_CHECKSUM:
#ifdef HAS_IOE_LED
        IoeLed_SetupMode(LED_ID_RED, IOE_LED_MODE_BLINK, LED_BLINK_MODE_SLOW, 0x00);
        IoeLed_Refresh();
#endif        
        break;
    case ERR_MSG_SYSTEM_FAULT:
#ifdef HAS_IOE_LED
        IoeLed_SetupMode(LED_ID_RED, IOE_LED_MODE_BLINK, LED_FLASH_MODE_FAST, 0x00);
        IoeLed_Refresh();
#endif        
        break;
    case ERR_MSG_ENTER_DFU:
#ifdef HAS_IOE_LED
        IoeLed_SetupMode(LED_ID_RED, IOE_LED_MODE_BLINK, LED_BLINK_MODE_SLOW, 0x00);
        IoeLed_Refresh();
#endif        
        break;
    default : 
        break;
    }
}

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    SystemInit();

    /* When jump from STM32-BL or other region, Memory mapping may be wrong,
       Thus we remap FLASH at 0x00000000 here */
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_Flash);

    bl_BSP_init();

    Bl_cust_init();

    // check "upgrading" flag first
    uint32 enter_dfu_flag;
    enter_dfu_flag = *(uint32 *)(SETT_PAGE_ROM_ADDR+ENTER_DFU_FLAG_OFFSET);
    if( ENTER_DFU_MAGIC_NUMBER == enter_dfu_flag )
    {
        uint32 backup_setting[SETT_MAX_USED_DWORDS];
        uint32 ii, *p_u32, flash_addr;
        printf("\n\rEnter Dfu... UPGRADING......\n\r");
        BSP_BlockingDelayMs(20);

        // backup the useful data and clear the "upgrading" flag
        p_u32 = (uint32 *)SETT_PAGE_ROM_ADDR;
        for(ii=0; ii<SETT_MAX_USED_DWORDS; ii++)
        {
            backup_setting[ii] = *p_u32;
            p_u32 ++;
        }
        FLASH_Unlock();
        FLASH_ErasePage(SETT_PAGE_ROM_ADDR);
        flash_addr = SETT_PAGE_ROM_ADDR;
        for(ii=0; ii<SETT_MAX_USED_DWORDS; ii++)
        {
            FLASH_ProgramWord(flash_addr, backup_setting[ii]);
            flash_addr += 4;
        }
        flash_addr = (SETT_PAGE_ROM_ADDR+ENTER_DFU_FLAG_OFFSET);
        FLASH_ProgramWord(flash_addr, 0);
        FLASH_Lock();
/*
        p_u32 = (uint32 *)SETT_PAGE_ROM_ADDR;
        for(ii=0; ii<SETT_MAX_USED_DWORDS; ii++)
        {
            backup_setting[ii] = *p_u32;
            printf("\n\r ii%d = 0x%x", ii, *p_u32);
            p_u32 ++;
        }
*/
        // show LED
        BL_error_msg(ERR_MSG_ENTER_DFU);
        // jump to ST bootloader.
        bl_jumpAddr(MEMORY_ADDR_ISP);

        return 0;
    }

    // check the APP firmware
    uint32 csm_calc, csm_in_rom;
    csm_in_rom = *(uint32 *)MEMORY_ADDR_APP_CSM;
    csm_calc = bl_calcChecksum((uint32 *)MEMORY_ADDR_APP, (uint32 *)MEMORY_ADDR_APP_END);
    if( csm_in_rom == csm_calc )
    {   // app firmware is OK, jump to app
        bl_jumpAddr(MEMORY_ADDR_APP);
    }
    else
    {
        printf("\n\r - app firmware was destroied. -\n\r");
        printf("\n\r - stored checksum = 0x%x, calculated checksum = 0x%x. \n\r", csm_in_rom, csm_calc);
        BSP_BlockingDelayMs(20);
        BL_error_msg(ERR_MSG_CHECKSUM);
        // jump to ST bootloader.
        bl_jumpAddr(MEMORY_ADDR_ISP);
    }
    
    return 1;   // never reach here
}




