/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        deCompressorTypeA.c
@brief       decompressot type A
@author      Alexey
@date        2016-01-18
@copyright (c) Tymphany Ltd. All rights reserved.

-------------------------------------------------------------------------------
*/

#include "deCompressorTypeA.h"
#include "../common/delta.h"
#include "../common/rle.h"

#define HS_PRINTF(...)

static struct zipReader glreader;

HSD_sink_res deCompressorTypeA_FillDecoderBuffer();

unsigned short deCompressorTypeA_GetBlockOffSet(unsigned short blockNum)
{
    unsigned short ii;
    unsigned short blockOffSet = 0;
    unsigned short offSet = 0;
    unsigned short bn = blockNum;

    blockOffSet = 2;
    offSet = 2;
    
    for (ii = 0; ii < bn; ii++)
    {  
        blockOffSet += 6;
        blockOffSet += glreader.compData[offSet + 0] << 8 | glreader.compData[offSet + 1];
        blockOffSet += glreader.compData[offSet + 2] << 8 | glreader.compData[offSet + 3];

        offSet = blockOffSet;
    }

    return blockOffSet;
}

unsigned short deCompressorTypeA_InitBlock(unsigned short blockNum) 
{
    unsigned short ii;
    unsigned short blockOffSet = 0;

    for (ii = 0; ii < 200; ii++)
    {
        HS_PRINTF(("\n ==glreader[%d] = 0x%x",ii,glreader.compData[ii]));
    }

    glreader.blockCount = glreader.compData[1];

    HS_PRINTF(("\nglreader.blockCount[%d]",glreader.blockCount));

    blockOffSet = deCompressorTypeA_GetBlockOffSet(blockNum);

    HS_PRINTF(("\nblockOffSet[%d]",blockOffSet));


    for (ii = 0; ii < 20; ii++)
    {
        HS_PRINTF(("\n glreader[%d] = 0x%x",ii,glreader.compData[blockOffSet + ii]));
    }
       
    glreader.curRegLength  = glreader.compData[blockOffSet + 0] << 8 | glreader.compData[blockOffSet + 1];
    glreader.curDataLength = glreader.compData[blockOffSet + 2] << 8 | glreader.compData[blockOffSet + 3];
    glreader.dataLeft = (HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE/2);
    glreader.w = glreader.compData[blockOffSet + 4];
    glreader.l = glreader.compData[blockOffSet + 5];
    glreader.curBlock = blockNum;

    glreader.curRegOffSet = blockOffSet + 6;
    glreader.curDataOffSet = glreader.curRegOffSet + glreader.curRegLength;

    config_set_w(glreader.w);
    config_set_l(glreader.l);
/*
    printf("\n curRegLength[%d]",glreader.curRegLength);
    printf("\n curDataLength[%d]",glreader.curDataLength);
    printf("\n dataLeft[%d]",glreader.dataLeft);
    printf("\n w[%d]",glreader.w);
    printf("\n l[%d]",glreader.l);
    printf("\n curBlock[%d]",glreader.curBlock);
    printf("\n curRegOffSet[%d]",glreader.curRegOffSet);
    printf("\n curDataOffSet[%d]",glreader.curDataOffSet);
    printf("\ncurRegLength[%d]",glreader.curRegLength);
*/
    for (ii = 0; ii < glreader.curRegLength; ii++)
    {
        glreader.regData[ii] = glreader.compData[glreader.curRegOffSet + ii];
        HS_PRINTF(("\nglreader->regData[%d] = %d",ii, glreader.regData[ii]));
    }

    rle_decode(glreader.regData, glreader.curRegLength);

    for (ii = 0; ii < HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE/2; ii++)
    {
        HS_PRINTF(("dec1 - [%d][%d]\n",ii,glreader.regData[ii]));
    }

    delta_decode(glreader.regData, HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE/2);

    for (ii = 0; ii < HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE/2; ii++)
    {
        HS_PRINTF(("dec2 - [%d][%d]\n",ii,glreader.regData[ii]));
    }    

    deCompressorTypeA_FillDecoderBuffer();

    return 1;
}

HSD_sink_res deCompressorTypeA_FillDecoderBuffer()
{
    unsigned short ii;
    HSD_sink_res sres;

    HSD_poll_res pres;
    unsigned short readSize = 0;
    unsigned short readSizeAll = 0;

    unsigned short dataSizeSunk = 0;
    unsigned short sinkSize = 0;

    unsigned short offSet = 0;

    for (ii = 0; ii < glreader.curDataLength; ii++)
    {
        HS_PRINTF(("Decode_1[%d]=[0x%x]\n",ii,glreader.compData[glreader.curDataOffSet + dataSizeSunk + ii]));
    }
    
    do
    {
        sres = heatshrink_decoder_sink(&glreader.hsd, &glreader.compData[glreader.curDataOffSet + dataSizeSunk], glreader.curDataLength - dataSizeSunk, &sinkSize);
        HS_PRINTF(("\nsinkSize[%d]sres[%d]glreader.curDataLength[%d]",sinkSize,sres,glreader.curDataLength));
        dataSizeSunk += sinkSize;
    } while (dataSizeSunk < glreader.curDataLength);

    return sres;
}

HSD_poll_res deCompressorTypeA_ReadDecodedData(unsigned char *data)
{
    HSD_poll_res pres;
    unsigned short readSize = 0;
    unsigned short readSizeAll = 0;
    unsigned char *decodedData = data;
    unsigned short ii;

    static unsigned short counter = 0;
    
    decodedData[0] = glreader.regData[HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE/2 - glreader.dataLeft];

    do
    {
        pres = heatshrink_decoder_poll(&glreader.hsd, &decodedData[readSizeAll + 1], 1, &readSize);
        readSizeAll += readSize;
    } while (readSizeAll < 1 && pres == HSDR_POLL_MORE);
   // printf("\nreadSizeAll[%d]pres[%d]dataLeft[%d]counter[%d]",readSizeAll,pres,glreader.dataLeft,counter);

    glreader.dataLeft -= readSizeAll;

    HS_PRINTF(("\npres[%d],glreader.dataLeft[%d]",pres,glreader.dataLeft));
    HS_PRINTF(("",decodedData[0]));
    HS_PRINTF(("[0x%02x]",decodedData[1]));
    counter++;

    if (0 == glreader.dataLeft)
    {
        if (++glreader.curBlock < glreader.blockCount)
        {
            HS_PRINTF(("\nInit next block start[%d]",glreader.curBlock));

            //heatshrink_decoder_finish(&glreader.hsd);
            heatshrink_decoder_reset(&glreader.hsd);
            
            deCompressorTypeA_InitBlock(glreader.curBlock);   
            HS_PRINTF(("\nInit next block done"));
        }
        else
        {
            return HSDR_POLL_EMPTY;
        }
    }      
    return pres;
}

void deCompressorTypeA_Create(const unsigned char* array)
{
    unsigned char ii;
    
    heatshrink_decoder_reset(&glreader.hsd);
    
    glreader.compData = array;

    deCompressorTypeA_InitBlock(0);
}

unsigned char deCompressorTypeA_Read(unsigned char* data)
{   
    return deCompressorTypeA_ReadDecodedData(data);
}

