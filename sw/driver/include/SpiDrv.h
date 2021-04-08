/**
 *  @file      SpiDrv.h
 *  @brief     This file presents low level interface to SPI driver
 *              and reference to specific port
 *  @author    Viking Wang
 *  @date      28-Apr-2016
 *  @copyright Tymphany Ltd.
 */

#ifndef __SPIDRV_H__
#define __SPIDRV_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "deviceTypes.h"

typedef struct tagSpiDrv{
    bool    isReady;
    bool    isBusy;
    tSpiDevice  *pConfig;
}cSpiDrv;

/**
 * Construct the spi driver instance.
 * @param cSpiDrv * me - driver to construct
 * @param tSpiDevice * config - parameters to initialise with.
 */
void SpiDrv_Ctor(cSpiDrv * me, tSpiDevice * config);

/*
 * Start driver instance.
 * @param cSpiDrv * me - pointer to driver instance
 */
void SpiDrv_Startup(cSpiDrv * me);

/*
 * Exit & clean up the driver.
 * @param cSpiDrv * me - pointer to driver instance
 */
void SpiDrv_Xtor(cSpiDrv * me);

/**
 * This function sends one byte over the SPI bus in master mode
 * @param me - instance of the driver
 * @param byte - data for transmit
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SpiDrv_WriteByte(cSpiDrv * me, uint8_t byte);

/**
 * This function sends a variable length uint8 array over the SPI bus in master mode
 * @param me - instance of the driver
 * @param pArr - data array to be sent
 * @param len - how many bytes should be sent
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SpiDrv_WriteArray(cSpiDrv * me, uint8_t *pArr, uint32_t len);

/**
 * This function reads a byte over the SPI bus in master mode
 * @param me - instance of the driver
 * @param pByte - the reading data pointer.
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SpiDrv_ReadByte(cSpiDrv * me, uint8_t *pByte);

/**
 * This function reads a variable length uint8 array over the SPI bus in master mode
 * @param me - instance of the driver
 * @param pArr - the reading data pointer.
 * @param len - how many bytes to read.
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SpiDrv_ReadArray(cSpiDrv * me, uint8_t *pArr, uint32_t len);

/**
* This function reads/writes a variable length uint8 array over the SPI bus in master mode
* @param me - instance of the driver
* @param pRxArr - the reading data pointer.
* @param pTxArr - the writing data pointer.
* @param len - how many bytes to read.
* @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
*/
eTpRet SpiDrv_ReadWriteArray(cSpiDrv * me, uint8_t *pRxArr, uint8_t *pTxArr, uint32_t len);

#ifdef  __cplusplus
}
#endif

#endif  // __SPIDRV_H__

