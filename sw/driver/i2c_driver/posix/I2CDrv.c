/**
 *  @file      I2CDrv.c
 *  @brief     
 *  @author    
 *  @date      
 *  @copyright Tymphany Ltd.
 */
#include "commonTypes.h"
#include "I2CDrv.h"
#include "./I2CDrv_priv.h"
#include "trace.h"
#include "assert.h"

#define I2C_TIMEOUT     0xffff;
uint32 i2c_Timeout = I2C_TIMEOUT;

/*============================================================================*/
/* Forward declaration of private functions */
/*============================================================================*/
/* PUBLIC FUNCTIONS */

/**
 * Construct the i2c driver instance.
 * @param me - instance of the driver
 * @param pConfig pointer to the config structure
 */
void I2CDrv_Ctor(cI2CDrv * me, tI2CDevice * pConfig)
{
    (void) me;
    (void) pConfig;
}

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void I2CDrv_Xtor(cI2CDrv * me)
{
    (void) me;
}

/**
 * This function sends a variable length uint8 array over the i2C bus in master mode
 * @param me - instance of the driver
 * @param tI2CMsg const * const  msg
 *      representing structure: length of the array to be sent
 *      and pointer to a uint8 array of size length to be sent over the I2C bus
 * @return TRUE on success, otherwise FALSE
 */
eTpRet I2CDrv_MasterWrite(cI2CDrv * me, tI2CMsg const * const  msg)
{
    (void) me;
    (void) msg;
#ifdef I2C_DEBUG
    int size = msg->length;
    int i = 0;
    uint8* p = msg->pMsg;

    printf("I2C sending %d data to [%d, %d]: \r\n", 
        size, msg->devAddr, msg->regAddr);
    for ( ; i < size; ++i)
    {
        printf("0x%02X ", *(p+i));
        if ((i & 0xF) == 0xF)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
#endif
    return TP_SUCCESS;
}

/**
 * This function reads a variable length uint8 array over the i2C bus in master mode
 * @param me - instance of the driver
 * @param tI2CMsg * const  msg representing structure: length of the array to be read
 * and pointer to a uint8 array of size length to be read from the I2C bus
 * @return TRUE if sucess, otherwise FALSE
 */
eTpRet I2CDrv_MasterRead(cI2CDrv * const  me, tI2CMsg *  msg)
{
    (void) me;
    (void) msg;
    return TP_SUCCESS;
}

eTpRet I2CDrv_MasterWriteWith2ByteRegAddress(cI2CDrv * me, tI2CMsg * const msg)
{
    (void) me;
    (void) msg;
    return TP_SUCCESS;
}

/**
 * This function reads a variable length uint8 array over the i2C bus in master mode
 * @param me - instance of the driver
 * @param tI2CMsg * const  msg representing structure: length of the array to be read
 * and pointer to a uint8 array of size length to be read from the I2C bus
 * @return TRUE if sucess, otherwise FALSE
 */
eTpRet I2CDrv_MasterReadWith2ByteRegAddress( cI2CDrv * const me, tI2CMsg * msg)
{
    (void) me;
    (void) msg;
    return TP_SUCCESS;
}

/*============================================================================*/
/* PRIVATE FUNCTIONS */
/*============================================================================*/


