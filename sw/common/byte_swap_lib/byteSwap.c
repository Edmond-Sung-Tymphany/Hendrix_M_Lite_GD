/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Byte Swap lib
                  -------------------------

                  SW Module Document




@file        byteSwap.c
@brief       This file implements the basic byte swap API
@author      Daniel Qin
@date        2015-7-28
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2015-7-28     Daniel Qin
SCO/ERROR  :
-------------------------------------------------------------------------------
*/
#include "byteSwapLib.h"

unsigned short _byteswap_ushort(unsigned short i)
{
    unsigned short j;
    j =  (i << 8) ;
    j += (i >> 8) ;
    return j;
}

unsigned long _byteswap_ulong(unsigned long i)
{
    unsigned int j;
    j =  (i << 24);
    j += (i <<  8) & 0x00FF0000;
    j += (i >>  8) & 0x0000FF00;
    j += (i >> 24);
    return j;
}

uint64_t _byteswap_uint64(uint64_t i)
{
    uint64_t j;
    j =  (i << 56);
    j += (i << 40)&0x00FF000000000000;
    j += (i << 24)&0x0000FF0000000000;
    j += (i <<  8)&0x000000FF00000000;
    j += (i >>  8)&0x00000000FF000000;
    j += (i >> 24)&0x0000000000FF0000;
    j += (i >> 40)&0x000000000000FF00;
    j += (i >> 56);
    return j;

}
