/**
 * @file        NBI2CDrv.h
 * @brief       This file defines the implementation of non-blcoking I2C driver
 * @author      Bob.Xu 
 * @date        2014-04-25
 * @copyright   Tymphany Ltd.
 */

#ifndef NBI2C_DRIVER_H
#define NBI2C_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"

typedef enum
{
    I2C_TRM_STAGE_DONE = 0,
    I2C_TRM_STAGE_WRITE_DATA,
    I2C_TRM_STAGE_WRITE_DEV_ADDRESS_W,
    I2C_TRM_STAGE_WRITE_DEV_ADDRESS_R,
    I2C_TRM_STAGE_WRITE_REG_ADDRESS_L,
    I2C_TRM_STAGE_WRITE_REG_ADDRESS_H,
    I2C_TRM_STAGE_REPEAT_START,
    I2C_TRM_STAGE_CHECK_I2C_SLAVE_ACK,
    I2C_TRM_STAGE_MASTER_ACK_TRM,
    I2C_TRM_STAGE_READ_DATA,
    I2C_TRM_STAGE_MASTER_ACK,
    I2C_TRM_STAGE_STOP
} eNBI2CTrmStage; // transmission stages

typedef enum
{
    I2C_NO_FAULT = 0,
    I2C_BUS_BUSY,
    I2C_TIMEOUT,
    I2C_PERMANENT_BUS_FAULT,
    I2C_ACK_ERROR
}eNBI2CFault;

typedef enum
{
    I2C_DATA_INVALID_MODE = 0,
    I2C_DATA_READ_MODE,
    I2C_DATA_WRITE_MODE,
}eNBI2CDataMode;

typedef struct 
{
    uint8 devAddr;     /* device address*/
    uint32 regAddr;
    uint8 length;
    uint8* pMsg;
}tNBI2CMsg;

typedef void (*pNBI2CCb)(eNBI2CFault nbI2CFault);

CLASS(cNBI2CDrv)
    /* private data */
    eNBI2CTrmStage I2CTrmStage;
    eNBI2CDataMode i2cDataMode;
    eNBI2CFault eI2CFault;
    uint16 dataIndex;
    tI2CDevice config;
    pNBI2CCb nbI2CWriteCb;
    pNBI2CCb nbI2CReadCb;
    bool isIdle;
METHODS
    /* public functions */
void NBI2CDrv_Ctor(cNBI2CDrv * me, const tI2CDevice * pConfig);

void NBI2CDrv_Xtor(cNBI2CDrv * me);

void NBI2CDrv_MasterWrite(cNBI2CDrv * me, tNBI2CMsg * const  msg);

END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* NBI2C_DRIVER_H */

