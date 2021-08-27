/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/
/**
 *  @file      I2CDrv.h
 *  @brief     This file presents low level interface to I2C driver
 *              and reference to specific port
 *  @author    Dmitry Abdulov
 *  @date      19-Aug-2013
 *  @copyright Tymphany Ltd.
 */

#ifndef I2CDRV_H
#define	I2CDRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "deviceTypes.h"



/*____________________________________________*/

//#define I2C_SLAVE_SUPPORT_ON                       1


typedef struct 
{
    uint8 devAddr;     /* device address*/
    uint32 regAddr;
    uint16 length;
    uint8* pMsg;
}tI2CMsg;


/* Class definition */

CLASS(cI2CDrv)
    uint8 registeredUser; // if registeredUser == 0, it means the no one use I2C driver.
    uint16 driverID;
    tI2CDevice *pConfig;
    bool isReady;
    bool stopEnable; //TRUE means use Repeated-START to replace STOP+START. Default FALS
METHODS
/* PUBLIC FUNCTION PROTOTYPES */
/**
 * Construct the i2c driver instance.
 * @param cI2CDrv * me - driver to construct
 * @param tI2CBus * config - parameters to initialise with.
 */
void I2CDrv_Ctor(cI2CDrv * me, tI2CDevice * config);


/*
 * get i2c baud rate - value of I2C1BRG register
 * @param cI2CDrv * me - pointer to driver instance
 * return I2C1BRG value
 */
uint32 I2CDrv_GetBR(cI2CDrv * me);

/*
 * set i2c baud rate - update I2C1BRG register
 * @param cI2CDrv * me - pointer to driver instance
 * @param updated I2C1BRG value
 */
void I2CDrv_SetBR(cI2CDrv * me, uint32 baudrate);

/*
 * Start driver instance.
 * @param cI2CDrv * me - pointer to driver instance
 */
void I2CDrv_Startup(cI2CDrv * me);

/*
 * Exit & clean up the driver.
 * @param cI2CDrv * me - pointer to driver instance
 */
void I2CDrv_Xtor(cI2CDrv * me);


/* Set if send stop bit on end of serial commands
 * parameter stopEnable:
 *    TRUE: send STOP on end of serial commands
 *    FALSE: do not send STOP on end of serial commands
 */
void I2CDrv_SetStopOperation(cI2CDrv * me, bool stopEnable);


/*
 * Check if the driver is in a ready state.
 * @param cI2CDrv * me - pointer to driver instance
 * return driver instance status
 * TRUE  : driver is on and ready for read\write etc commands
 * FALSE : driver is off\or busy
 */
bool I2CDrv_isReady(const cI2CDrv * me);

/*
 * @param cI2CDrv * me - pointer to driver instance
 * return driver instance id
 */
uint16 I2CDrv_getID(const cI2CDrv * me);

/*
 * reset I2C driver
 * @param cI2CDrv * me - pointer to driver instance
 */
void I2CDrv_Reset(const cI2CDrv * me);

eTpRet I2CDrv_MasterWrite(cI2CDrv * me, tI2CMsg const * const  msg);
eTpRet I2CDrv_MasterRead(cI2CDrv * const  me, tI2CMsg *  msg);


/*
 * This function sends a variable length uint8 array over the i2C bus in master mode
 * The chip address is one byte, register address is two bytes, data are in byte
 * @param cI2CDrv * me - pointer to driver instance
 * @param tI2CMsg * const pI2CData - msg representing structure: length of the array to be sent.
 * and pointer to a uint8 array of size length to be sent over the I2C bus
 * @return TRUE on success, otherwise FALSE
 */
eTpRet I2CDrv_MasterWriteWith2ByteRegAddress(cI2CDrv * me, tI2CMsg * const msg);

/*
 * This function reads a variable length uint8 array over the i2C bus in master mode
 * The chip address is one byte, register address is two bytes, data are in byte
 * @param cI2CDrv * const me - pointer to driver instance
 * @param tI2CMsg * const  msg representing structure: length of the array to be read
 * and pointer to a uint8 array of size length to be read from the I2C bus
 * @return TRUE if sucess, otherwise FALSE
 */
eTpRet I2CDrv_MasterReadWith2ByteRegAddress( cI2CDrv * const me, tI2CMsg * msg);


END_CLASS

#ifdef I2C_SLAVE_SUPPORT_ON
/*
 * TODO: to be implemented
 * This function sends a variable length uint8 array over the i2C bus in slave mode
 */
void I2CDrv_SlaveWrite(const cI2CDrv * me, ...);
/*
 * TODO: to be implemented
 * This function reads a variable length uint8 array over the i2C bus in slave mode 
 */
void I2CDrv_SlaveRead(const cI2CDrv * me, ...);
#endif



#ifdef	__cplusplus
}
#endif

#endif	/* I2CDRV_H */

