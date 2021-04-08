/**
 *  @file      ringbuf.h
 *  @brief     This file defines a general purpose Ring Buffer
 *  @author    Donald Leung, Wesley Lee
 *  @date      12-Dec-2013
 *  @copyright Tymphany Ltd.
 */

#ifndef RINGBUF_H
#define RINGBUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"

typedef uint8 data;

CLASS(cRingBuf)
    data*       pData;  // pointer to items
    uint32      size;   // number of items can be stored
    data*       pIn;    // pointer to the position for next push 
    data*       pOut;   // pointer to the position for next pop 
    
METHODS

/**
* Ring Buffer object constructor
* @param[in]    me              Ring Buffer object
* @param[in]    pBuf            given buffer memory allocation
* @param[in]    length          number of items for the buffer
*/
void            RingBuf_Ctor(cRingBuf* me, data* pBuf, const uint32 length);

/**
* Ring Buffer object destructor
* @param[in]    me              Ring Buffer object
*/
void            RingBuf_Xtor(cRingBuf* me);

/**
* Clean the Ring Buffer
* @param[in]    me              Ring Buffer object
*/
void            RingBuf_Reset(cRingBuf* me);

/**
* Get the available size in the Ring Buffer
* @param[in]    me              Ring Buffer object
* @return       uint32          Available size in the Ring Buffer
*/
uint32          RingBuf_GetFreeSize(cRingBuf* me);

/**
* Get the used size of the Ring Buffer
* @param[in]    me              Ring Buffer object
* @return       uint32          Used size in the Ring Buffer
*/
uint32          RingBuf_GetUsedSize(cRingBuf* me);

/**
* Check if the Ring Buffer is empty
* @param[in]    me              Ring Buffer object
* @return       bool            true: empty; false: non-empty
*/
bool            RingBuf_IsEmpty(cRingBuf* me);

/**
* Push data to the Ring Buffer
* @param[in]    me              Ring Buffer object
* @param[in]    pBuf            given buffer with data to push
* @param[in]    length          length of the pBuf from the user
* @return       uint32          0: TP_SUCCESS, otherwise: fail
*/
eTpRet          RingBuf_Push(cRingBuf* me, const data* pBuf, const uint32 length);

/**
* Push single data to the Ring Buffer
* @param[in]    me              Ring Buffer object
* @param[in]    d               data to push
* @return       uint32          0: TP_SUCCESS, otherwise: fail
*/
eTpRet          RingBuf_PushData(cRingBuf* me, data d);

/**
* Pop data from the Ring Buffer
* @param[in]    me              Ring Buffer object
* @param[in]    pBuf            buffer for pop data return
* @param[in]    length          length of the data to pop
* @return       uint32          0: TP_SUCCESS, otherwise: fail
*/
eTpRet          RingBuf_Pop(cRingBuf* me, data* pBuf, const uint32 length);

/**
* Pop single data from the Ring Buffer
* User must check if the Ring Buffer has element 
*   before popping a single data with RingBuf_IsEmpty()
* @param[in]    me              Ring Buffer object
* @return       data            the data popped
*/
data            RingBuf_PopData(cRingBuf* me);

/**
* Examine the top-most data from the Ring Buffer
* @param[in]    me              Ring Buffer object
* @return       data            the top-most data in the ring buffer
*/
data            RingBuf_TopData(cRingBuf* me);

END_CLASS

#ifdef __cplusplus
}
#endif

#endif

