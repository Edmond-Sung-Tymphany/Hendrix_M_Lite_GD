/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.7 at Mon Apr 17 15:09:18 2017. */

#ifndef PB_PROTO_ASEFEP_ASE_FEP_PB_H_INCLUDED
#define PB_PROTO_ASEFEP_ASE_FEP_PB_H_INCLUDED
#include <pb.h>

#include "firmware-update.pb.h"

#include "common.pb.h"

#include "hdmi.pb.h"

#include "power-link.pb.h"

#include "ase-fep-ReqResp.pb.h"

#include "fep-ase-ReqResp.pb.h"

#include "production.pb.h"

#include "system.pb.h"

#include "dsp.pb.h"

#include "eeb.pb.h"

#include "wpl.pb.h"

#include "puc.pb.h"

#include "player.pb.h"

#include "light-sensor.pb.h"

#include "soundwall.pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _Proto_AseFep_Event_Type {
    Proto_AseFep_Event_Type_BOOTED = 0,
    Proto_AseFep_Event_Type_SYSTEM_STATUS_STANDBY = 1,
    Proto_AseFep_Event_Type_SYSTEM_STATUS_ON = 2,
    Proto_AseFep_Event_Type_SYSTEM_STATUS_ASE_RESTART = 3,
    Proto_AseFep_Event_Type_FACTORY_RESET_DONE = 4,
    Proto_AseFep_Event_Type_SYSTEM_STATUS_ON_NO_OPERATION = 5,
    Proto_AseFep_Event_Type_TUNNEL = 6,
    Proto_AseFep_Event_Type_SW_UPDATE_STARTED = 7,
    Proto_AseFep_Event_Type_SW_UPDATE_FINISHED = 8,
    Proto_AseFep_Event_Type_SW_UPDATE_FAILED = 9,
    Proto_AseFep_Event_Type_COMFORT_TONE_START = 10,
    Proto_AseFep_Event_Type_COMFORT_TONE_DONE = 11,
    Proto_AseFep_Event_Type_VOLUME_CHANGED = 12,
    Proto_AseFep_Event_Type_MUTE_CHANGED = 13,
    Proto_AseFep_Event_Type_NETWORK_INFO = 14,
    Proto_AseFep_Event_Type_PLAYER_DATA = 15,
    Proto_AseFep_Event_Type_FACTORY_RESET_START = 16,
    Proto_AseFep_Event_Type_BT_PAIRING_ENABLED = 17,
    Proto_AseFep_Event_Type_BT_PAIRING_DISABLED = 18,
    Proto_AseFep_Event_Type_BT_PAIRING_FAILED = 19,
    Proto_AseFep_Event_Type_BT_PAIRING_SUCCEEDED = 20,
    Proto_AseFep_Event_Type_BTLE_PAIRING_ENABLED = 21,
    Proto_AseFep_Event_Type_BTLE_PAIRING_DISABLED = 22,
    Proto_AseFep_Event_Type_BTLE_PAIRING_FAILED = 23,
    Proto_AseFep_Event_Type_BTLE_PAIRING_SUCCEEDED = 24,
    Proto_AseFep_Event_Type_LOG_MESSAGE_AVAILABLE = 25,
    Proto_AseFep_Event_Type_LOG_MESSAGE_UNAVAILABLE = 26,
    Proto_AseFep_Event_Type_SOUNDWALL_PRODUCT_TEST_CTL = 200,
    Proto_AseFep_Event_Type_SOUNDWALL_VOLUME_CTL = 201,
    Proto_AseFep_Event_Type_SOUNDWALL_VERSION_INFO_CTL = 202,
    Proto_AseFep_Event_Type_SOUNDWALL_MUTE_CTL = 203,
    Proto_AseFep_Event_Type_SOUNDWALL_LED_CTL = 204,
    Proto_AseFep_Event_Type_SOUNDWALL_SOURCE_CTL = 205,
    Proto_AseFep_Event_Type_SOUNDWALL_BYPASS_CTL = 206,
    Proto_AseFep_Event_Type_SOUNDWALL_ENTER_DFU_CTL = 207,
    Proto_AseFep_Event_Type_SOUNDWALL_A2B_MASTER_CTL = 208,
    Proto_AseFep_Event_Type_SOUNDWALL_NTC_INFO_CTL = 209,
    Proto_AseFep_Event_Type_SOUNDWALL_MUTE_AMP_CTL = 210,
    Proto_AseFep_Event_Type_SOUNDWALL_SYSTEM_RESET_CTL = 211,
    Proto_AseFep_Event_Type_SOUNDWALL_SYSTEM_STATUS_CTL = 212,
    Proto_AseFep_Event_Type_SOUNDWALL_TEST_TONE_CTL = 213,
    Proto_AseFep_Event_Type_SOUNDWALL_WRITE_DSP_PARAM_CTL = 214,
    Proto_AseFep_Event_Type_SOUNDWALL_WRITE_TYPE_NO_CTL = 215,
    Proto_AseFep_Event_Type_SOUNDWALL_WRITE_ITEM_NO_CTL = 216,
    Proto_AseFep_Event_Type_SOUNDWALL_WRITE_SERIAL_NO_CTL = 217,
    Proto_AseFep_Event_Type_SOUNDWALL_GET_SERIAL_NO_CTL = 218
} Proto_AseFep_Event_Type;
#define _Proto_AseFep_Event_Type_MIN Proto_AseFep_Event_Type_BOOTED
#define _Proto_AseFep_Event_Type_MAX Proto_AseFep_Event_Type_SOUNDWALL_GET_SERIAL_NO_CTL
#define _Proto_AseFep_Event_Type_ARRAYSIZE ((Proto_AseFep_Event_Type)(Proto_AseFep_Event_Type_SOUNDWALL_GET_SERIAL_NO_CTL+1))

/* Struct definitions */
typedef struct _Proto_AseFep_Event {
    bool has_type;
    Proto_AseFep_Event_Type type;
    pb_size_t which_data;
    union {
        uint32_t soundwallParam;
    } data;
/* @@protoc_insertion_point(struct:Proto_AseFep_Event) */
} Proto_AseFep_Event;

typedef struct _Proto_AseFep_Req {
    bool has_type;
    Proto_AseFep_ReqResp type;
    bool has_id;
    uint32_t id;
    pb_size_t which_data;
    union {
        Proto_Dsp_RequestAudioInput audioInput;
        Proto_SoundWall_A2Bmode reqA2Bmode;
        Proto_SoundWall_GainAndDelay reqGainAndDelay;
        Proto_SoundWall_DriverGain reqDriverGain;
        Proto_SoundWall_BassAndRoomEQ reqBassAndRoomEQ;
        Proto_SoundWall_PowerMode reqPowerMode;
        Proto_SoundWall_NodeIndex nodeIndex;
        Proto_SoundWall_MuteMode muteMode;
        Proto_SoundWall_ReqTestTone reqTestTone;
    } data;
/* @@protoc_insertion_point(struct:Proto_AseFep_Req) */
} Proto_AseFep_Req;

typedef struct _Proto_AseFep_Resp {
    bool has_type;
    Proto_FepAse_ReqResp type;
    bool has_id;
    uint32_t id;
    bool has_genericResponse;
    Proto_Core_GenericResponse genericResponse;
/* @@protoc_insertion_point(struct:Proto_AseFep_Resp) */
} Proto_AseFep_Resp;

/* Default values for struct fields */

/* Initializer values for message structs */
#define Proto_AseFep_Req_init_default            {false, (Proto_AseFep_ReqResp)0, false, 0, 0, {Proto_Dsp_RequestAudioInput_init_default}}
#define Proto_AseFep_Resp_init_default           {false, (Proto_FepAse_ReqResp)0, false, 0, false, Proto_Core_GenericResponse_init_default}
#define Proto_AseFep_Event_init_default          {false, (Proto_AseFep_Event_Type)0, 0, {0}}
#define Proto_AseFep_Req_init_zero               {false, (Proto_AseFep_ReqResp)0, false, 0, 0, {Proto_Dsp_RequestAudioInput_init_zero}}
#define Proto_AseFep_Resp_init_zero              {false, (Proto_FepAse_ReqResp)0, false, 0, false, Proto_Core_GenericResponse_init_zero}
#define Proto_AseFep_Event_init_zero             {false, (Proto_AseFep_Event_Type)0, 0, {0}}

/* Field tags (for use in manual encoding/decoding) */
#define Proto_AseFep_Event_soundwallParam_tag    9
#define Proto_AseFep_Event_type_tag              1
#define Proto_AseFep_Req_audioInput_tag          13
#define Proto_AseFep_Req_reqA2Bmode_tag          31
#define Proto_AseFep_Req_reqGainAndDelay_tag     32
#define Proto_AseFep_Req_reqDriverGain_tag       33
#define Proto_AseFep_Req_reqBassAndRoomEQ_tag    34
#define Proto_AseFep_Req_reqPowerMode_tag        36
#define Proto_AseFep_Req_nodeIndex_tag           37
#define Proto_AseFep_Req_muteMode_tag            38
#define Proto_AseFep_Req_reqTestTone_tag         39
#define Proto_AseFep_Req_type_tag                1
#define Proto_AseFep_Req_id_tag                  2
#define Proto_AseFep_Resp_type_tag               1
#define Proto_AseFep_Resp_id_tag                 2
#define Proto_AseFep_Resp_genericResponse_tag    3

/* Struct field encoding specification for nanopb */
extern const pb_field_t Proto_AseFep_Req_fields[12];
extern const pb_field_t Proto_AseFep_Resp_fields[4];
extern const pb_field_t Proto_AseFep_Event_fields[3];

/* Maximum encoded size of messages (where known) */
/* Proto_AseFep_Req_size depends on runtime parameters */
#define Proto_AseFep_Resp_size                   12
#define Proto_AseFep_Event_size                  9

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define ASE_FEP_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif