/**
 * @file        BluetoothStatus.h
 * @brief       BT Status header file
 * @author      Edmond Sung
 * @date        2014-02-10
 * @copyright   Tymphany Ltd.

 */

#ifndef BLUETOOTH_STATUS_H
#define BLUETOOTH_STATUS_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum
{
    BT_OFF_STA             = 0x01,
    BT_CONNECTABLE_STA     = 0x02,
    BT_RECONNECTING_STA    = 0x04,
    BT_DISCOVERABLE_STA    = 0x08,
    BT_CONNECTED_STA       = 0x10,
    BT_STREAMING_A2DP_STA  = 0x20,
    BT_LINK_LOSS_RECONNECT = 0x40,
    BT_LINK_LOSS_RECONNECT_OFF  = 0x80,
    BT_MAX_STA                  = 0x100,
}eBtStatus;

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
    BT_PWR_ON_CMD, //22
    BT_PWR_OFF_CMD,
    BT_WAIT_CMD,
    BT_MAX_CMD,
}eBtCmd;

typedef enum
{
    BT_REPORT_TYPE_STATUS_IND,
    BT_REPORT_TYPE_COMMAND_RESP,
}eBtReportType;

typedef enum
{
    BT_ERROR_STATUS_SUCCESS = 0,
    BT_ERROR_STATUS_FAILURE = 1,
}eBtErrorStatus;

typedef void (*btDrvReportCallback)(eBtReportType type, uint32 reportData);

#endif /* BLUETOOTH_STATUS_H */


