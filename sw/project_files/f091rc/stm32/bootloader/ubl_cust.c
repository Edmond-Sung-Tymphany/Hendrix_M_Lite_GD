/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  UBL - Upgradable Boot-Loader
                  -------------------------

                  SW Module Document




@file        ubl_cust.c
@brief       Implemented the customizable part of boot-loader which allowed to be upgraded
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

#include "deviceTypes.h"
#include "IoExpanderDrv.h"
#include "pattern.h"
#include "tplog.h"

#define UBL_UART_BUF_SIZE (128)

static uint8 uartTxBuf[UBL_UART_BUF_SIZE];
static cDbg dbg;
#ifdef BL_HAS_I2C
static cIoExpanderDrv ioeDrv;
#endif

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static const uint32 Ubl_version@"UBL_VERSION_SECT" =
    (BL_MAJOR_VERSION << 24) | (BL_MINOR_VERSION1 << 16) |
    (BL_MINOR_VERSION2 << 8) | (BL_MINOR_VERSION3);

#ifdef BL_HAS_I2C
static void UBL_ioeInit()
{
    tIoeLedDevice *pIoeLedConfig = (tIoeLedDevice*) getDevicebyId(LED_DEV_ID, NULL);

    // IO-Expander Init
    IoExpanderDrv_Ctor(&ioeDrv, (const tDevice*)pIoeLedConfig->i2cDeviceConf);
}
#endif

/*****************************************************************************************************************
 *
 * public functions
 *
 *****************************************************************************************************************/
void UBL_cust_init(void)
{
    tUARTDevice* pUartDevice = (tUARTDevice*)getDevicebyId(DEBUG_DEV_ID, NULL);

    // Uart Debug Init
    RingBuf_Ctor(&dbg.txBuf, uartTxBuf, sizeof(uartTxBuf));
    UartDrv_Ctor(&dbg.uartDrv, pUartDevice, &dbg.txBuf, NULL);
    TPLOG_INFO("UBL version: %s [%x]", BOOTLOADER_VERSION, Ubl_version);

#ifdef BL_HAS_I2C
    UBL_ioeInit();
#endif
    // show static LED for an early feeling on unit power-up
}

void UBL_cust_before_stbl(void)
{
    // place holder for the default PIU UI
#ifdef BL_HAS_I2C
    IoExpanderDrv_AutoBlink(&ioeDrv, IOE_AUTO_PATT_STBL);
#endif
}


