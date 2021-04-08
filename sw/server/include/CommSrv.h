/**
 * @file        CommSrv.h
 * @brief       It's the server to communicate with SoC
 * @author      Eason Huang
 * @date        2016-06-02
 * @copyright   Tymphany Ltd.
 */
 

#ifndef COMMSRV_H
#define COMMSRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"
#include "CommDrv.h"

/* command and audio q will be added here*/
typedef enum
{
    COMM_BT_PAIRING_CMD,
    COMM_BT_DISCONNECT_CMD,
    COMM_MAX_CMD,
}eCommCmd;

IND_EVT(CommStateEvt)
    eSocEvt                 evtId;
    uint32                  param;
END_IND_EVT(CommStateEvt)

/* Comm cmd event*/
REQ_EVT(commSendCmdEvt)
    eCommCmd commCmd;
END_REQ_EVT(commSendCmdEvt)

SUBCLASS(cCommSrv, cServer)
    /* private data */
    cCommDrv                *pCommDrvObj;
    bool                    isReady;
METHODS
    /* public functions */
END_CLASS

/* Implement these so the controller can launch the server */
void CommSrv_StartUp(cPersistantObj *me);
void CommSrv_ShutDown(cPersistantObj *me);
void CommSrv_SendCmd(QActive* me, eCommCmd cmd);

/* Single instance object. Must be defined in controller */
extern cCommSrv CommSrv;

#ifdef __cplusplus
}
#endif

#endif /* COMMSRV_H */

