/**
 * @file        BluetoothSrv.h
 * @brief       it's the server to control the BT module
 * @author      Johnny Fan
 * @date        2014-05-11
 * @copyright   Tymphany Ltd.
 */


#ifndef BLUETOOTH_SERVER_H
#define BLUETOOTH_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"

/* BT status*/
typedef enum
{
    BT_CONNECTABLE_STA,
    BT_DISCOVERABLE_STA,
    BT_CONNECTED_STA,
    BT_OUTGOING_CALL_EST_STA,
    BT_INCOMING_CALL_EST_STA,
    BT_ACTIVE_CALL_STA,
    BT_TEST_MODE_STA,
    BT_MAX_LED0_STA,
    BT_STREAMING_A2DP_STA,
    BT_TWC_CALL_WAITING_STA,
    BT_TWC_CALL_ON_HOLD_STA,
    BT_TWC_MULTI_PARTY_CALL_STA,
    BT_INCOMING_CALL_ON_HOLD_STA,
    BT_ACTIVE_CALL_NO_SCO_STA,
    BT_LOW_BAT_WARNING_STA,
    BT_MAX_STA,
}eBtStatus;

/* the indication event from BT module*/
typedef enum
{
    BT_SCL_CONNECTED_EVT = BT_MAX_STA+1,
    BT_PAIRING_FAIL_EVT,
    BT_LINK_LOSS_EVT,
    BT_MAX_EVT,
}eBtIndEvt;

/* command and audio q will be added here*/
typedef enum
{
    BT_ENTER_PAIRING_CMD,
    BT_ENTER_CONNECTABLE_CMD,
    BT_CONNECT_CMD,
    BT_PLAY_PAUSE_CMD,
    BT_RESET_PAIR_LIST_CMD,
    BT_ANSWER_CMD,
    BT_REJECT_CMD,
    BT_CANCEL_END_CMD,
    BT_TWC_ACCEPT_WAITING_HOLD_ACTIVE_CMD,
    BT_TWC_RELEASE_ALL_HELD_CMD,
    BT_AVRCP_SKIP_FORWARD_CMD,
    BT_AVRCP_SKIP_BACKWORD_CMD,
    BT_OFF_CMD,
/*********** TONE CMD ****************/
    BT_TONE_RESET_PDL_CMD,
    BT_TONE_BAT_DOCK_CMD,
    BT_TONE_AC_IN_CMD,
    BT_TONE_LIM_VOL_CMD,
    BT_TONE_PAIRING_CMD,
    BT_TONE_POWER_OFF_CMD,
    BT_TONE_CONNECTED_CMD,
    BT_TONE_PAIR_FAIL_CMD,
    BT_COMMON_MAX_CMD,
/******** PRODUCTION  TEST CMD *************/
    BT_PWR_ON_CMD,
    BT_PWR_OFF_CMD,
    BT_MAX_CMD,
}eBtCmd;

#define BT_FIRST_TONE    BT_TONE_RESET_PDL_CMD

/* BT status event*/
IND_EVT(BtStatusEvt)
    eBtStatus btStatus;
    eBtIndEvt btIndEvt;
    bool isBtStatus;
END_IND_EVT(BtStatusEvt)

/* BT cmd event*/
REQ_EVT(BtCmdEvt)
    eBtCmd btCmd;
END_REQ_EVT(BtCmdEvt)

#ifdef BT_NFC_PAIR
REQ_EVT(BtNfcEvt)
END_REQ_EVT(BtNfcEvt)
#endif

SUBCLASS(cBluetoothSrv, cServer)
    /* private data */
    int32 timeCount;
    eBtStatus btStatus;
    eBtIndEvt btIndEvt;
    int16 cmdQueueDelayMs;
    bool isCmdExcuting;
    bool isQueueWaiting;
METHODS
    /* public functions */
END_CLASS

/* Implement these so the controller can launch the server */
void BluetoothSrv_StartUp(cPersistantObj *me);
void BluetoothSrv_ShutDown(cPersistantObj *me);

#ifdef GTEST
  /* State function definitions */
QState BluetoothSrv_Initial(cBluetoothSrv * const me, QEvt const * const e);
QState BluetoothSrv_PreActive(cBluetoothSrv * const me, QEvt const * const e);
QState BluetoothSrv_Active(cBluetoothSrv * const me, QEvt const * const e);
QState BluetoothSrv_DeActive(cBluetoothSrv * const me, QEvt const * const e);
#endif

void BluetoothSrv_SendBtCmd(QActive* me, eBtCmd cmd);

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_SERVER_H */

