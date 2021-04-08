/**************************************************************
 * Copyright (C) 2013, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

#ifndef CRC_CCITT_H_
#define CRC_CCITT_H_


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "BootLoader.h" //each file must include Bootloader.h
#include "allplay_common.h"


/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
#define CRC_INIT 0xFFFF;
typedef uint16 Crc;


/*****************************************************************************
 * Function Protocol                                                         *
 *****************************************************************************/
void computeCrc(const uint8* buf, size_t len, Crc* runningCrc);

#endif /* CRC_CCITT_H_ */
