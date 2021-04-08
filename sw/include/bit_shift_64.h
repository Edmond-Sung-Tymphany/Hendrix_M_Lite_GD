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
#ifndef BIT_SHIFT_64_H
#define BIT_SHIFT_64_H
#include "stdint.h"

unsigned long long shift_left64(unsigned long long d, unsigned char len);
unsigned long long shift_right64(unsigned long long d, unsigned char len);

#endif
