/**
 *  @file      MsgRingBuf.h
 *  @brief     This file defines a general purpose Ring Buffer
 *  @author    Donald Leung, Wesley Lee
 *  @date      12-Dec-2013
 *  @copyright Tymphany Ltd.
 */

#ifndef MsgRingBuf_H
#define MsgRingBuf_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "I2CDmaDrv.h"

#define  msgData void*

CLASS(cMsgRingBuf)
msgData*       pData;  // pointer to items
uint32      size;   // number of items can be stored
msgData*       pIn;    // pointer to the position for next push
msgData*       pOut;   // pointer to the position for next pop

METHODS

/**
* Ring Buffer object constructor
* @param[in]    me              Ring Buffer object
* @param[in]    pBuf            given buffer memory allocation
* @param[in]    length          number of items for the buffer
*/
void            MsgRingBuf_Ctor(cMsgRingBuf* me, msgData* pBuf, const uint32 length);

/**
* Ring Buffer object destructor
* @param[in]    me              Ring Buffer object
*/
void            MsgRingBuf_Xtor(cMsgRingBuf* me);

/**
* Clean the Ring Buffer
* @param[in]    me              Ring Buffer object
*/
void            MsgRingBuf_Reset(cMsgRingBuf* me);

/**
* Get the available size in the Ring Buffer
* @param[in]    me              Ring Buffer object
* @return       uint32          Available size in the Ring Buffer
*/
uint32          MsgRingBuf_GetFreeSize(cMsgRingBuf* me);

/**
* Get the used size of the Ring Buffer
* @param[in]    me              Ring Buffer object
* @return       uint32          Used size in the Ring Buffer
*/
uint32          MsgRingBuf_GetUsedSize(cMsgRingBuf* me);

/**
* Check if the Ring Buffer is empty
* @param[in]    me              Ring Buffer object
* @return       bool            true: empty; false: non-empty
*/
bool            MsgRingBuf_IsEmpty(cMsgRingBuf* me);

/**
* Push msgData to the Ring Buffer
* @param[in]    me              Ring Buffer object
* @param[in]    pBuf            given buffer with msgData to push
* @param[in]    length          length of the pBuf from the user
* @return       uint32          0: TP_SUCCESS, otherwise: fail
*/
eTpRet          MsgRingBuf_Push(cMsgRingBuf* me, const msgData* pBuf, const uint32 length);

/**
* Push single msgData to the Ring Buffer
* @param[in]    me              Ring Buffer object
* @param[in]    d               msgData to push
* @return       uint32          0: TP_SUCCESS, otherwise: fail
*/
eTpRet          MsgRingBuf_PushData(cMsgRingBuf* me, msgData d);

/**
* Pop msgData from the Ring Buffer
* @param[in]    me              Ring Buffer object
* @param[in]    pBuf            buffer for pop msgData return
* @param[in]    length          length of the msgData to pop
* @return       uint32          0: TP_SUCCESS, otherwise: fail
*/
eTpRet          MsgRingBuf_Pop(cMsgRingBuf* me, msgData* pBuf, const uint32 length);

/**
* Pop single msgData from the Ring Buffer
* User must check if the Ring Buffer has element
*   before popping a single msgData with MsgRingBuf_IsEmpty()
* @param[in]    me              Ring Buffer object
* @return       msgData            the msgData popped
*/
msgData            MsgRingBuf_PopData(cMsgRingBuf* me);

/**
* Examine the top-most msgData from the Ring Buffer
* @param[in]    me              Ring Buffer object
* @return       msgData            the top-most msgData in the ring buffer
*/
msgData            MsgRingBuf_TopData(cMsgRingBuf* me);

END_CLASS

#ifdef __cplusplus
}
#endif

#endif

