/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        rleDecompression.c
@brief       rleDecompression
@author      Alexey
@date        2016-01-18
@copyright (c) Tymphany Ltd. All rights reserved.

-------------------------------------------------------------------------------
*/

#include "heatshrink_config.h"

unsigned char rle_encode(unsigned char *buffer, unsigned short *length)
{
    unsigned short ii;

    unsigned char currentChar = 0;
    unsigned char bufferEncoded[HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE] = {0};
    
    unsigned short repeatNumber = 0;
    unsigned short currentPositionOut = 0;

    currentChar = buffer[0];
    repeatNumber = 1;

    for (unsigned short ii = 1; ii < (*length); ii++)
    {        
        if (currentChar != buffer[ii])
        {            
            bufferEncoded[currentPositionOut] = currentChar;
            bufferEncoded[currentPositionOut + 1] = repeatNumber;

            currentChar = buffer[ii];
            repeatNumber = 1;

            currentPositionOut += 2;
            if (currentPositionOut > HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE)
            {
                return 0;
            }
        }
        else
        {
            repeatNumber++;
        }     
    }

    bufferEncoded[currentPositionOut] = currentChar;
    bufferEncoded[currentPositionOut + 1] = repeatNumber;

    currentPositionOut += 2;
    if (currentPositionOut > HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE)
    {
        return 0;
    }

    for (unsigned short ii = 0; ii < *length; ii++)
    {
        buffer[ii] = bufferEncoded[ii];
    }

    *length = currentPositionOut;

    return 1;
}

void rle_decode(unsigned char *buffer, unsigned short length)
{
    unsigned short ii;
    unsigned short zz;

    unsigned char bufferTemp[HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE] = {0};
    unsigned short currentPositionOut = 0;

    for (unsigned short ii = 0; ii < length; ii++)
    {
        bufferTemp[ii] = buffer[ii];
    }

    for (unsigned short ii = 0; ii < length; ii += 2)
    {         
        for (unsigned short zz = 0; zz < bufferTemp[ii + 1]; zz++)
        {
            buffer[currentPositionOut++] = bufferTemp[ii];
        }
    }
}

