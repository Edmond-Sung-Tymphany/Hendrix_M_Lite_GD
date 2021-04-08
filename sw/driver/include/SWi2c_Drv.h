#ifndef __SWI2C_DRV_H__
#define __SWI2C_DRV_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "deviceTypes.h"

typedef struct tagSWi2cDrv
{
    bool    isReady;
    bool    isBusy;
    bool    devReady;      // device available?
    stSWi2cDevice_t  *pConfig;
}cSWi2cDrv_t;

/**
 * Construct the s/w i2c driver instance.
 * @param me - instance of the driver
 * @param pConfig pointer to the config structure
 */
void SWi2cDrv_Ctor(cSWi2cDrv_t *me, stSWi2cDevice_t *pConfig);

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void SWi2cDrv_Xtor(cSWi2cDrv_t * me);

/**
 * This function detect whether the i2c device is available or not?
 * @param me - instance of the driver
 * @param retry_cnt - how many times to try the i2c comminucation?
 * @return TRUE on success, otherwise FALSE
 */
bool SWi2cDrv_DeviceAvailable(cSWi2cDrv_t * me, uint32_t retry_cnt);

/**
 * This function sends one byte over the I2C bus in master mode
 * @param me - instance of the driver
 * @param addr - register address
 * @param data - data for transmit
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SWi2cDrv_WriteByte(cSWi2cDrv_t * me, uint8_t addr, uint8_t data);

/**
 * This function sends a variable length uint8 array over the I2C bus in master mode
 * @param me - instance of the driver
 * @param addr - register address
 * @param pArr - data array to be sent
 * @param len - how many bytes should be sent
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SWi2cDrv_WriteArray(cSWi2cDrv_t * me, uint8_t addr, uint8_t *pArr, uint32_t len);

/**
 * This function read one byte from the I2C bus in master mode
 * @param me - instance of the driver
 * @param addr - register address
 * @param p_data - store the data
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SWi2cDrv_ReadByte(cSWi2cDrv_t * me, uint8_t addr, uint8_t *p_data);

/**
 * This function read a variable length uint8 array over the I2C bus in master mode
 * @param me - instance of the driver
 * @param addr - register address
 * @param p_data - store the data
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SWi2cDrv_ReadArray(cSWi2cDrv_t * me, uint8_t addr, uint8_t *pArr, uint32_t len);

#ifdef  __cplusplus
}
#endif

#endif	// __SWI2C_DRV_H__

