/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.7 at Tue May 16 18:29:07 2017. */

#ifndef PB_PROTO_HDMI_HDMI_PB_H_INCLUDED
#define PB_PROTO_HDMI_HDMI_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _Proto_Hdmi_StandbyCmd_Type {
    Proto_Hdmi_StandbyCmd_Type_HDMI_CEC_STANDBY_TV = 0,
    Proto_Hdmi_StandbyCmd_Type_HDMI_CEC_STANDBY_ALL = 1
} Proto_Hdmi_StandbyCmd_Type;
#define _Proto_Hdmi_StandbyCmd_Type_MIN Proto_Hdmi_StandbyCmd_Type_HDMI_CEC_STANDBY_TV
#define _Proto_Hdmi_StandbyCmd_Type_MAX Proto_Hdmi_StandbyCmd_Type_HDMI_CEC_STANDBY_ALL
#define _Proto_Hdmi_StandbyCmd_Type_ARRAYSIZE ((Proto_Hdmi_StandbyCmd_Type)(Proto_Hdmi_StandbyCmd_Type_HDMI_CEC_STANDBY_ALL+1))

typedef enum _Proto_Hdmi_Arc_Status {
    Proto_Hdmi_Arc_Status_HDMI_ARC_NOT_STARTED = 0,
    Proto_Hdmi_Arc_Status_HDMI_ARC_STARTED = 1,
    Proto_Hdmi_Arc_Status_HDMI_ARC_START_BY_TV = 2,
    Proto_Hdmi_Arc_Status_HDMI_ARC_START_BY_US = 3,
    Proto_Hdmi_Arc_Status_HDMI_ARC_END_BY_TV = 4,
    Proto_Hdmi_Arc_Status_HDMI_ARC_END_BY_US = 5,
    Proto_Hdmi_Arc_Status_HDMI_ARC_ENDED = 6,
    Proto_Hdmi_Arc_Status_HDMI_ARC_AUDIO_MODE_ON = 7,
    Proto_Hdmi_Arc_Status_HDMI_ARC_AUDIO_MODE_OFF = 8
} Proto_Hdmi_Arc_Status;
#define _Proto_Hdmi_Arc_Status_MIN Proto_Hdmi_Arc_Status_HDMI_ARC_NOT_STARTED
#define _Proto_Hdmi_Arc_Status_MAX Proto_Hdmi_Arc_Status_HDMI_ARC_AUDIO_MODE_OFF
#define _Proto_Hdmi_Arc_Status_ARRAYSIZE ((Proto_Hdmi_Arc_Status)(Proto_Hdmi_Arc_Status_HDMI_ARC_AUDIO_MODE_OFF+1))

typedef enum _Proto_Hdmi_AudioFormat_AudioMode {
    Proto_Hdmi_AudioFormat_AudioMode_CHANNELS_2 = 0,
    Proto_Hdmi_AudioFormat_AudioMode_CHANNELS_8 = 1,
    Proto_Hdmi_AudioFormat_AudioMode_HBRA = 2,
    Proto_Hdmi_AudioFormat_AudioMode_DSD = 3
} Proto_Hdmi_AudioFormat_AudioMode;
#define _Proto_Hdmi_AudioFormat_AudioMode_MIN Proto_Hdmi_AudioFormat_AudioMode_CHANNELS_2
#define _Proto_Hdmi_AudioFormat_AudioMode_MAX Proto_Hdmi_AudioFormat_AudioMode_DSD
#define _Proto_Hdmi_AudioFormat_AudioMode_ARRAYSIZE ((Proto_Hdmi_AudioFormat_AudioMode)(Proto_Hdmi_AudioFormat_AudioMode_DSD+1))

typedef enum _Proto_Hdmi_InputSelected_AudioSelected {
    Proto_Hdmi_InputSelected_AudioSelected_ARC = 0,
    Proto_Hdmi_InputSelected_AudioSelected_HDMI = 1
} Proto_Hdmi_InputSelected_AudioSelected;
#define _Proto_Hdmi_InputSelected_AudioSelected_MIN Proto_Hdmi_InputSelected_AudioSelected_ARC
#define _Proto_Hdmi_InputSelected_AudioSelected_MAX Proto_Hdmi_InputSelected_AudioSelected_HDMI
#define _Proto_Hdmi_InputSelected_AudioSelected_ARRAYSIZE ((Proto_Hdmi_InputSelected_AudioSelected)(Proto_Hdmi_InputSelected_AudioSelected_HDMI+1))

typedef enum _Proto_Hdmi_AudioModeSelect_Type {
    Proto_Hdmi_AudioModeSelect_Type_AUTOMATIC = 0,
    Proto_Hdmi_AudioModeSelect_Type_ARC_ONLY = 1
} Proto_Hdmi_AudioModeSelect_Type;
#define _Proto_Hdmi_AudioModeSelect_Type_MIN Proto_Hdmi_AudioModeSelect_Type_AUTOMATIC
#define _Proto_Hdmi_AudioModeSelect_Type_MAX Proto_Hdmi_AudioModeSelect_Type_ARC_ONLY
#define _Proto_Hdmi_AudioModeSelect_Type_ARRAYSIZE ((Proto_Hdmi_AudioModeSelect_Type)(Proto_Hdmi_AudioModeSelect_Type_ARC_ONLY+1))

/* Struct definitions */
typedef struct _Proto_Hdmi_InputsSense {
    pb_callback_t senseStatus;
/* @@protoc_insertion_point(struct:Proto_Hdmi_InputsSense) */
} Proto_Hdmi_InputsSense;

typedef struct _Proto_Hdmi_UhdDeepColour {
    pb_callback_t uhddcStatus;
/* @@protoc_insertion_point(struct:Proto_Hdmi_UhdDeepColour) */
} Proto_Hdmi_UhdDeepColour;

typedef struct _Proto_Hdmi_Arc {
    bool has_status;
    Proto_Hdmi_Arc_Status status;
/* @@protoc_insertion_point(struct:Proto_Hdmi_Arc) */
} Proto_Hdmi_Arc;

typedef struct _Proto_Hdmi_AudioFormat {
    pb_callback_t audioInfoFrameData;
    pb_callback_t channelStatusData;
    bool has_audioMode;
    Proto_Hdmi_AudioFormat_AudioMode audioMode;
/* @@protoc_insertion_point(struct:Proto_Hdmi_AudioFormat) */
} Proto_Hdmi_AudioFormat;

typedef struct _Proto_Hdmi_AudioModeSelect {
    bool has_type;
    Proto_Hdmi_AudioModeSelect_Type type;
/* @@protoc_insertion_point(struct:Proto_Hdmi_AudioModeSelect) */
} Proto_Hdmi_AudioModeSelect;

typedef struct _Proto_Hdmi_HdmiInput {
    uint32_t number;
/* @@protoc_insertion_point(struct:Proto_Hdmi_HdmiInput) */
} Proto_Hdmi_HdmiInput;

typedef struct _Proto_Hdmi_InputSelected {
    Proto_Hdmi_InputSelected_AudioSelected audio;
    bool has_inputNumber;
    uint32_t inputNumber;
/* @@protoc_insertion_point(struct:Proto_Hdmi_InputSelected) */
} Proto_Hdmi_InputSelected;

typedef struct _Proto_Hdmi_InputSense {
    bool senseOn;
    uint32_t hdmiPort;
/* @@protoc_insertion_point(struct:Proto_Hdmi_InputSense) */
} Proto_Hdmi_InputSense;

typedef struct _Proto_Hdmi_InputsSense_SenseState {
    bool senseOn;
    uint32_t hdmiPort;
/* @@protoc_insertion_point(struct:Proto_Hdmi_InputsSense_SenseState) */
} Proto_Hdmi_InputsSense_SenseState;

typedef struct _Proto_Hdmi_StandbyCmd {
    bool has_type;
    Proto_Hdmi_StandbyCmd_Type type;
/* @@protoc_insertion_point(struct:Proto_Hdmi_StandbyCmd) */
} Proto_Hdmi_StandbyCmd;

typedef struct _Proto_Hdmi_UhdDeepColour_UhdDCState {
    bool uhddcEnabled;
    uint32_t hdmiPort;
/* @@protoc_insertion_point(struct:Proto_Hdmi_UhdDeepColour_UhdDCState) */
} Proto_Hdmi_UhdDeepColour_UhdDCState;

/* Default values for struct fields */

/* Initializer values for message structs */
#define Proto_Hdmi_HdmiInput_init_default        {0}
#define Proto_Hdmi_StandbyCmd_init_default       {false, (Proto_Hdmi_StandbyCmd_Type)0}
#define Proto_Hdmi_Arc_init_default              {false, (Proto_Hdmi_Arc_Status)0}
#define Proto_Hdmi_AudioFormat_init_default      {{{NULL}, NULL}, {{NULL}, NULL}, false, (Proto_Hdmi_AudioFormat_AudioMode)0}
#define Proto_Hdmi_UhdDeepColour_init_default    {{{NULL}, NULL}}
#define Proto_Hdmi_UhdDeepColour_UhdDCState_init_default {0, 0}
#define Proto_Hdmi_InputSelected_init_default    {(Proto_Hdmi_InputSelected_AudioSelected)0, false, 0}
#define Proto_Hdmi_InputsSense_init_default      {{{NULL}, NULL}}
#define Proto_Hdmi_InputsSense_SenseState_init_default {0, 0}
#define Proto_Hdmi_InputSense_init_default       {0, 0}
#define Proto_Hdmi_AudioModeSelect_init_default  {false, (Proto_Hdmi_AudioModeSelect_Type)0}
#define Proto_Hdmi_HdmiInput_init_zero           {0}
#define Proto_Hdmi_StandbyCmd_init_zero          {false, (Proto_Hdmi_StandbyCmd_Type)0}
#define Proto_Hdmi_Arc_init_zero                 {false, (Proto_Hdmi_Arc_Status)0}
#define Proto_Hdmi_AudioFormat_init_zero         {{{NULL}, NULL}, {{NULL}, NULL}, false, (Proto_Hdmi_AudioFormat_AudioMode)0}
#define Proto_Hdmi_UhdDeepColour_init_zero       {{{NULL}, NULL}}
#define Proto_Hdmi_UhdDeepColour_UhdDCState_init_zero {0, 0}
#define Proto_Hdmi_InputSelected_init_zero       {(Proto_Hdmi_InputSelected_AudioSelected)0, false, 0}
#define Proto_Hdmi_InputsSense_init_zero         {{{NULL}, NULL}}
#define Proto_Hdmi_InputsSense_SenseState_init_zero {0, 0}
#define Proto_Hdmi_InputSense_init_zero          {0, 0}
#define Proto_Hdmi_AudioModeSelect_init_zero     {false, (Proto_Hdmi_AudioModeSelect_Type)0}

/* Field tags (for use in manual encoding/decoding) */
#define Proto_Hdmi_InputsSense_senseStatus_tag   1
#define Proto_Hdmi_UhdDeepColour_uhddcStatus_tag 1
#define Proto_Hdmi_Arc_status_tag                1
#define Proto_Hdmi_AudioFormat_audioInfoFrameData_tag 1
#define Proto_Hdmi_AudioFormat_channelStatusData_tag 2
#define Proto_Hdmi_AudioFormat_audioMode_tag     3
#define Proto_Hdmi_AudioModeSelect_type_tag      1
#define Proto_Hdmi_HdmiInput_number_tag          1
#define Proto_Hdmi_InputSelected_audio_tag       1
#define Proto_Hdmi_InputSelected_inputNumber_tag 2
#define Proto_Hdmi_InputSense_senseOn_tag        1
#define Proto_Hdmi_InputSense_hdmiPort_tag       2
#define Proto_Hdmi_InputsSense_SenseState_senseOn_tag 1
#define Proto_Hdmi_InputsSense_SenseState_hdmiPort_tag 2
#define Proto_Hdmi_StandbyCmd_type_tag           1
#define Proto_Hdmi_UhdDeepColour_UhdDCState_uhddcEnabled_tag 1
#define Proto_Hdmi_UhdDeepColour_UhdDCState_hdmiPort_tag 2

/* Struct field encoding specification for nanopb */
extern const pb_field_t Proto_Hdmi_HdmiInput_fields[2];
extern const pb_field_t Proto_Hdmi_StandbyCmd_fields[2];
extern const pb_field_t Proto_Hdmi_Arc_fields[2];
extern const pb_field_t Proto_Hdmi_AudioFormat_fields[4];
extern const pb_field_t Proto_Hdmi_UhdDeepColour_fields[2];
extern const pb_field_t Proto_Hdmi_UhdDeepColour_UhdDCState_fields[3];
extern const pb_field_t Proto_Hdmi_InputSelected_fields[3];
extern const pb_field_t Proto_Hdmi_InputsSense_fields[2];
extern const pb_field_t Proto_Hdmi_InputsSense_SenseState_fields[3];
extern const pb_field_t Proto_Hdmi_InputSense_fields[3];
extern const pb_field_t Proto_Hdmi_AudioModeSelect_fields[2];

/* Maximum encoded size of messages (where known) */
#define Proto_Hdmi_HdmiInput_size                6
#define Proto_Hdmi_StandbyCmd_size               2
#define Proto_Hdmi_Arc_size                      2
/* Proto_Hdmi_AudioFormat_size depends on runtime parameters */
/* Proto_Hdmi_UhdDeepColour_size depends on runtime parameters */
#define Proto_Hdmi_UhdDeepColour_UhdDCState_size 8
#define Proto_Hdmi_InputSelected_size            8
/* Proto_Hdmi_InputsSense_size depends on runtime parameters */
#define Proto_Hdmi_InputsSense_SenseState_size   8
#define Proto_Hdmi_InputSense_size               8
#define Proto_Hdmi_AudioModeSelect_size          2

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define HDMI_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
