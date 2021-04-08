/**
 *  @file      HWspiDrv.h
 *  @brief     This file presents low level interface to SPI driver
 *              and reference to specific port
 *  @author    Viking Wang
 *  @date      16-Oct-2017
 *  @copyright Tymphany Ltd.
 */

#ifndef __HWSPIDRV_H__
#define __HWSPIDRV_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "deviceTypes.h"

typedef struct tagHWspiDrv{
    bool    isReady;
    bool    isBusy;
    tSpiDevice  *pConfig;
}cHWspiDrv;

/**
 * Construct the spi driver instance.
 * @param cHWspiDrv * me - driver to construct
 * @param tSpiDevice * config - parameters to initialise with.
 */
void HWspiDrv_Ctor(cHWspiDrv * me, tSpiDevice * config);

/*
 * Start driver instance.
 * @param cHWspiDrv * me - pointer to driver instance
 */
void HWspiDrv_Startup(cHWspiDrv * me);

/*
 * Exit & clean up the driver.
 * @param cHWspiDrv * me - pointer to driver instance
 */
void HWspiDrv_Xtor(cHWspiDrv * me);

/**
 * @param me - instance of the driver
 * @param pTxData - TX data buffer
 * @param pRxData - RX data buffer
 * @param len - bytes to read/write
 * @param timeout - failure timeout, unit : ms
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet HWspiDrv_WriteRead(cHWspiDrv * me, uint8_t *pTxData, uint8_t *pRxData, uint32_t len, uint32_t timeout);

#ifdef  __cplusplus
}
#endif

#endif  // __HWSPIDRV_H__

