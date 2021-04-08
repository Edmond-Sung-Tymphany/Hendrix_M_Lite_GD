/**
*  @file      tool.c
*  @brief    public tools
*  @author    Johnny Fan
*  @date      8 -2015
*  @copyright Tymphany Ltd.
*/
#include <stdio.h>      /*standard input and output*/
#include <stdlib.h>     /*standard library*/
#include <unistd.h>     /*Unix standard function*/
#include "tym_type.h"
#include "tool.h"
/* Debug switch */
#define TOOL_DEBUG_ENABLE
#ifdef TOOL_DEBUG_ENABLE
#define TOOL_DEBUG(x) {printf x;}
#else
#define TOOL_DEBUG(x)
#endif

#define TOOL_ERROR_ENABLE
#ifdef TOOL_ERROR_ENABLE
#define TOOL_ERROR(x) {printf x;}
#else
#define TOOL_ERROR(x)
#endif


void tool_print_buff(char* buff, uint32 size)
{
    int i;
    for(i=0; i<size; i++)
    {
        TOOL_DEBUG(( "%02X ", (unsigned char)buff[i]));
        if (i%8 == 7)
        {
            TOOL_DEBUG(("\n"));
        }
    }
    TOOL_DEBUG(("\n"));
}
