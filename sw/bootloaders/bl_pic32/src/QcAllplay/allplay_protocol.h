/**************************************************************
 * Copyright (C) 2013, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

#ifndef PROTOCOL_H_
#define PROTOCOL_H_


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "BootLoader.h" //each file must include Bootloader.h


/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/
// Tymphany define the MTU for mcuUpdate protocol
#define MAX_DATA_SIZE (2048)

//Timeout for whole upgrade process
//If app flash is broken, bootloader will ignore this timeout, and wait RESET forever
#define UPDATE_TIMEOUS_MS 5*60*1000 //5min

//Timeout between two data
#define DATA_1ST_TIMEOUT_MS 1000 //1sec

//Timeout between two data
#define DATA_TIMEOUT_MS 4000 //4sec

//Timeout between two data
#define REBOOT_TIMEOUT_MS 4000 //4sec

//Print packet recieve/send
//#define LOG_PACKET


/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
#if defined(LOG_PACKET)
extern int logPacket;
#endif // LOG_PACKET

/*****************************************************************************
 * Function Protocol                                                          *
 *****************************************************************************/
uint32_t getResetTimeout();
void setResetTimeout(uint32_t timeout);
uint32_t getAckTimeout();
void setAckTimeout(uint32_t timeout);
size_t getMaxDataSize(void);
int32 writeReset(void);
uint8 *getDataBuffer(void);
int32 writeData(size_t outLen); // returns: -1: error, 1: success

// Return data from input queue. Data is valid only until the next call to a
// protocol function.
//
// buffer: will get the pointer to the data array
// len: will get the size of the data in buffer
// returns: -1: error, 0: timeout, 1: has data
int32 readData(uint8 **buffer, size_t *len, int32 timeout);

// Return data from input queue if any
//
// buffer: will get the pointer to the data array
// len: will get the size of the data in buffer
// returns: -1: error, 0: timeout, 1: has data
int32 peekData(uint8 **buffer, size_t *len);


#endif /* PROTOCOL_H_ */
