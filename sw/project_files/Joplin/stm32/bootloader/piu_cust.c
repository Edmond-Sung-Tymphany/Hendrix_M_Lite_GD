/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  PIU - Power Initialize Unit
                  -------------------------

                  SW Module Document




@file        piu_cust.c
@brief       Implemented the customizable part of boot-loader which will NOT be updated through OTA
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

static cGpioDrv btmGpioDrv;
static cGpioDrv audioGpioDrv;

static const uint32 Piu_version@"PIU_VERSION_SECT" =
    (BL_MAJOR_VERSION << 24) | (BL_MINOR_VERSION1 << 16) |
    (BL_MINOR_VERSION2 << 8) | (BL_MINOR_VERSION3);


/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void btm_pin_init(void)
{
    tGPIODevice *pAsetkConfig = (tGPIODevice*) getDevicebyIdAndType(BT_DEV_ID, GPIO_DEV_TYPE, NULL);
    GpioDrv_Ctor(&btmGpioDrv, pAsetkConfig);
}

 
#ifdef BL_HAS_I2C
// IO-Expander Init
static void PIU_ioeInit(void)
{    
    tIoeLedDevice *pIoeLedConfig = (tIoeLedDevice*) getDevicebyIdAndType(LED_DEV_ID, IO_EXPANDER_DEV_TYPE, NULL);
    IoExpanderDrv_Ctor_aw9110b(&ioeDrv, pIoeLedConfig);
}
#endif


static void btm_power_up(void)
{
    const uint32_t delay = 100;   // ms
    GpioDrv_ClearBit(&btmGpioDrv, BT_PWR_EN);
    GpioDrv_ClearBit(&btmGpioDrv, BT_RESET_PIN);
    BSP_BlockingDelayMs(delay);

    GpioDrv_SetBit(&btmGpioDrv, BT_PWR_EN);
    BSP_BlockingDelayMs(delay);

    GpioDrv_SetBit(&btmGpioDrv, BT_RESET_PIN);
    BSP_BlockingDelayMs(delay);      
}

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
    TPLOG_INFO("PIU: v%s [%x], built  on %s %s", PIU_VERSION, Piu_version, __DATE__, __TIME__);
    
    /* PIU may need to jump to ST-Bootloader, and pull high GPIO_OUT_BOOT_STATUS_SW, 
     * thus we earily init bt module pins here.
     * Note default value of (BT_PWR_EN, BT_PWR_EN) is floating, thus this init
     * do not reset BT module
     */
    btm_pin_init();
    BSP_BlockingDelayMs(100); // 100ms
}


void PIU_cust_normal_init(void)
{
    btm_power_up();
}


void PIU_cust_new_init(void)
{
    //TODO: check BT module pin, if low, then still power up it
}


void PIU_cust_before_stbl(void)
{
    //Notify BT module that FEP need upgrade
    GpioDrv_SetBit(&btmGpioDrv, GPIO_OUT_BOOT_STATUS_SW);
    
    //Init Audio pins to turn off amplifier
    audioGpioDrv.gpioConfig= (tGPIODevice*)getDevicebyIdAndType(AUDIO_DEV_ID, GPIO_DEV_TYPE, NULL);
    GpioDrv_Ctor(&audioGpioDrv, audioGpioDrv.gpioConfig);
    
    // place holder for the default PIU UI
#ifdef BL_HAS_I2C
    PIU_ioeInit();
    IoExpanderDrv_AutoBlink_aw9110b(&ioeDrv, IOE_AUTO_PATT_STBL);
#endif

}




