/**
 *  @file      SpiDrv.c
 *  @brief     This file contains the stm32f0xx Spi driver implementation.
 *  @author    Viking Wang
 *  @date      28-Apr-2016
 *  @copyright Tymphany Ltd.
 */

#include "stm32f0xx.h"
#include "commonTypes.h"
#include "SpiDrv.h"
#include "trace.h"
#include "assert.h"
#include "attachedDevices.h"

#ifdef HAS_HW_SPI_DEVICE

#define SPI_TIMEOUT_MS      3

extern uint32 getSysTime();

__weak void Spi1_LowLevel_Deinit()
{
    // projects should have the own implementation
}

__weak void Spi2_LowLevel_Deinit()
{
    // projects should have the own implementation
}


static void SpiDrv_LowLevelInit(cSpiDrv * me)
{
    SPI_TypeDef *SPIx;

    // GPIO select & RCC clock enable
    if (me->pConfig->channel == TP_SPI_CH_1)
    {
        Spi1_LowLevel_Init();
        SPIx = SPI1;
    }
    else if (me->pConfig->channel == TP_SPI_CH_2)
    {
        Spi2_LowLevel_Init();
        SPIx = SPI2;
    }
    else
    {	// config error
        ASSERT(0);
    }

    SPI_Init(SPIx, (SPI_InitTypeDef *)(&(me->pConfig->spiConfig)));
    SPI_CalculateCRC(SPIx, DISABLE);	// CRC disable
    if( me->pConfig->spiConfig.SPI_NSS == SPI_NSS_Hard )
    {
        SPI_SSOutputCmd(SPIx, ENABLE);	// master mode only
        SPI_NSSPulseModeCmd(SPIx, DISABLE);
    }

    //	SPI_Cmd(SPIx, ENABLE);		// in hardware NSS mode, SPI_Cmd used to control CS high/low
}

/**
 * Construct the spi driver instance.
 * @param me - instance of the driver
 * @param pConfig pointer to the config structure
 */
void SpiDrv_Ctor(cSpiDrv * me, tSpiDevice * pConfig)
{
    ASSERT(me && pConfig->deviceInfo.deviceType==SPI_DEV_TYPE);

    /* Duplicate ctor is invalid. If duplicate ctor but no dulipcate xtor,
    */
    ASSERT(!me->isReady);

    me->isBusy = FALSE;
    me->pConfig = pConfig;
    SpiDrv_LowLevelInit(me);

    me->isReady = TRUE;
}

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void SpiDrv_Xtor(cSpiDrv * me)
{
    ASSERT(me);

    /* Duplicate xtor is invalid. If duplicate xtor but no dulipcate ctor,
    */
    ASSERT(me->isReady);

    SPI_TypeDef *SPIx;

    // GPIO select & RCC clock enable
    if (me->pConfig->channel == TP_SPI_CH_1)
    {
        Spi1_LowLevel_Deinit();
        SPIx = SPI1;
    }
    else if (me->pConfig->channel == TP_SPI_CH_2)
    {
        Spi2_LowLevel_Deinit();
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
 * This function sends one byte over the SPI bus in master mode
 * @param me - instance of the driver
 * @param byte - data for transmit
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SpiDrv_WriteByte(cSpiDrv * me, uint8_t byte)
{
    uint32_t tickStart, tx_count;
    SPI_TypeDef *SPIx;
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

    // enable SPI CS : only for Hardware NSS & Master mode
    SPI_Cmd(SPIx, ENABLE);

    tx_count = 1;
    tickStart = getSysTime();

/*
    // check tx empty first
    while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET )
    {
        if( (getSysTime() - tickStart) >= SPI_TIMEOUT_MS )
        {	// timeout error occur.
            tp_ret = TP_FAIL;
            tx_count = 0;	// exit
            break;
        }
    }
*/

    while( tx_count )
    {
        SPI_SendData8(SPIx, byte);
        /* Wait until TXE flag is set to send data */
        if( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == SET )
        {
            tickStart = getSysTime();	// reset timeout count
            tx_count --;
        }
        else
        {
            if( (getSysTime() - tickStart) >= SPI_TIMEOUT_MS )
            {	// timeout error occur.
                tp_ret = TP_FAIL;
                break;
            }
        }
    }

    // disable SPI CS : only for Hardware NSS & Master mode
    SPI_Cmd(SPIx, DISABLE);

    me->isBusy = FALSE;	// release the SPI bus

    return tp_ret;
}

/**
 * This function sends a variable length uint8 array over the SPI bus in master mode
 * @param me - instance of the driver
 * @param pArr - data array to be sent
 * @param len - how many bytes should be sent
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SpiDrv_WriteArray(cSpiDrv * me, uint8_t *pArr, uint32_t len)
{
    uint32_t tickStart, tx_count;
    SPI_TypeDef *SPIx;
    eTpRet tp_ret=TP_SUCCESS;

    ASSERT( ((me->isBusy == FALSE) && me->isReady && len) );

    me->isBusy = TRUE;	// hold the bus

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

    // enable SPI CS : only for Hardware NSS & Master mode
    SPI_Cmd(SPIx, ENABLE);

    tx_count = len;
    tickStart = getSysTime();

/*
    // check tx empty first
    while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET )
    {
        if( (getSysTime() - tickStart) >= SPI_TIMEOUT_MS )
        {	// timeout error occur.
            tp_ret = TP_FAIL;
            tx_count = 0;	// exit
            break;
        }
    }
*/

    while( tx_count )
    {
        SPI_SendData8(SPIx, *pArr);
        pArr ++;
        /* Wait until TXE flag is set to send data */
        if( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == SET )
        {
            tickStart = getSysTime();	// reset timeout count
            tx_count --;
        }
        else
        {
            if( (getSysTime() - tickStart) >= SPI_TIMEOUT_MS )
            {	// timeout error occur.
                tp_ret = TP_FAIL;
                break;
            }
        }
    }

    // disable SPI CS : only for Hardware NSS & Master mode
    SPI_Cmd(SPIx, DISABLE);

    me->isBusy = FALSE;		// bus release

    return tp_ret;
}

/**
 * This function reads a byte over the SPI bus in master mode
 * @param me - instance of the driver
 * @param pByte - the reading data pointer.
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SpiDrv_ReadByte(cSpiDrv * me, uint8_t *pByte)
{
    uint32_t tickStart, rx_count;
    SPI_TypeDef *SPIx;
    eTpRet tp_ret=TP_SUCCESS;

    ASSERT( ((me->isBusy == FALSE) && me->isReady) );

    me->isBusy = TRUE;	// hold the bus

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

    // setup RX fifo to 8bits
    SPI_RxFIFOThresholdConfig(SPIx, SPI_RxFIFOThreshold_QF);

    // enable SPI CS : only for Hardware NSS & Master mode
    SPI_Cmd(SPIx, ENABLE);

    rx_count = 1;
    tickStart = getSysTime();

    while( rx_count )
    {
        while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET )
        {
            if( (getSysTime() - tickStart) >= SPI_TIMEOUT_MS )
            {	// timeout error occur.
                tp_ret = TP_FAIL;
                rx_count = 0;	// exit
                break;
            }
            tickStart = getSysTime();
        }

        if( rx_count )
        {
            // send a dummy data for SPI clock
            SPI_SendData8(SPIx, 0xee);			
            while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET )
            {
                if( (getSysTime() - tickStart) >= SPI_TIMEOUT_MS )
                {	// timeout error occur.
                    tp_ret = TP_FAIL;
                    rx_count = 0;	// exit
                    break;
                }
            }
            if( rx_count )
            {
                *pByte = SPI_ReceiveData8(SPIx);
                rx_count --;
            }
        }
    }

    // disable SPI CS : only for Hardware NSS & Master mode
    SPI_Cmd(SPIx, DISABLE);

    me->isBusy = FALSE;		// bus release

    return tp_ret;
}

/**
 * This function reads a variable length uint8 array over the SPI bus in master mode
 * @param me - instance of the driver
 * @param pArr - the reading data pointer.
 * @param len - how many bytes to read.
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SpiDrv_ReadArray(cSpiDrv * me, uint8_t *pArr, uint32_t len)
{
    uint32_t tickStart, rx_count;
    SPI_TypeDef *SPIx;
    eTpRet tp_ret=TP_SUCCESS;

    ASSERT( ((me->isBusy == FALSE) && me->isReady && len) );

    me->isBusy = TRUE;	// hold the bus

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

    // setup RX fifo to 8bits
    SPI_RxFIFOThresholdConfig(SPIx, SPI_RxFIFOThreshold_QF);

    // enable SPI CS : only for Hardware NSS & Master mode
    SPI_Cmd(SPIx, ENABLE);

    rx_count = len;
    tickStart = getSysTime();

    while( rx_count )
    {
        while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET )
        {
            if( (getSysTime() - tickStart) >= SPI_TIMEOUT_MS )
            {	// timeout error occur.
                tp_ret = TP_FAIL;
                rx_count = 0;	// exit
                break;
            }
            tickStart = getSysTime();
        }

        if( rx_count )
        {
            // send a dummy data for SPI clock
            SPI_SendData8(SPIx, 0xee);			
            while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET )
            {
                if( (getSysTime() - tickStart) >= SPI_TIMEOUT_MS )
                {	// timeout error occur.
                    tp_ret = TP_FAIL;
                    rx_count = 0;	// exit
                    break;
                }
            }
            if( rx_count )
            {
                *pArr = SPI_ReceiveData8(SPIx);
                pArr ++;
                rx_count --;
            }
        }
    }

    // disable SPI CS : only for Hardware NSS & Master mode
    SPI_Cmd(SPIx, DISABLE);

    me->isBusy = FALSE; 	// bus release

    return tp_ret;
}

/**
 * This function reads/writes a variable length uint8 array over the SPI bus in master mode
 * @param me - instance of the driver
 * @param pRxArr - the reading data pointer.
 * @param pTxArr - the writing data pointer.
 * @param len - how many bytes to read.
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SpiDrv_ReadWriteArray(cSpiDrv * me, uint8_t *pRxArr, uint8_t *pTxArr, uint32_t len)
{
    uint32_t tickStart, rx_count;
    SPI_TypeDef *SPIx;
    eTpRet tp_ret=TP_SUCCESS;

    ASSERT( ((me->isBusy == FALSE) && me->isReady && len) );

    me->isBusy = TRUE;	// hold the bus

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

    // setup RX fifo to 8bits
    SPI_RxFIFOThresholdConfig(SPIx, SPI_RxFIFOThreshold_QF);

    // enable SPI CS : only for Hardware NSS & Master mode
    SPI_Cmd(SPIx, ENABLE);

    rx_count = len;
    tickStart = getSysTime();

    while( rx_count )
    {
        while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET )
        {
            if( (getSysTime() - tickStart) >= SPI_TIMEOUT_MS )
            {	// timeout error occur.
                tp_ret = TP_FAIL;
                rx_count = 0;	// exit
                break;
            }
            tickStart = getSysTime();
        }

        if( rx_count )
        {
            SPI_SendData8(SPIx, *pTxArr);
			pTxArr ++;
            while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET )
            {
                if( (getSysTime() - tickStart) >= SPI_TIMEOUT_MS )
                {	// timeout error occur.
                    tp_ret = TP_FAIL;
                    rx_count = 0;	// exit
                    break;
                }
            }
            if( rx_count )
            {
                *pRxArr = SPI_ReceiveData8(SPIx);
                pRxArr ++;
                rx_count --;
            }
        }
    }

    // disable SPI CS : only for Hardware NSS & Master mode
    SPI_Cmd(SPIx, DISABLE);

    me->isBusy = FALSE; 	// bus release

    return tp_ret;
}


#endif	// HAS_HW_SPI_DEVICE

