/**
*  @file      I2CDmaDrv.h
*  @brief     This file contains functions for I2C master read or write in DMA mode
*  @version   v0.2
*  @author    Gavin Lee , Alex.Li
*  @date      2017/8/4
*  @copyright Tymphany Ltd.
*/

#ifndef I2CDMADRV_H
#define I2CDMADRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "deviceTypes.h"
#include "commonTypes.h"

typedef enum
{
    I2cCbRet_WriteSuccess,//transmit success
    I2cCbRet_ReadSuccess,
    I2cCbRet_WriteDmaError,
    I2cCbRet_ReadDmaError,
    I2cCbRet_BuffFullError,
    I2cCbRet_NACK,//Not Acknowledge received
    I2cCbRet_Err,//Red Alert Error I2C BUS NEED TO RESET
    I2cCbRet_Timeout//Red Alert Error I2C BUS NEED TO RESET
} eI2cCbRet;


/* Defines the prototype to I2CDma callback function */
typedef void (*pdI2CDma_CALLBACK)(void *, eI2cCbRet);

CLASS(cI2CDmaDrv)
uint8 registeredUser; // if registeredUser == 0, it means the no one use I2C driver.
uint16 driverID;
tI2CDmaDevice *pConfig;
bool isReady;
pdI2CDma_CALLBACK pvI2CDmaCallback;//Callback function should be short
METHODS

typedef struct
{
    cI2CDmaDrv * owner;
    uint8 devAddr;     /*should be a 8-bit addr*/
    uint32 regAddr;    /*only useful for read*/
    eI2CRegAddLen regAddrLen;/*only useful for read */
    uint16 length;
    uint8* pMsg;       /*warning memory of msg should not be freed until transmit completed*/
    volatile bool bIsFinish;
} tI2CDmaMsg;

/* PUBLIC FUNCTION PROTOTYPES */
/**
* Construct the i2c_dma driver instance.
* @param cI2CDmaDrv * me - driver to construct
* @param tI2CDmaDevice * config - parameters to initialise with.
*/
void I2CDmaDrv_Ctor(cI2CDmaDrv * me, tI2CDmaDevice * pConfig,
                    pdI2CDma_CALLBACK callback);
void I2CDmaDrv_Xtor(cI2CDmaDrv * me);
eTpRet I2CDmaDrv_MasterWrite(cI2CDmaDrv * me, tI2CDmaMsg const * const  msg);
eTpRet I2CDmaDrv_MasterRead(cI2CDmaDrv * const me, tI2CDmaMsg *  msg);
int16 I2CDmaDrv_CheckRingBuf(cI2CDmaDrv * const me);
END_CLASS

#endif
