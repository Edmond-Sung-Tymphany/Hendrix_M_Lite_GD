/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  PIU - Power Initialize Unit
                  -------------------------

                  SW Module Document




@file        piu.c
@brief       Implemented a boot-loader which will NOT be updated through OTA
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
#include "piu.h"
#include "tplog.h"

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void PIU_ublErrorHandler()
{
    PIU_cust_before_stbl();
    bl_jumpAddr(FEP_ADDR_ISP);
}

__weak void PIU_cust_init(void)
{
    // projects should have the own implementation
}

__weak void PIU_cust_new_init(void)
{
    // projects should have the own implementation
}

__weak void PIU_cust_normal_init(void)
{
    // projects should have the own implementation
}

__weak void PIU_cust_before_stbl(void)
{
    // projects should have the own implementation
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

    /* When jump from STM32-BL or other region, Memory mapping may be wrong,
       Thus we remap FLASH at 0x00000000 here */
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_Flash);

    uint32 ubl_checksum = *(uint32 *) FEP_ADDR_UBL_CHECKSUM;
    eFepStblStatus stbl_status = bl_getStblStatus();
    
    /* We do not enable BL_ACCURARY_DELAY, thus do not need tick timer */ 
    //bl_BSP_init();
    
    PIU_cust_init();

    //Write version if stored version is wrong
    bl_writeVersion((void*)FEP_ADDR_PIU_VER, PIU_MAJOR_VERSION, PIU_MINOR_VERSION1, PIU_MINOR_VERSION2, PIU_MINOR_VERSION3);

    uint32 ubl_calc = bl_calcChecksum((uint32*)FEP_ADDR_UBL, (uint32*)FEP_ADDR_UBL_END);
    uint32 fw_calc = bl_calcChecksum((uint32*)FEP_ADDR_FIRMWARE, (uint32*)FEP_ADDR_FIRMWARE_END);

#ifdef BRINGUP_DEBUG
    TPLOG_INFO("\n\rPIU jump to UBL...");
    bl_jumpAddr(FEP_ADDR_UBL);
#else
    if (stbl_status == FEP_STBL_NEW)
    {
        // skip WiFi/BT module boot
        PIU_cust_new_init();
        bl_setStblStatus(FEP_STBL_NORMAL);  // avoid skipping for next boot-up
        bl_setFirmwareStatus(FEP_FIRMWARE_NEW);
        TPLOG_INFO("PIU: skip Wifi/BT module boot");
    }
    else
    {
        PIU_cust_normal_init();
        bl_setFirmwareStatus(FEP_FIRMWARE_POWERED_ASETK);
        TPLOG_INFO("PIU: boot WiFi/BT module");
    }
    
    TPLOG_INFO("PIU: checksum:  calc  [  stored  ]");
    TPLOG_INFO("PIU: ubl = 0x%X [0x%X]", ubl_calc, ubl_checksum);
    TPLOG_INFO("PIU: fw  = 0x%X [0x%X]", fw_calc, *(uint32 *) FEP_ADDR_FIRMWARE_CHECKSUM);
    TPLOG_INFO("PIU: STBL status = 0x%X\r\n", stbl_status);
    
    switch(stbl_status)
    {
    case FEP_STBL_NORMAL:
    case FEP_STBL_NEW:
    case FEP_STBL_UPGRADE_UBL:
    case FEP_STBL_UPGRADE_FIRMWARE:
    case FEP_STBL_ERROR_UBL:
    case FEP_STBL_ERROR_FIRMWARE:
        if ( ubl_checksum == ubl_calc )
        {
            bl_jumpAddr(FEP_ADDR_UBL);
        }
        bl_setStblStatus(FEP_STBL_ERROR_UBL);
        // fall through to ISP
    default:
        TPLOG_ERROR("PIU: Entering ISP\r\n");
        PIU_ublErrorHandler();
        break;
    }
#endif

    return 1;   // never reach here
}


