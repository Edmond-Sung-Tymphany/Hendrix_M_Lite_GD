/*
-------------------------------------------------------------------------------
TYMPHANY LTD

                  PIU - Power Initialize Unit
                  -------------------------

                  SW Module Document

@file        atmos_bl.c
@brief       Implemented the customizable part of boot-loader which will NOT be updated through OTA
@author      Viking Wang
@date        04-Jun-2016
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2015-11     Wesley Lee
DESCRIPTION: First Draft.
SCO/ERROR  :
-------------------------------------------------------------------------------
*/

#include "stm32f0xx.h"
#include "piu.h"

#include "deviceTypes.h"
#include "GpioDrv.h"
#include "IoExpanderDrv.h"
#include "pattern.h"
#include "tplog.h"
#include "fep_addr.h"
#include "fep_addr_priv.h"

#define PIU_UART_BUF_SIZE   (128)

static uint8 uartTxBuf[PIU_UART_BUF_SIZE];
static cDbg dbg;
#ifdef BL_HAS_I2C
static cIoExpanderDrv ioeDrv;
#endif

static const uint32 BL_version@"BL_VERSION_SECT" =
    (BOOTLOADER_VERSION0 << 24) | (BOOTLOADER_VERSION1 << 16) |
    (BOOTLOADER_VERSION2 << 8) | (BOOTLOADER_VERSION3);


#ifdef BL_HAS_I2C
// IO-Expander Init
static void PIU_ioeInit(void)
{    
    tIoeLedDevice *pIoeLedConfig = (tIoeLedDevice*) getDevicebyIdAndType(LED_DEV_ID, I2C_DEV_TYPE, NULL);
    IoExpanderDrv_Ctor(&ioeDrv, (const tDevice*)pIoeLedConfig->i2cDeviceConf);
}
#endif

/*****************************************************************************************************************
 *
 * public functions
 *
 *****************************************************************************************************************/
void PIU_cust_init(void)
{
    tUARTDevice* pUartDevice = (tUARTDevice*)getDevicebyId(DEBUG_DEV_ID, NULL);

    // Uart Debug Init
    RingBuf_Ctor(&dbg.txBuf, uartTxBuf, sizeof(uartTxBuf));
    UartDrv_Ctor(&dbg.uartDrv, pUartDevice, &dbg.txBuf, NULL);
//    TPLOG_INFO("Bootloader version: %s [%x], built on %s %s", ATMOS_BL_VERSION, BL_version, __DATE__, __TIME__);
    printf("\n\r--------------------------------------------------------------\n\r");
    printf("\n\r ATMOS BL Ver:%s, built on %s %s \n\r", ATMOS_BL_VERSION,  __DATE__, __TIME__);    
    printf("\n\r--------------------------------------------------------------\n\r");
    BSP_BlockingDelayMs(10);
}

__weak void PIU_cust_new_init(void)
{
    // projects should have the own implementation
}

__weak void PIU_cust_normal_init(void)
{
    // projects should have the own implementation
}

void PIU_cust_before_stbl(void)
{
    // place holder for the default PIU UI
#ifdef BL_HAS_I2C
    PIU_ioeInit();
#ifdef IOEXP_IS_AW9110B
    IoExpanderDrv_AutoBlink(&ioeDrv, IOE_AUTO_PATT_STBL);
#endif
#endif

}

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    /* When jump from STM32-BL or other region, Memory mapping may be wrong,
       Thus we remap FLASH at 0x00000000 here */
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_Flash);

    eFepStblStatus stbl_status = bl_getStblStatus();
    
    /* We do not enable BL_ACCURARY_DELAY, thus do not need tick timer */ 
    //bl_BSP_init();
    
    PIU_cust_init();

#if 1
    bl_jumpAddr(FEP_ADDR_FIRMWARE);
#else
    uint32 ubl_calc = bl_calcChecksum((uint32*)FEP_ADDR_UBL, (uint32*)FEP_ADDR_UBL_END);
    uint32 fw_calc = bl_calcChecksum((uint32*)FEP_ADDR_FIRMWARE, (uint32*)FEP_ADDR_FIRMWARE_END);

    if (stbl_status == FEP_STBL_NEW)
    {
        // skip ASE-TK boot
        PIU_cust_new_init();
        bl_setStblStatus(FEP_STBL_NORMAL);  // avoid skipping for next boot-up
        bl_setFirmwareStatus(FEP_FIRMWARE_NEW);
        TPLOG_INFO("PIU: skip ASE-TK boot");
    }
    else
    {
        PIU_cust_normal_init();
        bl_setFirmwareStatus(FEP_FIRMWARE_POWERED_ASETK);
        TPLOG_INFO("PIU: boot ASE-TK");
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




