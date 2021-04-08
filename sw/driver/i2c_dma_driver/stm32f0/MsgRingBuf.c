/**
 *  @file      MsgMsgRingBuf.c
 *  @brief     This file defines a general purpose Ring Buffer
 *  @change    Rewrite some code when calc used size
 *  @author    Donald Leung, Wesley Lee
 *  @date      12-Dec-2013
 *  @copyright Tymphany Ltd.
 */

#include "MsgRingBuf.h"
#include "commonTypes.h"
#include "trace.h"



/* Comments for implementing MsgRingBuf_IteratePointer()
 * As the MsgRingBuf structure is commonly used in a way such that it is
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
#define MsgRingBuf_IteratePointer(p)   if (p+1 >= me->pData + me->size) \
                                    { \
                                        p = me->pData; \
                                    } \
                                    else{ \
                                        ++(p); \
                                    }

#define MsgRingBuf_Assert(me)  \
{  \
    ASSERT(me);  \
    ASSERT((uint32)(me->pIn) < (uint32)(me->pData + me->size));  \
    ASSERT((uint32)(me->pIn) >= (uint32)(me->pData));  \
    ASSERT((uint32)(me->pOut) < (uint32)(me->pData + me->size));  \
    ASSERT((uint32)(me->pOut) >= (uint32)(me->pData));  \
}

void MsgRingBuf_Reset(cMsgRingBuf* me)
{
    ASSERT(me);
    me->pIn  = me->pData;
    me->pOut = me->pData;
}

uint32 MsgRingBuf_GetFreeSize(cMsgRingBuf* me)
{
    int32 rtn = 0;
    MsgRingBuf_Assert(me);
    // ring buffer index design has one slot remains when full
    rtn = me->size - MsgRingBuf_GetUsedSize(me) - 1;
    ASSERT((rtn < me->size) && (rtn >= 0));
    return (uint32)rtn;
}

uint32 MsgRingBuf_GetUsedSize(cMsgRingBuf* me)
{
    MsgRingBuf_Assert(me);

    uint32 pOut = (uint32)me->pOut;
    uint32 pIn = (uint32)me->pIn;
    uint32 size = (uint32)me->size;
    uint32 consumedSize = 0;

    if(pIn >= pOut)
    {
        // the ring buffer is not wrapped around
        consumedSize = (pIn - pOut) / sizeof(msgData);
    }

    else
    {
        consumedSize = (size * sizeof(msgData) + pIn);
        consumedSize = (consumedSize - pOut) / sizeof(msgData);
    }

    if(consumedSize >= size)
    {
        ASSERT(consumedSize < size);
    }

    return consumedSize;
}

inline bool MsgRingBuf_IsEmpty(cMsgRingBuf* me)
{
    MsgRingBuf_Assert(me);
    return (me->pIn == me->pOut);
}

void MsgRingBuf_Ctor(cMsgRingBuf* me, msgData* pBuf, const uint32 length)
{
    ASSERT(me && pBuf && length);

    me->pData = pBuf;
    me->size  = length;
    MsgRingBuf_Reset(me);
}

void MsgRingBuf_Xtor(cMsgRingBuf* me)
{
    ASSERT(me);

    me->size  = 0;
    me->pData = NULL;
    me->pIn   = NULL;
    me->pOut  = NULL;
}

inline eTpRet MsgRingBuf_PushData(cMsgRingBuf* me, msgData d)
{
    MsgRingBuf_Assert(me);

    // check if the buffer is available
    if(MsgRingBuf_GetFreeSize(me) == 0)
    {
        // User of this function could handle this overflow in run-time,
        // but this should be eliminated during the design stage
        ASSERT(MsgRingBuf_GetFreeSize(me));
        return TP_FAIL;
    }

    *(me->pIn) = d;
    MsgRingBuf_IteratePointer(me->pIn);

    return TP_SUCCESS;
}

inline msgData MsgRingBuf_PopData(cMsgRingBuf* me)
{
    msgData ret;
    MsgRingBuf_Assert(me);

    // check if the buffer is empty
    if(MsgRingBuf_IsEmpty(me))
    {
        // User of this function could handle this overflow in run-time,
        // but this should be eliminated during the design stage
        ASSERT(!MsgRingBuf_IsEmpty(me));
    }

    else    // examine msgData only when the buffer is non-empty
    {
        ret = *(me->pOut);
        MsgRingBuf_IteratePointer(me->pOut);
    }

    return ret;
}

msgData MsgRingBuf_TopData(cMsgRingBuf* me)
{
    msgData ret;
    MsgRingBuf_Assert(me);

    // check if the buffer is empty
    if(MsgRingBuf_IsEmpty(me))
    {
        // User of this function could handle this overflow in run-time,
        // but this should be eliminated during the design stage
        ASSERT(!MsgRingBuf_IsEmpty(me));
    }

    else    // examine msgData only when the buffer is non-empty
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
