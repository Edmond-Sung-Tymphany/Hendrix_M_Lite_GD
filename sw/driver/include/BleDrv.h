/**
 * @file        BleDrv.h
 * @brief       BT Driver header file
 * @author      Neo Gan
 * @date        2018-03-23
 * @copyright   Tymphany Ltd.

 */

#ifndef BLEDRV_H
#define BLEDRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "qp_port.h"
#include "signals.h"
#include "ringbuf.h"


typedef struct
{
    void (*initSectionFunc)(void* me);  // function point to a specific initialization section
    uint16 delaytime;                   // time duration to next intialization section in ms
}tInitSection;

CLASS(cBleDrv)   
    /* private data */
    QActive *pSender;
    BOOL    isReady;
    cRingBuf txBufObj;
    cRingBuf rxBufObj;

    BOOL isHeaderReceived;
    uint32_t recvIndex;
    uint32_t recvSize;

    tInitSection const *pInitTable;
    uint8 step;
METHODS

/* public functions */
void BleDrv_Ctor(cBleDrv *me);
void BleDrv_Xtor(cBleDrv *me);

uint16 BleDrv_Init(cBleDrv *me);
uint16 BleDrv_DeInit(cBleDrv *me);
void BleDrv_UpdateStatus(cBleDrv * const me);
void BleDrv_RequestHandler(cBleDrv * const me, QEvt const * const e);
void BleDrv_HandleUnknown(cBleDrv * const me, QEvt const * const e);
void BleDrv_SendData(uint8* buf, uint16 size);

/**
* send the BT command to BT module 
* @param[in]      me         bluetooth driver object
* @param[in]      cmd       the BT command 
*/
void BleDrv_ExecuteCmd(cBleDrv * const me, QEvt const * const e);

/**
* register to get the BT module status signal from driver 
* clent can reset the receiver by calling this function
* @param[in]      me                 bluetooth driver object
* @param[in]      req                the object (BT server) to receive the signal     
*/
//void BleDrv_RegisterRxMsgCb(cBleDrv* me, rxCb_f cb);


END_CLASS


#endif /* BLEDRV_H */

