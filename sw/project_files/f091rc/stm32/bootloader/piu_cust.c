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
#include "fep_addr.h"

#include "deviceTypes.h"
#include "GpioDrv.h"
#include "IoExpanderDrv.h"
#include "pattern.h"
#include "tplog.h"

#define PIU_UART_BUF_SIZE   (128)

static uint8 uartTxBuf[PIU_UART_BUF_SIZE];
static cDbg dbg;
#ifdef BL_HAS_I2C
static cIoExpanderDrv ioeDrv;
#endif
static cGpioDrv powerDrv;

static const uint32 Piu_version@"PIU_VERSION_SECT" =
    (BL_MAJOR_VERSION << 24) | (BL_MINOR_VERSION1 << 16) |
    (BL_MINOR_VERSION2 << 8) | (BL_MINOR_VERSION3);

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
/**
  * @brief This function provides accurate delay (in milliseconds) based
  *        on variable incremented.
  * @note In the default implementation , SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals where uwTick
  *       is incremented.
  * @note ThiS function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @param Delay: specifies the delay time length, in milliseconds.
  * @retval None
  */
static void HAL_Delay(__IO uint32_t Delay)
{
    uint32_t tickstart = getSysTime();
    while(getSysTime() - tickstart < Delay)
    {
    }
}

static void asetk_pin_init(void)
{
    tGPIODevice *pPowerConfig = (tGPIODevice*) getDevicebyId(ASETK_DEV_ID, NULL);
    GpioDrv_Ctor(&powerDrv, pPowerConfig);
    GpioDrv_SetBit(&powerDrv, GPIO_OUT_SYSPWR_ON);
    GpioDrv_SetBit(&powerDrv, GPIO_OUT_IOEXP_RST);
}

static void asetk_power_up(void)
{
    const uint32_t delay = 100;     // ms

    asetk_pin_init();
    HAL_Delay(delay);

    GpioDrv_ClearBit(&powerDrv, GPIO_OUT_ASE_SYS_EN);
    GpioDrv_ClearBit(&powerDrv, GPIO_OUT_ASE_RST_N);
    HAL_Delay(delay);

    GpioDrv_SetBit(&powerDrv, GPIO_OUT_ASE_SYS_EN);
    HAL_Delay(delay);

    GpioDrv_SetBit(&powerDrv, GPIO_OUT_ASE_RST_N);
    HAL_Delay(delay);
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
    TPLOG_INFO("PIU version: %s [%x]", PIU_VERSION, Piu_version);
}

void PIU_cust_normal_init(void)
{
    asetk_power_up();
}

void PIU_cust_before_stbl(void)
{
    // place holder for the default PIU UI
#ifdef BL_HAS_I2C
    tIoeLedDevice *pIoeLedConfig = (tIoeLedDevice*) getDevicebyId(LED_DEV_ID, NULL);

    IoExpanderDrv_Ctor(&ioeDrv, (const tDevice*)pIoeLedConfig->i2cDeviceConf);
    IoExpanderDrv_AutoBlink(&ioeDrv, IOE_AUTO_PATT_STBL);
#endif

}




