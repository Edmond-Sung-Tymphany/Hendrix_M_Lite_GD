/**
 * @file      errorHandleLib.C
 * @brief     Tymphany Error Handler, handle the re-send timeout cases
 * @author    Johnny Fan
 * @date      8-2015
 * @copyright Tymphany Ltd.
 */


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "trace.h"
#include "deviceTypes.h"
#include "errorHandleLib.h"
#include "trace.h"


/*****************************************************************************
 * Config, user can config this part                                 *
 *****************************************************************************/
#define DEFAULT_TIME_OUT_MS  (2000)
#define MAX_RESEND_TIME      (4)
#define BUFFER_SIZE          (50)
#define MAX_MESSAGE_OH_HOLD  (4)

/*****************************************************************************
 * Macro define, user no need to change it                          *
 *****************************************************************************/
#define VERSION              (1)
#define MESSAGE_ID_MIN       (1)
#define MESSAGE_ID_MAX       (~0)

typedef struct
{
    uint32 messageId;
    uint16 timePassMs;
    bool isEnable;
    uint8 resend_count;
} tTimer;

typedef struct
{
    uint32 messageId;
    bool isFree;
    uint8 usedSize;
    uint8 data[BUFFER_SIZE];
} tBuffer;


/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
static uint32 curFreeMessageId;
static tTimer timerValue[MAX_MESSAGE_OH_HOLD];
static tBuffer bufferArray[MAX_MESSAGE_OH_HOLD];
static Func TimeOutHandler;

/*****************************************************************************
 * Local Function declaration                                                   *
 *****************************************************************************/
/* Message Function */
static void ResetMessageId();
static uint32 GetMessageId();

/* Timer Functions */
static void ResetTimer();
static bool RegisterTimer(uint32 messageId);
static void UnRegisterTimer(uint32 messageId);

/* Buffer Functions */
static void ResetBuffer();
static bool SetBuffer(uint32 messageId, uint8* pData, uint8 size);
static uint8 GetBuffer(uint32 messageId, uint8* pData);
static void FreeBuffer(uint32 messageId);


/*****************************************************************************
 * Public Function                                                   *
 *****************************************************************************/
void ErrorHandle_Init()
{
    ResetMessageId();
    ResetTimer();
    ResetBuffer();
}

void ErrorHandle_TimerFeed(uint16 timePassMs)
{
    /* scan the timer value to see which item is triggered*/
    uint8 i, size, buffer[BUFFER_SIZE];
    for(i=0; i<MAX_MESSAGE_OH_HOLD; i++)
    {
        if(timerValue[i].isEnable)
        {
            timerValue[i].timePassMs+=timePassMs;
            if(timerValue[i].timePassMs>=DEFAULT_TIME_OUT_MS)
            {
                /* get the data from buffer according to message Id*/
                size = GetBuffer(timerValue[i].messageId, buffer);
                if(size>0)
                {
                    /* call time out function */
                    TimeOutHandler(buffer, size);
                    /* check for next timeout */
                    timerValue[i].timePassMs = 0;
                    if((++timerValue[i].resend_count)>= MAX_RESEND_TIME)
                    {
                        timerValue[i].isEnable = FALSE;
                        FreeBuffer(timerValue[i].messageId);
                    }
                }
                else
                {
                    // TODO: ERROR, can not find it in the buffer, any more action need to do?
                    ASSERT(0);
                }
            }
        }
    }
}

/* push the message to send list */
void ErrorHandle_Push(eType type, uint8* pData, uint8 size)
{
    //TODO: only push to timer and buffer when the type is confirmation
    if(size>= sizeof(tAseTkMessage))
    {
        /* step 1 Fill in header */
        tAseTkMessage* pMessage = (tAseTkMessage*)pData;
        pMessage->header.version = VERSION;
        pMessage->header.type = type;
        pMessage->header.message_id = GetMessageId();

        /* only register re-send mechanism when it's CON package*/
        if(type == eType_CONFIRMATION)
        {
            /* step 2 Save data to memory */
            SetBuffer(pMessage->header.message_id, pData, size);

            /* step 3 Register timer to trigger re-send */
            RegisterTimer(pMessage->header.message_id);
        }
    }
}

/* pop the message from send list */
eType ErrorHandle_Pop(uint8* pData, uint8 size)
{
    eType rType = eType_RESET;
    if(size>= sizeof(tAseTkMessage))
    {
        /* get the header */
        tAseTkMessage* pMessage = (tAseTkMessage*)pData;
        rType = pMessage->header.type;
        /* reset the re-send timer */
        if((pMessage->header.type==eType_ACK) || (pMessage->header.type==eType_RESET))
        {
            UnRegisterTimer(pMessage->header.message_id);
            FreeBuffer(pMessage->header.message_id);
        }
    }
    return rType;
}

/* response ACK or RESET to ase tk module */
void ErrorHandle_PackageResp(eType type, uint32 messageId, uint8* pData, uint8 size)
{
    if(size>= sizeof(tAseTkMessage))
    {
        /* prepare the header */
        tAseTkMessage* pMessage = (tAseTkMessage*)pData;
        pMessage->header.version = VERSION;
        pMessage->header.type = type;
        pMessage->header.message_id= messageId;
    }
}

void ErrorHandle_RegisterTimeOutFunc(Func func)
{
    TimeOutHandler = func;
}

/*****************************************************************************/
/* Local Func                                                                                                               */
/*****************************************************************************/
static void ResetMessageId()
{
    curFreeMessageId = MESSAGE_ID_MIN;
}

static uint32 GetMessageId()
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

static void ResetTimer()
{
    uint8 i;
    for(i=0; i<MAX_MESSAGE_OH_HOLD; i++)
    {
        timerValue[i].isEnable = FALSE;
    }
}


static bool RegisterTimer(uint32 messageId)
{
    uint8 i;
    bool ret = FALSE;
    for(i=0; i<MAX_MESSAGE_OH_HOLD; i++)
    {
        if(!timerValue[i].isEnable)
        {
            timerValue[i].isEnable = TRUE;
            timerValue[i].messageId = messageId;
            timerValue[i].timePassMs = 0;
            timerValue[i].resend_count = 0;
            ret = TRUE;
            break;
        }
    }
    return ret;
}

static void UnRegisterTimer(uint32 messageId)
{
    uint8 i;
    for(i=0; i<MAX_MESSAGE_OH_HOLD; i++)
    {
        if((timerValue[i].isEnable)&& (timerValue[i].messageId == messageId))
        {
            timerValue[i].isEnable = FALSE;
        }
    }
}

static void ResetBuffer()
{
    uint8 i;
    for(i=0; i<MAX_MESSAGE_OH_HOLD; i++)
    {
        bufferArray[i].isFree = TRUE;
    }
}

/* save the data to buffer*/
static bool SetBuffer(uint32 messageId, uint8* pData, uint8 size)
{
    if(size>BUFFER_SIZE)
    {
        return FALSE;
    }
    uint8 i;
    for(i=0; i<MAX_MESSAGE_OH_HOLD; i++)
    {
        if(bufferArray[i].isFree)
        {
            bufferArray[i].isFree = FALSE;
            memcpy(bufferArray[i].data, pData, size);
            bufferArray[i].usedSize = size;
            bufferArray[i].messageId= messageId;
            return TRUE;
        }
    }
    return FALSE;
}


/* get the data from buffer by id*/
static uint8 GetBuffer(uint32 messageId, uint8* pData)
{
    uint8 i, size =0;
    for(i=0; i<MAX_MESSAGE_OH_HOLD; i++)
    {
        if(bufferArray[i].messageId == messageId)
        {
            memcpy(pData, bufferArray[i].data, bufferArray[i].usedSize);
            size = bufferArray[i].usedSize;
            break;
        }
    }
    return size;
}

/* get the data from buffer by id*/
static void FreeBuffer(uint32 messageId)
{
    uint8 i;
    for(i=0; i<MAX_MESSAGE_OH_HOLD; i++)
    {
        if(bufferArray[i].messageId == messageId)
        {
            bufferArray[i].isFree = TRUE;
            break;
        }
    }
}


