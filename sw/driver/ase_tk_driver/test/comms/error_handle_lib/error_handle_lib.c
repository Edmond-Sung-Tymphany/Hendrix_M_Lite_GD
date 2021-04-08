/**
 * @file      errorHandleLib.C
 * @brief     Tymphany Error Handler, handle the re-send timeout cases
 * @author    Johnny Fan
 * @date      8-2015
 * @copyright Tymphany Ltd.
 */
#include <stdio.h>      /*standard input and output*/
#include <stdlib.h>     /*standard library*/
#include "error_handle_lib.h"


/* Debug switch */
#define FRAMING_DEBUG_ENABLE
#ifdef FRAMING_DEBUG_ENABLE
#define FRAMING_DEBUG(x) {printf x;}
#else
#define FRAMING_DEBUG(x)
#endif

#define FRAMING_ERROR_ENABLE
#ifdef FRAMING_ERROR_ENABLE
#define FRAMING_ERROR(x) {printf x;}
#else
#define FRAMING_ERROR(x)
#endif


#define VERSION              (1)
#define MESSAGE_ID_MIN       (1)
#define MESSAGE_ID_MAX       (~0)

static uint32 curFreeMessageId;

void error_handle_lib_init()
{
    curFreeMessageId = MESSAGE_ID_MIN;
}

uint32 error_handle_lib_get_message_id()
{
    uint32 ret = curFreeMessageId;
    if(curFreeMessageId>=MESSAGE_ID_MAX)
    {
        curFreeMessageId = MESSAGE_ID_MIN;
    }
    else
    {
        curFreeMessageId++;
    }
    return ret;
}
