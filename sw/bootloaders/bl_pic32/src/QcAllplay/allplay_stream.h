/**************************************************************
 * Copyright (C) 2013, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

/**
 * @file
 * @brief Low level functions to access/query a serial device.
 */

#ifndef STREAM_H_
#define STREAM_H_

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "Bootloader.h"
#include "allplay_common.h"



/*****************************************************************************
 * Function Protocol                                                         *
 *****************************************************************************/
/**
 * Return current time in milliseconds. #getTime is intended only to compute
 * time period/duration. As such, the origin is undefined and implementation
 * should avoid significant jumps forward/backward in the time value.
 *
 * @return current time in milliseconds
 */
uint32 getTime(void);

/**
 * Open serial device.
 *
 * @retval 0 on success
 * @retval -1 on failure
 */
int32 openDevice(void);

/**
 * Close serial device.
 */
void closeDevice(void);

/**
 * Wait for some input data for up to @c ms milliseconds.
 *
 * @param ms maximum time to wait in milliseconds, -1 for infinity
 * @retval 1 if data is available
 * @retval 0 if timeout
 * @retval -1 if error
 */
int32 waitInput(ssize_t ms);

/**
 * Wait for the output to be ready to send more data for up to @c ms milliseconds.
 *
 * @param ms maximum time to wait in milliseconds, -1 for infinity
 * @retval 1 if ready
 * @retval 0 if timeout
 * @retval -1 if error
 */
int32 waitOutput(ssize_t ms);

/**
 * Read any available data from serial device.
 * #readStream does not block and will return once @c buf is full or all
 * currently available data has been read.
 *
 * @param buf buffer where to put the data
 * @param len size of the buffer
 * @return the amount of data put in the buffer, -1 if error
 */
ssize_t readStream(uint8 *buf, size_t len);

/**
 * Write some data to serial device.
 * #writeStream does not block and will return once the serial device's buffer
 * is full or @c buf is empty.
 *
 * @param buf buffer data to write
 * @param len amount of data to write
 * @return the amount of data actually sent, -1 if error
 */
ssize_t writeStream(uint8 *buf, size_t len);

#endif /* STREAM_H_ */
