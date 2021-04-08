#include "tp_common.h"

#include "controller.h"
#include "trace.h"
#include "signals.h"
#include "crc16.h"

uint16 Tp_PackData(uint8 sig, uint8 *pDest, uint8 *buf, uint16 size)
{
    uint16 length = 0;

    if(pDest == NULL && buf == NULL && size == 0)
    {
        ASSERT(0);
        return 0;
    }

    pDest[START_SIGN_IDX] = START_SIGN;
    pDest[SIG_IDX]        = sig;
    pDest[SRVID_IDX]      = 0; // ignore

    memcpy(&pDest[DATASTART_IDX], buf, size);

    length = size + DMSG_MIN_SIZE;
    pDest[SIZELSB_IDX] = length & 0x00FF;
    pDest[SIZEMSB_IDX] = (length & 0xFF00) >> 8;

    /*add crc */
    unsigned short crc = crc16(pDest, length - 2);
    pDest[length - 2] = crc & 0x00FF;
    pDest[length - 1] = (crc & 0xFF00) >> 8;

    return length;
}

bool Tp_CheckCrc(uint8 *buffer, uint32 size)
{
    /* Check CRC */
    unsigned short calcCrc = crc16(buffer, size - 2);
    unsigned short recvCrc = (unsigned short)(buffer[size - 1] << 8) | (buffer[size - 2]);
    if(calcCrc == recvCrc)
    {
        return TRUE;
    }

    return FALSE;
}

eSignalType Tp_GetSignalType(uint8 sig, tAppSigMapTable* tp_sig_map, uint8 num)
{
    
    if (sig > MAX_SIG)
    {
#ifdef HAS_APP_PRIVATE_SIG
        uint8 i;
        for (i = 0; i < num; i++)
        {
            if (tp_sig_map[i].appPrivSig == sig)
            {
                return tp_sig_map[i].sigType;
            }
        }
#endif
        return INVALID_EVT_SIG;
    }
    else
    {
        if (MAX_PUB_SIG > sig)
        {
            /*its indication signal, so publish it*/
            return IND_EVT_SIG;
        }
        else if((MAX_PUB_SIG < sig) && (MAX_RESP_SIG > sig))
        {
            /* resp signal, directly send to target srv*/
            return RESP_EVT_SIG;
        }
        else
        {
            /*req signal, directly send to target srv, with replacing sender field*/
            return REQ_EVT_SIG;
        }
    }
}

void Tp_DeliverFrameToServer(QActive *sender, tpMsgEvt *dmsg, eSignalType type, uint8 *buf, uint32 size)
{
    QEvtSize evtSize;
    QEvt *pEvt;
    uint8 *pDest;

    switch(type)
    {
        case REQ_EVT_SIG:
        {
            evtSize = size + sizeof(QEvt) + sizeof(QActive*);
            ASSERT(evtSize <= SIZE_OF_LARGE_EVENTS);
            pEvt = QF_newX_(evtSize, 0, dmsg->signal);
            pDest = (uint8*)pEvt;

            pDest += sizeof(QEvt);
            memcpy(pDest, (uint8_t*)&sender, sizeof(QActive*));
            pDest += sizeof(QActive*);
            memcpy(pDest, buf, size);
            SendToServer(dmsg->target_srv_id, pEvt);
            break;
        }
        case RESP_EVT_SIG:
        {
            evtSize = size + sizeof(QEvt);
            ASSERT(evtSize <= SIZE_OF_LARGE_EVENTS);
            pEvt = QF_newX_(evtSize, 0, dmsg->signal);
            pDest = (uint8*)pEvt;

            pDest += sizeof(QEvt);
            memcpy(pDest, buf, size);
            SendToServer(dmsg->target_srv_id, pEvt);
            break;
        }
        case IND_EVT_SIG:
        {
            evtSize = size + sizeof(QEvt);
            ASSERT(evtSize <= SIZE_OF_LARGE_EVENTS);
            pEvt = QF_newX_(evtSize, 0, dmsg->signal);
            pDest = (uint8*)pEvt;

            pDest += sizeof(QEvt);
            memcpy(pDest, buf, size);
            QF_PUBLISH(pEvt, 0);
            break;
        }
        default:
        {
            ASSERT(0);
            break;
        }
    }

}

uint16 Tp_HandleResponse(eSignalType type, uint8 *pDest, uint32 sig, uint16 size, uint8* e)
{
    uint8 *pSrc = (uint8*)e;
    uint32 length = 0;

    pDest[START_SIGN_IDX] = START_SIGN;
    pDest[SIG_IDX]        = sig;
    pDest[SRVID_IDX]      = 0; // ignore

    length = size;
    switch(type)
    {
        case REQ_EVT_SIG:
        {
            pSrc += sizeof(QEvt);
            pSrc += sizeof(QActive*);
            length -= sizeof(QEvt);
            length -= sizeof(QActive*);
            break;
        }
        case RESP_EVT_SIG:
        {
            pSrc += sizeof(QEvt);
            length -= sizeof(QEvt);
            break;
        }
        case IND_EVT_SIG:
        {
            pSrc += sizeof(QEvt);
            length -= sizeof(QEvt);
            break;
        }
        default:
        {
            ASSERT(0);
            break;
        }
    }

    memcpy(&pDest[DATASTART_IDX], pSrc, length);

    length += DMSG_MIN_SIZE;
    pDest[SIZELSB_IDX] = length & 0x00FF;
    pDest[SIZEMSB_IDX] = (length & 0xFF00) >> 8;

    /*add crc */
    unsigned short crc = crc16(pDest, length - 2);
    pDest[length - 2] = crc & 0x00FF;
    pDest[length - 1] = (crc & 0xFF00) >> 8;

    return length;
}
