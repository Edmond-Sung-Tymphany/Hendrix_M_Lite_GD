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
    BT_STREAMING_A2DP_STA,
    BT_OUTGOING_CALL_EST_STA,
    BT_INCOMING_CALL_EST_STA,
    BT_ACTIVE_CALL_STA,
    BT_INCOMING_CALL_ON_HOLD_STA,
    BT_ACTIVE_CALL_NO_SCO_STA,
    BT_DUMMY_STA_START,     // the following is not from the LED0 interrupt
    BT_TEST_MODE_STA,
    BT_MAX_LED0_STA,
    BT_LOW_BAT_WARNING_STA,
    BT_RECONNECTING_STA,      //13
    BT_LINKLOST_STA,          //14
    BT_LINKLOST_OFF_STA,      //15
    BT_OFF_STA,
    BT_AUDIO_CUE_START_STA,
    BT_AUDIO_CUE_STOP_STA,
    BT_AVRCP_PLAY_STA,
    BT_AVRCP_PAUSE_STA,
    BT_GET_VERION_STA,
    BT_MAX_STA=32,  // fix the enum value for sdf.json file
} eBtStatus;

/* the indication event from BT module*/
typedef enum
{
    BT_EVT_START = BT_MAX_STA,
    BT_SCL_CONNECTED_EVT,
    BT_PAIRING_FAIL_EVT,
    BT_LINK_LOSS_EVT,
    BT_MAX_EVT,
} eBtIndEvt;

/* command and audio q will be added here*/
typedef enum
{
    BT_ENTER_PAIRING_CMD,
    BT_ENTER_CONNECTABLE_CMD,
    BT_RESET_PAIR_LIST_CMD,

    BT_ANSWER_CMD,
    BT_REJECT_CMD,
    BT_CANCEL_END_CMD,

    BT_RECONNECT_CMD,
    BT_AVRCP_PAUSE_CMD,
    BT_AVRCP_PLAY_CMD,
#ifdef HAS_BT_SKIP_CMD
    BT_AVRCP_SKIP_FORWARD_CMD,
    BT_AVRCP_SKIP_BACKWORD_CMD,
#endif
    BT_DISCONNECT_CMD,
    BT_CONNECTED_CMD,
    BT_TEST_MODE_CMD,
    BT_QUERY_STATUS_CMD,
#ifdef HAS_BT_PWR_ON_CONFIRM
    BT_PWR_ON_CONFIRM_CMD,
#endif
#ifdef HAS_BT_CUE_CMD
    BT_CUE_MIN_CMD,
    BT_PAIRING_START_CUE_CMD = BT_CUE_MIN_CMD,	//16
    BT_PAIRING_SUCCESS_CUE_CMD,					//17				
    BT_PAIRING_FAIL_CUE_CMD,					//18
    BT_PLAY_CUE_CMD,							//19
    BT_PAUSE_CUE_CMD,							//20
    BT_SKIP_BACKWARDS_CUE_CMD,					//21
    BT_SKIP_FORWARDS_CUE_CMD,					//22
    BT_CHARGING_CUE_CMD,						//23
    BT_BAT_LOW_CUE_CMD,							//24	
    BT_BAT_FULL_CUE_CMD,						//25
    BT_TEST_CUE_CMD,
    BT_CUE_MAX_CMD = BT_TEST_CUE_CMD,
#endif
#ifdef HAS_BT_BATT_CMD
    BT_BATT_1,
    BT_BATT_2,
    BT_BATT_3,
    BT_BATT_4,
    BT_BATT_5,
    BT_BATT_6,
#endif
    BT_COMMON_MAX_CMD,
    /******** PRODUCTION  TEST CMD *************/
    BT_PWR_ON_CMD=80,     // fix the enum value for sdf.json file
    BT_PWR_OFF_CMD,
    BT_MAX_CMD,
} eBtCmd;

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
QTimeEvt timeEvt;
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

void BluetoothSrv_SendBtCmd(QActive* me, eBtCmd cmd);

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_SERVER_H */

