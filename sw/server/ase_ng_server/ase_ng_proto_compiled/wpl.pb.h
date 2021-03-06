/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.7 at Tue May 16 18:29:09 2017. */

#ifndef PB_PROTO_WPL_WPL_PB_H_INCLUDED
#define PB_PROTO_WPL_WPL_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _Proto_Wpl_Command {
    Proto_Wpl_Command_Init = 0,
    Proto_Wpl_Command_Shutdown = 1,
    Proto_Wpl_Command_DiscoveryFull = 2,
    Proto_Wpl_Command_DiscoveryFast = 3,
    Proto_Wpl_Command_StoreSpkConfig = 4,
    Proto_Wpl_Command_RemoveSpk = 5,
    Proto_Wpl_Command_ResetSpkConfig = 6,
    Proto_Wpl_Command_GetSummitModuleInfo = 7,
    Proto_Wpl_Command_GetSpkDesc = 8,
    Proto_Wpl_Command_GetSpkState = 9,
    Proto_Wpl_Command_Mute = 10,
    Proto_Wpl_Command_SetNetworkQualityThresholdLevel = 11,
    Proto_Wpl_Command_RawCmd = 12,
    Proto_Wpl_Command_GetSummitFwData = 13,
    Proto_Wpl_Command_GetSpeakerMap = 14,
    Proto_Wpl_Command_SetSpeakerMap = 15,
    Proto_Wpl_Command_GetFwVersion = 16,
    Proto_Wpl_Command_GetDFSRev = 17,
    Proto_Wpl_Command_GetAPIVersion = 18,
    Proto_Wpl_Command_SetTestSpkMac = 19,
    Proto_Wpl_Command_ClearTestSpkMac = 20,
    Proto_Wpl_Command_ResetTXMaster = 21,
    Proto_Wpl_Command_SetSpeakersOff = 22,
    Proto_Wpl_Command_SetSpeakersOn = 23,
    Proto_Wpl_Command_SetMasterMfgData = 24
} Proto_Wpl_Command;
#define _Proto_Wpl_Command_MIN Proto_Wpl_Command_Init
#define _Proto_Wpl_Command_MAX Proto_Wpl_Command_SetMasterMfgData
#define _Proto_Wpl_Command_ARRAYSIZE ((Proto_Wpl_Command)(Proto_Wpl_Command_SetMasterMfgData+1))

typedef enum _Proto_Wpl_Status {
    Proto_Wpl_Status_Done = 0,
    Proto_Wpl_Status_Error = 1
} Proto_Wpl_Status;
#define _Proto_Wpl_Status_MIN Proto_Wpl_Status_Done
#define _Proto_Wpl_Status_MAX Proto_Wpl_Status_Error
#define _Proto_Wpl_Status_ARRAYSIZE ((Proto_Wpl_Status)(Proto_Wpl_Status_Error+1))

/* Struct definitions */
typedef struct _Proto_Wpl_Event {
    bool has_type;
    uint32_t type;
/* @@protoc_insertion_point(struct:Proto_Wpl_Event) */
} Proto_Wpl_Event;

typedef struct _Proto_Wpl_Request {
    bool has_type;
    Proto_Wpl_Command type;
    pb_callback_t raw;
    bool has_param;
    uint32_t param;
/* @@protoc_insertion_point(struct:Proto_Wpl_Request) */
} Proto_Wpl_Request;

typedef struct _Proto_Wpl_Response {
    bool has_type;
    Proto_Wpl_Command type;
    bool has_status;
    Proto_Wpl_Status status;
    pb_callback_t raw;
    bool has_param;
    uint32_t param;
/* @@protoc_insertion_point(struct:Proto_Wpl_Response) */
} Proto_Wpl_Response;

/* Default values for struct fields */

/* Initializer values for message structs */
#define Proto_Wpl_Request_init_default           {false, (Proto_Wpl_Command)0, {{NULL}, NULL}, false, 0}
#define Proto_Wpl_Response_init_default          {false, (Proto_Wpl_Command)0, false, (Proto_Wpl_Status)0, {{NULL}, NULL}, false, 0}
#define Proto_Wpl_Event_init_default             {false, 0}
#define Proto_Wpl_Request_init_zero              {false, (Proto_Wpl_Command)0, {{NULL}, NULL}, false, 0}
#define Proto_Wpl_Response_init_zero             {false, (Proto_Wpl_Command)0, false, (Proto_Wpl_Status)0, {{NULL}, NULL}, false, 0}
#define Proto_Wpl_Event_init_zero                {false, 0}

/* Field tags (for use in manual encoding/decoding) */
#define Proto_Wpl_Event_type_tag                 1
#define Proto_Wpl_Request_type_tag               1
#define Proto_Wpl_Request_raw_tag                10
#define Proto_Wpl_Request_param_tag              11
#define Proto_Wpl_Response_type_tag              1
#define Proto_Wpl_Response_status_tag            2
#define Proto_Wpl_Response_raw_tag               10
#define Proto_Wpl_Response_param_tag             11

/* Struct field encoding specification for nanopb */
extern const pb_field_t Proto_Wpl_Request_fields[4];
extern const pb_field_t Proto_Wpl_Response_fields[5];
extern const pb_field_t Proto_Wpl_Event_fields[2];

/* Maximum encoded size of messages (where known) */
/* Proto_Wpl_Request_size depends on runtime parameters */
/* Proto_Wpl_Response_size depends on runtime parameters */
#define Proto_Wpl_Event_size                     6

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define WPL_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
