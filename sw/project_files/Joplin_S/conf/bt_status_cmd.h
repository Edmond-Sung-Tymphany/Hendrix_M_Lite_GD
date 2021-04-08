
#ifndef BT_STATUS_CMD_H
#define BT_STATUS_CMD_H



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
    BT_ENTER_PAIRING_CMD,
    BT_ENTER_CONNECTABLE_CMD,
    BT_RESET_PAIR_LIST_CMD,
    BT_AVRCP_NEXT_CMD,
    BT_AVRCP_PREV_CMD,
    BT_AVRCP_PLAY_PAUSE_CMD,
    BT_AVRCP_SKIP_FORWARD_CMD,
    BT_AVRCP_SKIP_BACKWORD_CMD,
    BT_CONNECTED_CMD,
    BT_TEST_MODE_CMD,
    BT_COMMON_MAX_CMD,
    /******** PRODUCTION  TEST CMD *************/
    BT_PWR_ON_CMD=80,     // fix the enum value for sdf.json file
    BT_PWR_OFF_CMD,
    BT_MAX_CMD,
} eBtCmd;

#endif

