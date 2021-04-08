
#ifndef BT_STATUS_CMD_H
#define BT_STATUS_CMD_H
#include "tym.pb.h"

#define BT_RX_EVT_DATA_LEN           (sizeof(Proto_Tym_BtMcuMessage))
#define BT_MAX_EVT_PARAM_LENGTH      16
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
    BT_RECONNECTING_STA,
    BT_OFF_STA,
    BT_AUDIO_CUE_START_STA,
    BT_AUDIO_CUE_STOP_STA,
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

typedef enum
{
    BT_MSG_TYPE_DATA,
    BT_MSG_TYPE_ACK,
    BT_MSG_TYPE_NACK,
}eBtReceivedMsgType;


/* command and audio q will be added here*/
typedef enum
{
/* REQUEST */
    BT_ENTER_PAIRING_REQ,
    BT_ENTER_CONNECTABLE_REQ,
    BT_ENTER_RECONNECT_REQ,
    BT_RESET_PAIR_LIST_REQ,
    BT_FACTORY_RESET_REQ,
    BT_AVRCP_NEXT_REQ,
    BT_AVRCP_PREV_REQ,
    BT_AVRCP_PLAY_PAUSE_REQ,
    BT_AVRCP_FAST_FORWARD_START_REQ,
    BT_AVRCP_FAST_FORWARD_STOP_REQ,
    BT_AVRCP_REWIND_START_REQ,
    BT_AVRCP_REWIND_STOP_REQ,
    BT_FIRMWARE_VERSION_REQ,
    BT_GET_ADDRESS_REQ,
    BT_PAIRING_TO_ADDR_REQ,
    BT_COMM_PING_REQ,

/* RESPONSE */
    BT_AUDIO_CUE_REQ_RESP,
    BT_MCU_FIRMWARE_UPDATE_REQ_RESP,

/* EVENT */
    MCU_SYSTEM_BOOTED,
    MCU_VOLUME_CHANGE_EVENT,
    MCU_SYSTEM_STANDBY,
    MCU_SOURCE_CHANGE_EVENT,

    /******** PRODUCTION  TEST CMD *************/
    BT_PWR_ON_CMD=80,     // fix the enum value for sdf.json file
    BT_PWR_OFF_CMD,
    BT_TEST_CUE_CMD,
    BT_MAX_CMD,
} eBtCmd;

#endif

