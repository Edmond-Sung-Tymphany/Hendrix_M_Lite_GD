/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.7 at Tue May 16 18:29:07 2017. */

#ifndef PB_PROTO_LIGHTSENSOR_LIGHT_SENSOR_PB_H_INCLUDED
#define PB_PROTO_LIGHTSENSOR_LIGHT_SENSOR_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _Proto_LightSensor_Command_Type {
    Proto_LightSensor_Command_Type_INITIALIZE = 0,
    Proto_LightSensor_Command_Type_READ_RAW = 1,
    Proto_LightSensor_Command_Type_CALIBRATE = 2,
    Proto_LightSensor_Command_Type_READ_CALIBRATION = 3,
    Proto_LightSensor_Command_Type_READ_LIGHT_LEVEL = 4
} Proto_LightSensor_Command_Type;
#define _Proto_LightSensor_Command_Type_MIN Proto_LightSensor_Command_Type_INITIALIZE
#define _Proto_LightSensor_Command_Type_MAX Proto_LightSensor_Command_Type_READ_LIGHT_LEVEL
#define _Proto_LightSensor_Command_Type_ARRAYSIZE ((Proto_LightSensor_Command_Type)(Proto_LightSensor_Command_Type_READ_LIGHT_LEVEL+1))

/* Struct definitions */
typedef struct _Proto_LightSensor_Command {
    Proto_LightSensor_Command_Type type;
    bool has_calibrationLightLevel;
    uint32_t calibrationLightLevel;
/* @@protoc_insertion_point(struct:Proto_LightSensor_Command) */
} Proto_LightSensor_Command;

typedef struct _Proto_LightSensor_LightLevel {
    uint32_t lux;
/* @@protoc_insertion_point(struct:Proto_LightSensor_LightLevel) */
} Proto_LightSensor_LightLevel;

typedef struct _Proto_LightSensor_ReplyData {
    bool successful;
    bool has_rawCombinedValue;
    uint32_t rawCombinedValue;
    bool has_rawIrValue;
    uint32_t rawIrValue;
    bool has_calibrationConst;
    uint32_t calibrationConst;
    bool has_lux;
    uint32_t lux;
/* @@protoc_insertion_point(struct:Proto_LightSensor_ReplyData) */
} Proto_LightSensor_ReplyData;

/* Default values for struct fields */

/* Initializer values for message structs */
#define Proto_LightSensor_Command_init_default   {(Proto_LightSensor_Command_Type)0, false, 0}
#define Proto_LightSensor_ReplyData_init_default {0, false, 0, false, 0, false, 0, false, 0}
#define Proto_LightSensor_LightLevel_init_default {0}
#define Proto_LightSensor_Command_init_zero      {(Proto_LightSensor_Command_Type)0, false, 0}
#define Proto_LightSensor_ReplyData_init_zero    {0, false, 0, false, 0, false, 0, false, 0}
#define Proto_LightSensor_LightLevel_init_zero   {0}

/* Field tags (for use in manual encoding/decoding) */
#define Proto_LightSensor_Command_type_tag       1
#define Proto_LightSensor_Command_calibrationLightLevel_tag 2
#define Proto_LightSensor_LightLevel_lux_tag     1
#define Proto_LightSensor_ReplyData_successful_tag 1
#define Proto_LightSensor_ReplyData_rawCombinedValue_tag 2
#define Proto_LightSensor_ReplyData_rawIrValue_tag 3
#define Proto_LightSensor_ReplyData_calibrationConst_tag 4
#define Proto_LightSensor_ReplyData_lux_tag      5

/* Struct field encoding specification for nanopb */
extern const pb_field_t Proto_LightSensor_Command_fields[3];
extern const pb_field_t Proto_LightSensor_ReplyData_fields[6];
extern const pb_field_t Proto_LightSensor_LightLevel_fields[2];

/* Maximum encoded size of messages (where known) */
#define Proto_LightSensor_Command_size           8
#define Proto_LightSensor_ReplyData_size         26
#define Proto_LightSensor_LightLevel_size        6

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define LIGHT_SENSOR_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
