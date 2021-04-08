/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Byte Swap lib
                  -------------------------

                  SW Module Document




@file        byteSwap.h
@brief       This file implements the basic byte swap API
@author      Daniel Qin
@date        2015-7-28
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2015-7-28     Daniel Qin
SCO/ERROR  :
-------------------------------------------------------------------------------
*/
#ifndef BYTE_SWAP_LIB_H
#define BYTE_SWAP_LIB_H
#include "stdint.h"

unsigned short _byteswap_ushort(unsigned short i);
unsigned long _byteswap_ulong(unsigned long i);
uint64_t _byteswap_uint64(uint64_t i);

#endif