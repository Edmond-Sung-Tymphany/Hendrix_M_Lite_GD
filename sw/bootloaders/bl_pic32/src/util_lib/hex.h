/**
 * @file      hex.h
 * @brief     The header source file for hex file parsing
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef HEX_H
#define HEX_H



/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <plib.h>
#include <GenericTypeDefs.h>
#include <sys/types.h> //ssize_t
#include "Bootloader.h"

/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/
#define HEX_HEADER_LEN   5   //Data Len(1byte) + type(1byte) + address(2bytes) + CRC(1bytes)
#define HEX_DATA_LEN_MAX 255
#define HEX_RECORD_LEN_MAX (HEX_DATA_LEN_MAX+HEX_HEADER_LEN)


/*****************************************************************************
 * Function Prototype                                                        *
 *****************************************************************************/
ssize_t WriteHexRecord2Flash(uint8* HexRecord, uint32 totalHexRecLen, uint32 hex_line);



#endif    /* HEX_H */

