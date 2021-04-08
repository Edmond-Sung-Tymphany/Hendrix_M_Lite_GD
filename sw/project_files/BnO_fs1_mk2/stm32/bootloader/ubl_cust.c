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
#include "GpioDrv.h"
#include "IoExpanderDrv.h"
#include "pattern.h"
#include "tplog.h"
#include "fep_addr.h"
#include "fep_addr_priv.h"

#define UBL_UART_BUF_SIZE (128)

static uint8 uartTxBuf[UBL_UART_BUF_SIZE];
static cDbg dbg;
#ifdef BL_HAS_I2C
static cIoExpanderDrv ioeDrv;
#endif

static cGpioDrv asetk;
static cGpioDrv audioGpioDrv;

static const uint32 Ubl_version@"UBL_VERSION_SECT" =
    (BL_MAJOR_VERSION << 24) | (BL_MINOR_VERSION1 << 16) |
    (BL_MINOR_VERSION2 << 8) | (BL_MINOR_VERSION3);


/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void asetk_pin_init(void)
{
    tGPIODevice *pAsetkConfig = (tGPIODevice*) getDevicebyIdAndType(ASETK_DEV_ID, GPIO_DEV_TYPE, NULL);
    GpioDrv_Ctor(&asetk, pAsetkConfig);
}

 
#ifdef BL_HAS_I2C
// IO-Expander Init
static void UBL_ioeInit()
{
    tIoeLedDevice *pIoeLedConfig = (tIoeLedDevice*) getDevicebyIdAndType(LED_DEV_ID, IO_EXPANDER_DEV_TYPE, NULL);
    IoExpanderDrv_Ctor_aw9110b(&ioeDrv, pIoeLedConfig);
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
    TPLOG_INFO("UBL: v%s [%x], built on %s %s", BOOTLOADER_VERSION, Ubl_version, __DATE__, __TIME__);

    asetk_pin_init();
    BSP_BlockingDelayMs(100); // 100ms

    //TPLOG_INFO("UBL: read firmware version: v%s", bl_readVersion((void*)FEP_ADDR_FIRMWARE_VER));
}


void UBL_cust_before_stbl(void)
{
    //Notify ASE-TK that FEP need upgrade
    GpioDrv_SetBit(&asetk, GPIO_OUT_BOOT_STATUS_SW);
	
	//Init Audio pins to turn off amplifier
    audioGpioDrv.gpioConfig= (tGPIODevice*)getDevicebyIdAndType(AUDIO_DEV_ID, GPIO_DEV_TYPE, NULL);
    GpioDrv_Ctor(&audioGpioDrv, audioGpioDrv.gpioConfig);
	
    // place holder for the default PIU UI
#ifdef BL_HAS_I2C
	UBL_ioeInit();
    IoExpanderDrv_AutoBlink_aw9110b(&ioeDrv, IOE_AUTO_PATT_STBL);
#endif

}




