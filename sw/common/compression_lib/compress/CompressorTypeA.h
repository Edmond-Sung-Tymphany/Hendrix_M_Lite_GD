/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        deCompressorTypeA.h
@brief       decompressot type A
@author      Alexey
@date        2016-01-18
@copyright (c) Tymphany Ltd. All rights reserved.

-------------------------------------------------------------------------------
*/
#include "../common/heatshrink_encoder.h"

typedef struct zipWriter
{
    heatshrink_encoder hse;
} zipWriter;

unsigned char CompressorTypeA_Compress(unsigned char *in_buffer, unsigned short in_buffer_len,
                              unsigned char *out_buffer, unsigned short *out_buffer_len);  

