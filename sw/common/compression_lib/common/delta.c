/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        deltaDecompression.c
@brief       deltaDecompression
@author      Alexey
@date        2016-01-18
@copyright (c) Tymphany Ltd. All rights reserved.

-------------------------------------------------------------------------------
*/

void delta_encode(unsigned char *buffer, unsigned short length)
{
    unsigned char last = 0;
    for (unsigned short ii = 0; ii < length; ii++)
    {
        unsigned char current = buffer[ii];
        buffer[ii] = current - last;
        last = current;
    }
}

void delta_decode(unsigned char *buffer, unsigned short length)
{
    unsigned char last = 0;
    for (unsigned short ii = 0; ii < length; ii++)
    {
        unsigned char delta = buffer[ii];
        buffer[ii] = delta + last;
        last = buffer[ii];
    }
}


