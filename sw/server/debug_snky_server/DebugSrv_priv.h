/**
 * @file        DebugSrv_priv.h
 * @brief       this\ is\ debug\ \ server
 * @author      Dmitry.Abdulov 
 * @date        2014-04-21
 * @copyright   Tymphany Ltd.
 */

#ifndef DEBUG_SERVER_PRIVATE_H
#define DEBUG_SERVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tDbgSnkyMsgEvt
{
    uint8 seq;
    eSignal signal;
    eServerID target_srv_id;
    uint16 msg_size;
} tDbgSnkyMsgEvt;

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_SERVER_PRIVATE_H */