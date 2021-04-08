/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        deCompressorTypeA.h
@brief       decompressot type A
             Compression library component design documentation:
             http://flow.tymphany.com/redmine/projects/tooling/dmsf?folder_id=16099
@author      Alexey
@date        2016-01-18
@copyright (c) Tymphany Ltd. All rights reserved.

-------------------------------------------------------------------------------
*/
#include "../common/heatshrink_decoder.h"

typedef struct zipReader
{
    unsigned short blockCount;
    unsigned short curRegLength;
    unsigned short curDataLength;
    unsigned short curRegOffSet;
    unsigned short curDataOffSet;
    unsigned short curBlock;
    unsigned short dataLeft;
    unsigned char w;
    unsigned char l;
    unsigned char regData[HEATSHRINK_STATIC_TYPE_A_BLOCK_SIZE/2];
    const unsigned char *compData;
    heatshrink_decoder hsd;
} zipReader;

void deCompressorTypeA_Create(const unsigned char* array);
unsigned char deCompressorTypeA_Read(unsigned char* data);


