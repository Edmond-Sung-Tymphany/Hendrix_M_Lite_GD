/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.7 at Mon Apr 17 15:09:19 2017. */

#ifndef PB_PROTO_DSP_DSP_PB_H_INCLUDED
#define PB_PROTO_DSP_DSP_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _Proto_Dsp_LineInSensitivity_Sensitivity {
    Proto_Dsp_LineInSensitivity_Sensitivity_HIGH = 0,
    Proto_Dsp_LineInSensitivity_Sensitivity_MEDIUM = 1,
    Proto_Dsp_LineInSensitivity_Sensitivity_LOW = 2,
    Proto_Dsp_LineInSensitivity_Sensitivity_DISABLED = 3
} Proto_Dsp_LineInSensitivity_Sensitivity;
#define _Proto_Dsp_LineInSensitivity_Sensitivity_MIN Proto_Dsp_LineInSensitivity_Sensitivity_HIGH
#define _Proto_Dsp_LineInSensitivity_Sensitivity_MAX Proto_Dsp_LineInSensitivity_Sensitivity_DISABLED
#define _Proto_Dsp_LineInSensitivity_Sensitivity_ARRAYSIZE ((Proto_Dsp_LineInSensitivity_Sensitivity)(Proto_Dsp_LineInSensitivity_Sensitivity_DISABLED+1))

typedef enum _Proto_Dsp_RequestAudioInput_AudioInput {
    Proto_Dsp_RequestAudioInput_AudioInput_ASE = 0,
    Proto_Dsp_RequestAudioInput_AudioInput_LINE = 1,
    Proto_Dsp_RequestAudioInput_AudioInput_TOS_LINK = 2,
    Proto_Dsp_RequestAudioInput_AudioInput_POWER_LINK = 3,
    Proto_Dsp_RequestAudioInput_AudioInput_HDMI = 4,
    Proto_Dsp_RequestAudioInput_AudioInput_WIRELESS_MULTICHANNEL = 5,
    Proto_Dsp_RequestAudioInput_AudioInput_HDMI_ARC = 6,
    Proto_Dsp_RequestAudioInput_AudioInput_MICROPHONE_1 = 7,
    Proto_Dsp_RequestAudioInput_AudioInput_MICROPHONE_2 = 8
} Proto_Dsp_RequestAudioInput_AudioInput;
#define _Proto_Dsp_RequestAudioInput_AudioInput_MIN Proto_Dsp_RequestAudioInput_AudioInput_ASE
#define _Proto_Dsp_RequestAudioInput_AudioInput_MAX Proto_Dsp_RequestAudioInput_AudioInput_MICROPHONE_2
#define _Proto_Dsp_RequestAudioInput_AudioInput_ARRAYSIZE ((Proto_Dsp_RequestAudioInput_AudioInput)(Proto_Dsp_RequestAudioInput_AudioInput_MICROPHONE_2+1))

typedef enum _Proto_Dsp_PositionSoundMode_Position {
    Proto_Dsp_PositionSoundMode_Position_UNDEFINED = 0,
    Proto_Dsp_PositionSoundMode_Position_FREE = 1,
    Proto_Dsp_PositionSoundMode_Position_WALL = 2,
    Proto_Dsp_PositionSoundMode_Position_CORNER = 3,
    Proto_Dsp_PositionSoundMode_Position_TABLE = 4
} Proto_Dsp_PositionSoundMode_Position;
#define _Proto_Dsp_PositionSoundMode_Position_MIN Proto_Dsp_PositionSoundMode_Position_UNDEFINED
#define _Proto_Dsp_PositionSoundMode_Position_MAX Proto_Dsp_PositionSoundMode_Position_TABLE
#define _Proto_Dsp_PositionSoundMode_Position_ARRAYSIZE ((Proto_Dsp_PositionSoundMode_Position)(Proto_Dsp_PositionSoundMode_Position_TABLE+1))

typedef enum _Proto_Dsp_PositionSoundMode_Orientation {
    Proto_Dsp_PositionSoundMode_Orientation_NONE = 0,
    Proto_Dsp_PositionSoundMode_Orientation_HORIZONTAL = 1,
    Proto_Dsp_PositionSoundMode_Orientation_VERTICAL = 2
} Proto_Dsp_PositionSoundMode_Orientation;
#define _Proto_Dsp_PositionSoundMode_Orientation_MIN Proto_Dsp_PositionSoundMode_Orientation_NONE
#define _Proto_Dsp_PositionSoundMode_Orientation_MAX Proto_Dsp_PositionSoundMode_Orientation_VERTICAL
#define _Proto_Dsp_PositionSoundMode_Orientation_ARRAYSIZE ((Proto_Dsp_PositionSoundMode_Orientation)(Proto_Dsp_PositionSoundMode_Orientation_VERTICAL+1))

typedef enum _Proto_Dsp_ResponsePositionSoundMode_Error {
    Proto_Dsp_ResponsePositionSoundMode_Error_NO_ERROR = 1,
    Proto_Dsp_ResponsePositionSoundMode_Error_POSITION_ERROR = 2,
    Proto_Dsp_ResponsePositionSoundMode_Error_ORIENTATION_ERROR = 3,
    Proto_Dsp_ResponsePositionSoundMode_Error_COMBINATION_ERROR = 4
} Proto_Dsp_ResponsePositionSoundMode_Error;
#define _Proto_Dsp_ResponsePositionSoundMode_Error_MIN Proto_Dsp_ResponsePositionSoundMode_Error_NO_ERROR
#define _Proto_Dsp_ResponsePositionSoundMode_Error_MAX Proto_Dsp_ResponsePositionSoundMode_Error_COMBINATION_ERROR
#define _Proto_Dsp_ResponsePositionSoundMode_Error_ARRAYSIZE ((Proto_Dsp_ResponsePositionSoundMode_Error)(Proto_Dsp_ResponsePositionSoundMode_Error_COMBINATION_ERROR+1))

typedef enum _Proto_Dsp_InternalSpeaker_Position {
    Proto_Dsp_InternalSpeaker_Position_LEFT = 0,
    Proto_Dsp_InternalSpeaker_Position_RIGHT = 1,
    Proto_Dsp_InternalSpeaker_Position_CENTRE = 2
} Proto_Dsp_InternalSpeaker_Position;
#define _Proto_Dsp_InternalSpeaker_Position_MIN Proto_Dsp_InternalSpeaker_Position_LEFT
#define _Proto_Dsp_InternalSpeaker_Position_MAX Proto_Dsp_InternalSpeaker_Position_CENTRE
#define _Proto_Dsp_InternalSpeaker_Position_ARRAYSIZE ((Proto_Dsp_InternalSpeaker_Position)(Proto_Dsp_InternalSpeaker_Position_CENTRE+1))

typedef enum _Proto_Dsp_InternalSpeaker_Type {
    Proto_Dsp_InternalSpeaker_Type_TWEETER = 0,
    Proto_Dsp_InternalSpeaker_Type_MIDRANGE = 1,
    Proto_Dsp_InternalSpeaker_Type_WOOFER = 2,
    Proto_Dsp_InternalSpeaker_Type_FULLRANGE = 3
} Proto_Dsp_InternalSpeaker_Type;
#define _Proto_Dsp_InternalSpeaker_Type_MIN Proto_Dsp_InternalSpeaker_Type_TWEETER
#define _Proto_Dsp_InternalSpeaker_Type_MAX Proto_Dsp_InternalSpeaker_Type_FULLRANGE
#define _Proto_Dsp_InternalSpeaker_Type_ARRAYSIZE ((Proto_Dsp_InternalSpeaker_Type)(Proto_Dsp_InternalSpeaker_Type_FULLRANGE+1))

typedef enum _Proto_Dsp_ResponseInternalSpeakerCompensation_Error {
    Proto_Dsp_ResponseInternalSpeakerCompensation_Error_NO_ERROR = 1,
    Proto_Dsp_ResponseInternalSpeakerCompensation_Error_POSITION_ERROR = 2,
    Proto_Dsp_ResponseInternalSpeakerCompensation_Error_TYPE_ERROR = 3,
    Proto_Dsp_ResponseInternalSpeakerCompensation_Error_POSITION_TYPE_COMBINATION_ERROR = 4,
    Proto_Dsp_ResponseInternalSpeakerCompensation_Error_GAIN_ERROR = 5
} Proto_Dsp_ResponseInternalSpeakerCompensation_Error;
#define _Proto_Dsp_ResponseInternalSpeakerCompensation_Error_MIN Proto_Dsp_ResponseInternalSpeakerCompensation_Error_NO_ERROR
#define _Proto_Dsp_ResponseInternalSpeakerCompensation_Error_MAX Proto_Dsp_ResponseInternalSpeakerCompensation_Error_GAIN_ERROR
#define _Proto_Dsp_ResponseInternalSpeakerCompensation_Error_ARRAYSIZE ((Proto_Dsp_ResponseInternalSpeakerCompensation_Error)(Proto_Dsp_ResponseInternalSpeakerCompensation_Error_GAIN_ERROR+1))

typedef enum _Proto_Dsp_InternalAmplifierCommand_State {
    Proto_Dsp_InternalAmplifierCommand_State_OFF = 0,
    Proto_Dsp_InternalAmplifierCommand_State_ON = 1
} Proto_Dsp_InternalAmplifierCommand_State;
#define _Proto_Dsp_InternalAmplifierCommand_State_MIN Proto_Dsp_InternalAmplifierCommand_State_OFF
#define _Proto_Dsp_InternalAmplifierCommand_State_MAX Proto_Dsp_InternalAmplifierCommand_State_ON
#define _Proto_Dsp_InternalAmplifierCommand_State_ARRAYSIZE ((Proto_Dsp_InternalAmplifierCommand_State)(Proto_Dsp_InternalAmplifierCommand_State_ON+1))

typedef enum _Proto_Dsp_SpeakerEnableCommand_Speaker_Id {
    Proto_Dsp_SpeakerEnableCommand_Speaker_Id_FRONT = 0,
    Proto_Dsp_SpeakerEnableCommand_Speaker_Id_REAR = 1
} Proto_Dsp_SpeakerEnableCommand_Speaker_Id;
#define _Proto_Dsp_SpeakerEnableCommand_Speaker_Id_MIN Proto_Dsp_SpeakerEnableCommand_Speaker_Id_FRONT
#define _Proto_Dsp_SpeakerEnableCommand_Speaker_Id_MAX Proto_Dsp_SpeakerEnableCommand_Speaker_Id_REAR
#define _Proto_Dsp_SpeakerEnableCommand_Speaker_Id_ARRAYSIZE ((Proto_Dsp_SpeakerEnableCommand_Speaker_Id)(Proto_Dsp_SpeakerEnableCommand_Speaker_Id_REAR+1))

typedef enum _Proto_Dsp_NTCDataEvent_NTCValue_NTC {
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_EXT_NTC1 = 0,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_EXT_NTC2 = 1,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_EXT_NTC3 = 2,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_EXT_NTC4 = 3,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_EXT_NTC5 = 4,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_EXT_NTC6 = 5,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_EXT_NTC7 = 6,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_EXT_NTC8 = 7,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_AMP_NTC_CH1 = 8,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_AMP_NTC_CH2_3 = 9,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_AMP_NTC_CH4 = 10,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_AMP_NTC_CH5 = 11,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_AMP_NTC_CH6_7 = 12,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_AMP_NTC_CH8 = 13,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_NTC_PSU = 14,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_NTC_DSP = 15,
    Proto_Dsp_NTCDataEvent_NTCValue_NTC_NTC_AMP = 16
} Proto_Dsp_NTCDataEvent_NTCValue_NTC;
#define _Proto_Dsp_NTCDataEvent_NTCValue_NTC_MIN Proto_Dsp_NTCDataEvent_NTCValue_NTC_EXT_NTC1
#define _Proto_Dsp_NTCDataEvent_NTCValue_NTC_MAX Proto_Dsp_NTCDataEvent_NTCValue_NTC_NTC_AMP
#define _Proto_Dsp_NTCDataEvent_NTCValue_NTC_ARRAYSIZE ((Proto_Dsp_NTCDataEvent_NTCValue_NTC)(Proto_Dsp_NTCDataEvent_NTCValue_NTC_NTC_AMP+1))

typedef enum _Proto_Dsp_Parameter_Type {
    Proto_Dsp_Parameter_Type_LOUDNESS = 0,
    Proto_Dsp_Parameter_Type_BASS = 1,
    Proto_Dsp_Parameter_Type_TREBLE = 2
} Proto_Dsp_Parameter_Type;
#define _Proto_Dsp_Parameter_Type_MIN Proto_Dsp_Parameter_Type_LOUDNESS
#define _Proto_Dsp_Parameter_Type_MAX Proto_Dsp_Parameter_Type_TREBLE
#define _Proto_Dsp_Parameter_Type_ARRAYSIZE ((Proto_Dsp_Parameter_Type)(Proto_Dsp_Parameter_Type_TREBLE+1))

/* Struct definitions */
typedef struct _Proto_Dsp_AbsoluteVolume {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_AbsoluteVolume) */
} Proto_Dsp_AbsoluteVolume;

typedef struct _Proto_Dsp_InternalAmplifierCommand {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_InternalAmplifierCommand) */
} Proto_Dsp_InternalAmplifierCommand;

typedef struct _Proto_Dsp_InternalSpeaker {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_InternalSpeaker) */
} Proto_Dsp_InternalSpeaker;

typedef struct _Proto_Dsp_LineInSensitivity {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_LineInSensitivity) */
} Proto_Dsp_LineInSensitivity;

typedef struct _Proto_Dsp_Mute {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_Mute) */
} Proto_Dsp_Mute;

typedef struct _Proto_Dsp_NTCDataEvent {
    pb_callback_t ntcValues;
/* @@protoc_insertion_point(struct:Proto_Dsp_NTCDataEvent) */
} Proto_Dsp_NTCDataEvent;

typedef struct _Proto_Dsp_NTCDataEvent_NTCValue {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_NTCDataEvent_NTCValue) */
} Proto_Dsp_NTCDataEvent_NTCValue;

typedef struct _Proto_Dsp_Parameter {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_Parameter) */
} Proto_Dsp_Parameter;

typedef struct _Proto_Dsp_PositionSoundMode {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_PositionSoundMode) */
} Proto_Dsp_PositionSoundMode;

typedef struct _Proto_Dsp_RelativeVolumeChange {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_RelativeVolumeChange) */
} Proto_Dsp_RelativeVolumeChange;

typedef struct _Proto_Dsp_RequestInternalSpeakerCompensation {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_RequestInternalSpeakerCompensation) */
} Proto_Dsp_RequestInternalSpeakerCompensation;

typedef struct _Proto_Dsp_RequestPositionSoundMode {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_RequestPositionSoundMode) */
} Proto_Dsp_RequestPositionSoundMode;

typedef struct _Proto_Dsp_ResponseInternalSpeakerCompensation {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_ResponseInternalSpeakerCompensation) */
} Proto_Dsp_ResponseInternalSpeakerCompensation;

typedef struct _Proto_Dsp_ResponsePositionSoundMode {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_ResponsePositionSoundMode) */
} Proto_Dsp_ResponsePositionSoundMode;

typedef struct _Proto_Dsp_SpeakerEnableCommand {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_SpeakerEnableCommand) */
} Proto_Dsp_SpeakerEnableCommand;

typedef struct _Proto_Dsp_SpeakerEnableCommand_Speaker {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_SpeakerEnableCommand_Speaker) */
} Proto_Dsp_SpeakerEnableCommand_Speaker;

typedef struct _Proto_Dsp_ToslinkOutSampleRate {
    char dummy_field;
/* @@protoc_insertion_point(struct:Proto_Dsp_ToslinkOutSampleRate) */
} Proto_Dsp_ToslinkOutSampleRate;

typedef struct _Proto_Dsp_RequestAudioInput {
    bool has_input;
    Proto_Dsp_RequestAudioInput_AudioInput input;
/* @@protoc_insertion_point(struct:Proto_Dsp_RequestAudioInput) */
} Proto_Dsp_RequestAudioInput;

/* Default values for struct fields */

/* Initializer values for message structs */
#define Proto_Dsp_LineInSensitivity_init_default {0}
#define Proto_Dsp_AbsoluteVolume_init_default    {0}
#define Proto_Dsp_RelativeVolumeChange_init_default {0}
#define Proto_Dsp_Mute_init_default              {0}
#define Proto_Dsp_RequestAudioInput_init_default {false, (Proto_Dsp_RequestAudioInput_AudioInput)0}
#define Proto_Dsp_PositionSoundMode_init_default {0}
#define Proto_Dsp_RequestPositionSoundMode_init_default {0}
#define Proto_Dsp_ResponsePositionSoundMode_init_default {0}
#define Proto_Dsp_InternalSpeaker_init_default   {0}
#define Proto_Dsp_RequestInternalSpeakerCompensation_init_default {0}
#define Proto_Dsp_ResponseInternalSpeakerCompensation_init_default {0}
#define Proto_Dsp_InternalAmplifierCommand_init_default {0}
#define Proto_Dsp_SpeakerEnableCommand_init_default {0}
#define Proto_Dsp_SpeakerEnableCommand_Speaker_init_default {0}
#define Proto_Dsp_NTCDataEvent_init_default      {{{NULL}, NULL}}
#define Proto_Dsp_NTCDataEvent_NTCValue_init_default {0}
#define Proto_Dsp_ToslinkOutSampleRate_init_default {0}
#define Proto_Dsp_Parameter_init_default         {0}
#define Proto_Dsp_LineInSensitivity_init_zero    {0}
#define Proto_Dsp_AbsoluteVolume_init_zero       {0}
#define Proto_Dsp_RelativeVolumeChange_init_zero {0}
#define Proto_Dsp_Mute_init_zero                 {0}
#define Proto_Dsp_RequestAudioInput_init_zero    {false, (Proto_Dsp_RequestAudioInput_AudioInput)0}
#define Proto_Dsp_PositionSoundMode_init_zero    {0}
#define Proto_Dsp_RequestPositionSoundMode_init_zero {0}
#define Proto_Dsp_ResponsePositionSoundMode_init_zero {0}
#define Proto_Dsp_InternalSpeaker_init_zero      {0}
#define Proto_Dsp_RequestInternalSpeakerCompensation_init_zero {0}
#define Proto_Dsp_ResponseInternalSpeakerCompensation_init_zero {0}
#define Proto_Dsp_InternalAmplifierCommand_init_zero {0}
#define Proto_Dsp_SpeakerEnableCommand_init_zero {0}
#define Proto_Dsp_SpeakerEnableCommand_Speaker_init_zero {0}
#define Proto_Dsp_NTCDataEvent_init_zero         {{{NULL}, NULL}}
#define Proto_Dsp_NTCDataEvent_NTCValue_init_zero {0}
#define Proto_Dsp_ToslinkOutSampleRate_init_zero {0}
#define Proto_Dsp_Parameter_init_zero            {0}

/* Field tags (for use in manual encoding/decoding) */
#define Proto_Dsp_NTCDataEvent_ntcValues_tag     1
#define Proto_Dsp_RequestAudioInput_input_tag    1

/* Struct field encoding specification for nanopb */
extern const pb_field_t Proto_Dsp_LineInSensitivity_fields[1];
extern const pb_field_t Proto_Dsp_AbsoluteVolume_fields[1];
extern const pb_field_t Proto_Dsp_RelativeVolumeChange_fields[1];
extern const pb_field_t Proto_Dsp_Mute_fields[1];
extern const pb_field_t Proto_Dsp_RequestAudioInput_fields[2];
extern const pb_field_t Proto_Dsp_PositionSoundMode_fields[1];
extern const pb_field_t Proto_Dsp_RequestPositionSoundMode_fields[1];
extern const pb_field_t Proto_Dsp_ResponsePositionSoundMode_fields[1];
extern const pb_field_t Proto_Dsp_InternalSpeaker_fields[1];
extern const pb_field_t Proto_Dsp_RequestInternalSpeakerCompensation_fields[1];
extern const pb_field_t Proto_Dsp_ResponseInternalSpeakerCompensation_fields[1];
extern const pb_field_t Proto_Dsp_InternalAmplifierCommand_fields[1];
extern const pb_field_t Proto_Dsp_SpeakerEnableCommand_fields[1];
extern const pb_field_t Proto_Dsp_SpeakerEnableCommand_Speaker_fields[1];
extern const pb_field_t Proto_Dsp_NTCDataEvent_fields[2];
extern const pb_field_t Proto_Dsp_NTCDataEvent_NTCValue_fields[1];
extern const pb_field_t Proto_Dsp_ToslinkOutSampleRate_fields[1];
extern const pb_field_t Proto_Dsp_Parameter_fields[1];

/* Maximum encoded size of messages (where known) */
#define Proto_Dsp_LineInSensitivity_size         0
#define Proto_Dsp_AbsoluteVolume_size            0
#define Proto_Dsp_RelativeVolumeChange_size      0
#define Proto_Dsp_Mute_size                      0
#define Proto_Dsp_RequestAudioInput_size         2
#define Proto_Dsp_PositionSoundMode_size         0
#define Proto_Dsp_RequestPositionSoundMode_size  0
#define Proto_Dsp_ResponsePositionSoundMode_size 0
#define Proto_Dsp_InternalSpeaker_size           0
#define Proto_Dsp_RequestInternalSpeakerCompensation_size 0
#define Proto_Dsp_ResponseInternalSpeakerCompensation_size 0
#define Proto_Dsp_InternalAmplifierCommand_size  0
#define Proto_Dsp_SpeakerEnableCommand_size      0
#define Proto_Dsp_SpeakerEnableCommand_Speaker_size 0
/* Proto_Dsp_NTCDataEvent_size depends on runtime parameters */
#define Proto_Dsp_NTCDataEvent_NTCValue_size     0
#define Proto_Dsp_ToslinkOutSampleRate_size      0
#define Proto_Dsp_Parameter_size                 0

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define DSP_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif