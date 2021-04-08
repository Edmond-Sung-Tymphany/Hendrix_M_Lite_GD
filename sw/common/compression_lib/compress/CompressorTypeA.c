/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        CompressorTypeA.c
@brief       compressor type A
@author      Alexey
@date        2016-01-18
@copyright (c) Tymphany Ltd. All rights reserved.

-------------------------------------------------------------------------------
*/

#include "CompressorTypeA.h"
#include "../common/delta.h"
#include "../common/rle.h"

#define HS_PRINTF(...)

static struct zipWriter glwriter;

unsigned char CompressorTypeA_CompressReg(char* data, unsigned short *length)
{
    int ii;

    for (ii = 0; ii < *length; ii++)
    {
        HS_PRINTF(("data0[%d][%d]\n",ii,data[ii]));
    } 
    delta_encode(data, (*length));

    for (ii = 0; ii < *length; ii++)
    {
        HS_PRINTF(("data1[%d][%d]\n",ii,data[ii]));
    } 

    if (0 == rle_encode(data, length))
    {
        return 0;
    }

    for (ii = 0; ii < *length; ii++)
    {
        HS_PRINTF(("data2[%d][%d]\n",ii,data[ii]));
    }
    return 1;
}


void CompressorTypeA_CompressData(unsigned char* data, unsigned short *length, unsigned char* out)
{
    unsigned short sink_sz;
    unsigned short poll_sz;

    unsigned short sunk = 0;
    unsigned short polled = 0;

    HSE_poll_res pres;
    HSE_sink_res sres;
    HSE_finish_res fres;

    unsigned short dataLength = *length;

    unsigned long ii = 0;

    static char oo = 0;
    
    heatshrink_encoder_reset(&glwriter.hse);

    if (oo == 0)
    {
        oo = 1;

        for (ii = 0; ii < 256; ii++)
        {
            HS_PRINTF(("\n databefore comp[%d] = 0x%02x",ii,data[ii]));           
        }

    }

    while(1)
    {
        HS_PRINTF(("\n(*length - sunk)[%d]\n",(dataLength - sunk)));
        if (sunk < dataLength)
        {
            sres = heatshrink_encoder_sink(&glwriter.hse, &data[sunk], (dataLength - sunk), &sink_sz);
            sunk += sink_sz;
            

            HS_PRINTF(("\nsink_sz[%d]sunk[%d]sres[%d]\n",sink_sz,sunk,sres));
        }

        do
        {
            pres = heatshrink_encoder_poll(&glwriter.hse, &out[polled], (1 << HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE_BITS-1), &poll_sz);
            polled += poll_sz;
            HS_PRINTF(("\npoll_sz[%d]polled[%d]pres[%d][%d]",poll_sz,polled,pres,(pres == HSER_POLL_MORE)));

        } while (pres == HSER_POLL_MORE);

        if ((poll_sz == 0) && (sunk == dataLength))
        {
            fres = heatshrink_encoder_finish(&glwriter.hse);
            if (fres == HSER_FINISH_DONE)
            {
                break;
            }
        }
    }

    if (oo == 1)
    {
        oo = 2;
        for (ii = 0; ii < polled; ii++)
        {
            HS_PRINTF(("\n dataAfter comp out[%d] = 0x%02x",ii,out[ii]));           
        }
    }

    *length = polled;
}

unsigned char CompressorTypeA_Compress(unsigned char *in_buffer, unsigned short in_buffer_len,
                                 unsigned char *out_buffer, unsigned short *out_buffer_len)
{
    unsigned char  blockCount;
    unsigned short blockSize, ii, zz, regLength, dataLength, regLength1, dataLength1, offSet;
    static unsigned char Reg     [HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE/2] = {0};
    static unsigned char Data    [HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE/2] = {0};
    static unsigned char DataComp[HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE/2] = {0};

    if (0 != (in_buffer_len % HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE))
    {
        blockCount = (in_buffer_len / HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE) + 1;
    }
    else
    {
        blockCount = (in_buffer_len / HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE);
    }
    
    HS_PRINTF(("\nNotcompressed All length[%d]",in_buffer_len));
    HS_PRINTF(("\nBlockCount[%d]",blockCount));
    
    out_buffer[0] = 0x0A;
    out_buffer[1] = blockCount;
    
    offSet = 2;
    
    
    for (ii = 0; ii < HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE; ii++)
    {
        HS_PRINTF(("\nin_buffer[%d] = %d",ii,in_buffer[ii]));           
    }
    for (zz = 0; zz < blockCount; zz++)
    {
    
        if (zz == (blockCount - 1))
        {
            blockSize  = (in_buffer_len - HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE * zz);
            HS_PRINTF(("\nLastBlockSize[%d]",blockSize));
        }
        else
        {
            blockSize  = HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE;
        }
    
        for (ii = 0; ii < (blockSize/2); ii++)
        {
    
            HS_PRINTF(("\n[%d]",zz * HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE + 2 * ii + 1));
            Reg[ii]  = in_buffer[zz * HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE + 2 * ii];
            Data[ii] = in_buffer[zz * HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE + 2 * ii + 1];
        }
    
        if (zz == 0)
        {
            for (ii = 0; ii < 256; ii++)
            {
                HS_PRINTF(("\nData[%d] = %d", ii, Data[ii]));           
            }
        }
    
        regLength = blockSize / 2;
        dataLength = blockSize / 2;
    
        HS_PRINTF(("\nnot_compressed reg  length[%d]",regLength));
        HS_PRINTF(("\nnot_compressed data length[%d]\n",dataLength));

        regLength1 = regLength;
        dataLength1 = dataLength;
    
        if (0 == CompressorTypeA_CompressReg(Reg, &regLength))
        {
            return 0;
        }
        CompressorTypeA_CompressData(Data, &dataLength, DataComp);
        if (dataLength > HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE/2)
        {
            return 0;
        }



        HS_PRINTF(("\ncompressed reg  length[%d][%d%%]",regLength,(regLength*100)/regLength1));
        HS_PRINTF(("\ncompressed data length[%d][%d%%]\n",dataLength,(dataLength*100)/dataLength1));
        out_buffer[offSet + 0] = (char)((regLength >> 8) & 0xFF);
        out_buffer[offSet + 1] = (char)(regLength & 0xFF);
        out_buffer[offSet + 2] = (char)((dataLength >> 8) & 0xFF);
        out_buffer[offSet + 3] = (char)(dataLength & 0xFF);
        out_buffer[offSet + 4] = (char)config_get_w();
        out_buffer[offSet + 5] = (char)config_get_l();
        
        for (ii = 0; ii < regLength; ii++)
        {
            out_buffer[offSet + 6 + ii] = Reg[ii];
        }
        for (ii = 0; ii < dataLength; ii++)
        {
            out_buffer[offSet + 6 + regLength + ii] = DataComp[ii]; 
    
            if (zz == 1)
            {
               //aprintf("\nBlock_1_comp:[%d][0x%02x]",ii,compressedFileAll[offSet + 6 + regLength + ii]);           
            }
        }
    
        offSet = offSet + 6 + regLength + dataLength;
        HS_PRINTF(("\n***offSet[%d]", offSet));
    }
    *out_buffer_len = offSet;
    return 1;
}

