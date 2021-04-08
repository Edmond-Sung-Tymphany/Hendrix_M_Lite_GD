/**
 *  @file      ringbuf.c
 *  @brief     This file defines a general purpose Ring Buffer
 *  @author    Donald Leung, Wesley Lee
 *  @date      12-Dec-2013
 *  @copyright Tymphany Ltd.
 */

#include "ringbuf.h"
#include "commonTypes.h"
#include "trace.h"



/* Comments for implementing RingBuf_IteratePointer()
 * As the ringbuf structure is commonly used in a way such that it is
 * often shared by the main code and IRQ.
 *
 * so **at any time**, our ring buf pointer should always within the range of:
 *    [ me->pData, me->pData + me->size)  ## never at 'me->pData + me->size'
 *  
 * so never implement like the following, as it will expose a chance that
 * 'p' has a out of bound value of 'me->pData + me->size'
 * {
 *     ++p;
 *     if( p>= me->pData + me->size)
 *         p = me->pData;
 * }
 */
#define RingBuf_IteratePointer(p)   if (p+1 >= me->pData + me->size) \
                                    { \
                                        p = me->pData; \
                                    } \
                                    else{ \
                                        ++(p); \
                                    }

#define RingBuf_Assert(me)  \
{  \
    ASSERT(me);  \
    ASSERT((uint32)(me->pIn) < (uint32)(me->pData + me->size));  \
    ASSERT((uint32)(me->pIn) >= (uint32)(me->pData));  \
    ASSERT((uint32)(me->pOut) < (uint32)(me->pData + me->size));  \
    ASSERT((uint32)(me->pOut) >= (uint32)(me->pData));  \
}

void RingBuf_Reset(cRingBuf* me)
{
    ASSERT(me);
    me->pIn  = me->pData;
    me->pOut = me->pData;
}

uint32 RingBuf_GetFreeSize(cRingBuf* me)
{
    int32 rtn = 0;
    RingBuf_Assert(me);
    // ring buffer index design has one slot remains when full
    rtn = me->size - RingBuf_GetUsedSize(me) - 1;
    ASSERT((rtn < me->size)&& (rtn >= 0));
    return (uint32)rtn;
}

uint32 RingBuf_GetUsedSize(cRingBuf* me)
{
    RingBuf_Assert(me);

    unsigned int pOut = (unsigned int)me->pOut;
    unsigned int pIn = (unsigned int)me->pIn;
    unsigned int size = (unsigned int)me->size;
    unsigned int consumedSize = 0;
    if (pIn >= pOut)
    {
        // the ring buffer is not wrapped around
        consumedSize = pIn - pOut;
    }
    else
    {
        consumedSize = (size + pIn);
        consumedSize = consumedSize - pOut;
    }
    if(consumedSize >= size)
    {
        ASSERT(consumedSize < size);
    }
    return consumedSize;
}

inline bool RingBuf_IsEmpty(cRingBuf* me)
{
    RingBuf_Assert(me);
    return (me->pIn == me->pOut);
}

void RingBuf_Ctor(cRingBuf* me, data* pBuf, const uint32 length)
{
    ASSERT(me && pBuf && length);

    me->pData = pBuf;
    me->size  = length;
    RingBuf_Reset(me);
}

void RingBuf_Xtor(cRingBuf* me)
{
    ASSERT(me);

    me->size  = 0;
    me->pData = NULL;
    me->pIn   = NULL;
    me->pOut  = NULL;
}

eTpRet RingBuf_Push(cRingBuf* me, const data* pBuf, const uint32 length)
{
    uint32 i = 0;
    ASSERT(pBuf && length);
    RingBuf_Assert(me);

    // check if the buffer is enough for the incoming message
    if (RingBuf_GetFreeSize(me) < length)
    {
        // User of this function could handle this overflow in run-time,
        // but this should be eliminated during the design stage
        ASSERT(RingBuf_GetFreeSize(me) >= length);
        return TP_FAIL;
    }

    for (i=0 ; i<length ; i++)
    {
        *(me->pIn) = pBuf[i];
        RingBuf_IteratePointer(me->pIn);
    }
 
    return TP_SUCCESS;
}

eTpRet RingBuf_Pop(cRingBuf* me, data* pBuf, const uint32 length)
{
    uint32 i = 0;
    ASSERT(pBuf && length);
    RingBuf_Assert(me);

    // check if the consumed buffer is enough for the request
    if (RingBuf_GetUsedSize(me) < length)
    {
        // User of this function could handle this overflow in run-time,
        // but this should be eliminated during the design stage
        ASSERT(RingBuf_GetUsedSize(me) >= length);
        return TP_FAIL;
    }

    for (i=0 ; i<length ; i++)
    {
        pBuf[i] = *(me->pOut);
        RingBuf_IteratePointer(me->pOut);
    }
 
    return TP_SUCCESS;
}

inline eTpRet RingBuf_PushData(cRingBuf* me, data d)
{
    RingBuf_Assert(me);

    // check if the buffer is available
    if (RingBuf_GetFreeSize(me) == 0)
    {
        // User of this function could handle this overflow in run-time,
        // but this should be eliminated during the design stage
        ASSERT(RingBuf_GetFreeSize(me));
        return TP_FAIL;
    }

    *(me->pIn) = d;
    RingBuf_IteratePointer(me->pIn);
 
    return TP_SUCCESS;
}

inline data RingBuf_PopData(cRingBuf* me)
{
    data ret = 0;
    RingBuf_Assert(me);

    // check if the buffer is empty
    if (RingBuf_IsEmpty(me))
    {
        // User of this function could handle this overflow in run-time,
        // but this should be eliminated during the design stage
        ASSERT(!RingBuf_IsEmpty(me));
    }
    else    // examine data only when the buffer is non-empty
    {
        ret = *(me->pOut);
        RingBuf_IteratePointer(me->pOut);
    }
    return ret;
}

data RingBuf_TopData(cRingBuf* me)
{
    data ret = 0;
    RingBuf_Assert(me);

    // check if the buffer is empty
    if (RingBuf_IsEmpty(me))
    {
        // User of this function could handle this overflow in run-time,
        // but this should be eliminated during the design stage
        ASSERT(!RingBuf_IsEmpty(me));
    }
    else    // examine data only when the buffer is non-empty
    {
        if(me->pIn == me->pData)
        {
            ret = *(me->pData + me->size);
        }
        else
        {
            ret = *(me->pIn - 1);
        }
    }

    return ret;
}
