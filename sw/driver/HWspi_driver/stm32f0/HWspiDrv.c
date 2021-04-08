/**
 *  @file      HWspiDrv.c
 *  @brief     This file contains the stm32f0xx HWspi driver implementation.
 *  @author    Viking Wang
 *  @date      16-Oct-2017
 *  @copyright Tymphany Ltd.
 */

#include "stm32f0xx.h"
#include "commonTypes.h"
#include "HWspiDrv.h"
#include "trace.h"
#include "assert.h"
#include "attachedDevices.h"

#ifdef HAS_HW_SPI_DEVICE

#define SPI_TIMEOUT_MS      3

extern uint32 getSysTime();

__weak void HWspi1_LowLevel_Init()
{
    // projects should have the own implementation
}

__weak void HWspi2_LowLevel_Init()
{
    // projects should have the own implementation
}

__weak void HWspi1_LowLevel_Deinit()
{
    // projects should have the own implementation
}

__weak void HWspi2_LowLevel_Deinit()
{
    // projects should have the own implementation
}


static void HWspiDrv_LowLevelInit(cHWspiDrv * me)
{
    SPI_TypeDef *SPIx;

    // GPIO select & RCC clock enable
    if (me->pConfig->channel == TP_SPI_CH_1)
    {
        HWspi1_LowLevel_Init();
        SPIx = SPI1;
    }
    else if (me->pConfig->channel == TP_SPI_CH_2)
    {
        HWspi2_LowLevel_Init();
        SPIx = SPI2;
    }
    else
    {	// config error
        ASSERT(0);
    }

    SPI_Init(SPIx, (SPI_InitTypeDef *)(&(me->pConfig->spiConfig)));
    SPI_CalculateCRC(SPIx, DISABLE);	// CRC disable
    if( me->pConfig->spiConfig.SPI_NSS == SPI_NSS_Soft )
    {
        // SSOE bit control, use external GPIO for chip-select control.
        SPI_SSOutputCmd(SPIx, DISABLE);
        // disable NSS pulse between two consecutive data
        SPI_NSSPulseModeCmd(SPIx, DISABLE);
        // setup RX fifo to 8bits
        SPI_RxFIFOThresholdConfig(SPIx, SPI_RxFIFOThreshold_QF);
    }
    else
    {
        // only support soft mode now.
        ASSERT(0);
    }

    SPI_Cmd(SPIx, ENABLE);
}

/**
 * Construct the spi driver instance.
 * @param me - instance of the driver
 * @param pConfig pointer to the config structure
 */
void HWspiDrv_Ctor(cHWspiDrv * me, tSpiDevice * pConfig)
{
    ASSERT(me && pConfig->deviceInfo.deviceType==SPI_DEV_TYPE);

    /* Duplicate ctor is invalid. If duplicate ctor but no dulipcate xtor,
    */
    ASSERT(!me->isReady);

    me->isBusy = FALSE;
    me->pConfig = pConfig;
    HWspiDrv_LowLevelInit(me);

    me->isReady = TRUE;
}

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void HWspiDrv_Xtor(cHWspiDrv * me)
{
    ASSERT(me);

    /* Duplicate xtor is invalid. If duplicate xtor but no dulipcate ctor,
    */
    ASSERT(me->isReady);

    SPI_TypeDef *SPIx;

    // GPIO select & RCC clock enable
    if (me->pConfig->channel == TP_SPI_CH_1)
    {
        HWspi1_LowLevel_Deinit();
        SPIx = SPI1;
    }
    else if (me->pConfig->channel == TP_SPI_CH_2)
    {
        HWspi2_LowLevel_Deinit();
        SPIx = SPI2;
    }
    else
    {	// config error
        ASSERT(0);
    }

    SPI_Cmd(SPIx, DISABLE);
    SPI_I2S_DeInit(SPIx);

    me->isReady = FALSE;	
}

/**
 * @param me - instance of the driver
 * @param pTxData - TX data buffer
 * @param pRxData - RX data buffer
 * @param len - bytes to read/write
 * @param timeout - failure timeout, unit : ms
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet HWspiDrv_WriteRead(cHWspiDrv * me, uint8_t *pTxData, uint8_t *pRxData, uint32_t len, uint32_t timeout)
{
    uint32_t tickStart, tx_count, rx_count;
    SPI_TypeDef *SPIx;
    uint8_t tmp_buf;
    eTpRet tp_ret=TP_SUCCESS;

    ASSERT( ((me->isBusy == FALSE) && me->isReady) );

    me->isBusy = TRUE;	// hold the SPI bus

    if (me->pConfig->channel == TP_SPI_CH_1)
    {
        SPIx = SPI1;
    }
    else if (me->pConfig->channel == TP_SPI_CH_2)
    {
        SPIx = SPI2;
    }
    else
    {	// config error
        ASSERT(0);
    }

    // enable SPI
//    SPI_Cmd(SPIx, ENABLE);

    tx_count = len;
    rx_count = len;
    tickStart = getSysTime();

    while( tx_count || rx_count )
    {
        if( tx_count && (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == SET) )
        {
            if( pTxData )
            {
                SPI_SendData8(SPIx, *pTxData);
                pTxData ++;
            }
            else
            {
                SPI_SendData8(SPIx, 0x00);
            }
            tx_count --;
        }
        if( rx_count && (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == SET) )
        {
            tmp_buf = SPI_ReceiveData8(SPIx);
            if( pRxData )
            {
                *pRxData = tmp_buf;
                pRxData ++;
            }
            rx_count --;
        }
        if( (getSysTime() - tickStart) > timeout )
        {
            tp_ret = TP_FAIL;
            break;
        }
    }

    // check Fifo empty
    while( SPI_I2S_GetFlagStatus(SPIx, SPI_SR_FTLVL) == SET )
    {
        if( (getSysTime() - tickStart) > timeout )
        {
            tp_ret = TP_FAIL;
            break;
        }
    }
    // check busy flag
    while( SPI_I2S_GetFlagStatus(SPIx, SPI_SR_BSY) == SET )
    {
        if( (getSysTime() - tickStart) > timeout )
        {
            tp_ret = TP_FAIL;
            break;
        }
    }

    me->isBusy = FALSE;	// release the SPI bus

    if( tp_ret != TP_SUCCESS )
    {
        TP_PRINTF("\n\rHW SPI timeout...");
    }
    
    return tp_ret;
}

#endif	// HAS_HW_SPI_DEVICE

