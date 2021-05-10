/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.8 at Mon Dec 18 16:01:24 2017. */

#ifndef PB_PROTO_BTSTATE_BT_STATE_PB_H_INCLUDED
#define PB_PROTO_BTSTATE_BT_STATE_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _Proto_BtState_ConnState {
    Proto_BtState_ConnState_LIMBO = 0,
    Proto_BtState_ConnState_CONNECTABLE = 1,
    Proto_BtState_ConnState_CONN_DISCOVERABLE = 2,
    Proto_BtState_ConnState_CONNECTED = 3,
    Proto_BtState_ConnState_OUTGOING_CALL_ESTABLISH = 4,
    Proto_BtState_ConnState_INCOMING_CALL_ESTABLISH = 5,
    Proto_BtState_ConnState_ACTIVE_CALL_SCO = 6,
    Proto_BtState_ConnState_TESTMODE = 7,
    Proto_BtState_ConnState_THREE_WAY_CALL_WAITING = 8,
    Proto_BtState_ConnState_THREE_WAY_CALL_ON_HOLD = 9,
    Proto_BtState_ConnState_THREE_WAY_MULTICALL = 10,
    Proto_BtState_ConnState_INCOMING_CALL_ON_HOLD = 11,
    Proto_BtState_ConnState_ACTIVE_CALL_NO_SCO = 12,
    Proto_BtState_ConnState_A2DP_STREAMING = 13,
    Proto_BtState_ConnState_LOW_BATTERY = 14,
    Proto_BtState_ConnState_IN_CONFIG_MODE = 15
} Proto_BtState_ConnState;
#define _Proto_BtState_ConnState_MIN Proto_BtState_ConnState_LIMBO
#define _Proto_BtState_ConnState_MAX Proto_BtState_ConnState_IN_CONFIG_MODE
#define _Proto_BtState_ConnState_ARRAYSIZE ((Proto_BtState_ConnState)(Proto_BtState_ConnState_IN_CONFIG_MODE+1))

typedef enum _Proto_BtState_PlayState {
    Proto_BtState_PlayState_AVRCP_STOP = 0,
    Proto_BtState_PlayState_AVRCP_PLAYING = 1,
    Proto_BtState_PlayState_AVRCP_PAUSED = 2,
    Proto_BtState_PlayState_AVRCP_FWD_SEEK = 3,
    Proto_BtState_PlayState_AVRCP_REV_SEEK = 4,
    Proto_BtState_PlayState_AVRCP_ERROR = 5
} Proto_BtState_PlayState;
#define _Proto_BtState_PlayState_MIN Proto_BtState_PlayState_AVRCP_STOP
#define _Proto_BtState_PlayState_MAX Proto_BtState_PlayState_AVRCP_ERROR
#define _Proto_BtState_PlayState_ARRAYSIZE ((Proto_BtState_PlayState)(Proto_BtState_PlayState_AVRCP_ERROR+1))

typedef enum _Proto_BtState_BleState {
    Proto_BtState_BleState_IDLE = 0,
    Proto_BtState_BleState_ADVERTISING = 1,
    Proto_BtState_BleState_SCANNING = 2,
    Proto_BtState_BleState_PERIPHERALS_CONNECTED = 3,
    Proto_BtState_BleState_CENTRAL_CONNECTED = 4
} Proto_BtState_BleState;
#define _Proto_BtState_BleState_MIN Proto_BtState_BleState_IDLE
#define _Proto_BtState_BleState_MAX Proto_BtState_BleState_CENTRAL_CONNECTED
#define _Proto_BtState_BleState_ARRAYSIZE ((Proto_BtState_BleState)(Proto_BtState_BleState_CENTRAL_CONNECTED+1))

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif