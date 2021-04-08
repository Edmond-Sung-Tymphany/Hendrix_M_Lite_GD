/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  64bit shift lib
                  -------------------------

                  SW Module Document




@file        64bit_shift.c
@brief       This file implements the 64bit shift APIs
@author      Daniel Qin
@date        2017-12-26
@copyright (c) Tymphany Ltd. All rights reserved.

-------------------------------------------------------------------------------
*/
#include "bit_shift_64.h"


unsigned long long shift_left64(unsigned long long d, unsigned char len)
{
    unsigned long *p = (unsigned long*) &d;
    if (len < 32)
    {
        *(p+1) <<= len;
        unsigned long tmp = (*p) >> (32-len);
        *(p+1) |= tmp;
        *p <<= len;
    }
    else
    {
        *(p+1) = *p;
        *p = 0UL;
        *(p+1) <<= (len-32);
    }
    return d;
}

unsigned long long shift_right64(unsigned long long d, unsigned char len)
{
    unsigned long *p = (unsigned long*) &d;
    if (len < 32)
    {
        *p >>= len;
        unsigned long tmp = *(p+1) << (32-len);
        *p |= tmp;
        *(p+1) >>= len;
    }
    else
    {
        *p = *(p+1);
        *(p+1) = 0UL;
        *p >>= (len-32);
    }
    return d;
}

