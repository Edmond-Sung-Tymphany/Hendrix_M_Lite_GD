/**
 * @file        BleSrv.h
 * @brief       it's the server to control the BLE module
 * @author      Neo Gan
 * @date        2018-03-21
 * @copyright   Tymphany Ltd.
 */


#ifndef BLESRV_H
#define BLESRV_H

#include "deviceTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"


typedef enum
{
    /* receive data request */
    BLE_START_TIMER_REQ,
    BLE_RECV_TIMEOUT_REQ,
    BLE_RECV_FRAME_REQ,

    /* send data request */
    BLE_RESPONSE_TIMEOUT_REQ,

    /* BLE control request */
    BLE_CTRL_REQ,
}eBleRequestType;


/* Ble cmd event*/
REQ_EVT(BleReqEvt)    
    eBleRequestType type;
    eBleCmd bleCmd;
END_REQ_EVT(BleReqEvt)


SUBCLASS(cBleSrv, cServer)
    /* private data */
    QActive     *pSender;
    eBleCmd     bleCmd;
    QTimeEvt    timeEvt;
    uint32      timeCount;
    eBleCmd     excutingCmd;
    eBleCmd     excutingParam;
METHODS
/* public functions */
END_CLASS

/* Implement these so the controller can launch the server */
void BleSrv_StartUp(cPersistantObj *me);
void BleSrv_ShutDown(cPersistantObj *me);

void BleSrv_SendBleCmd(eBleCmd cmd);

#ifdef __cplusplus
}
#endif

#endif /* BLESRV_H */

