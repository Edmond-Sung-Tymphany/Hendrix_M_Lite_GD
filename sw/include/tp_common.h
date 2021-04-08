/**
 * @file tp_common.h
 */

#ifndef TP_COMMON_H
#define TP_COMMON_H

#include "stdint.h"
#include "commonTypes.h"
#include "qf_port.h"

#include "object_ids.h"
#include "signals.h"
#include "bsp.h"

#define DMSG_MIN_SIZE   0x07

#define START_SIGN      0xAA  // package signature

typedef enum
{
    START_SIGN_IDX,
    SIG_IDX,
    SRVID_IDX,
    SIZELSB_IDX,
    SIZEMSB_IDX,
    DATASTART_IDX,
}eTpMsgIndx;

typedef enum
{
    REQ_EVT_SIG,
    RESP_EVT_SIG,
    IND_EVT_SIG,
    INVALID_EVT_SIG,
} eSignalType;

typedef struct
{
    uint8	        appPrivSig;
    eSignalType	    sigType;
}tAppSigMapTable;

typedef struct
{
    uint8 seq;
    uint16 msg_size;
    eSignal signal;
    ePersistantObjID target_srv_id;
} tpMsgEvt;


uint16 Tp_PackData(uint8 sig, uint8 *pDest, uint8 *buf, uint16 size);
bool Tp_CheckCrc(uint8 *buffer, uint32 size);
eSignalType Tp_GetSignalType(uint8 sig, tAppSigMapTable *tp_sig_map, uint8 num);
void Tp_DeliverFrameToServer(QActive *sender, tpMsgEvt *dmsg, eSignalType type, uint8 *buf, uint32 size);
uint16 Tp_HandleResponse(eSignalType type, uint8 *pDest, uint32 sig, uint16 size, uint8* e);


#endif
