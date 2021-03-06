/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.7 at Tue May 16 18:29:07 2017. */

#include "light-sensor.pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t Proto_LightSensor_Command_fields[3] = {
    PB_FIELD(  1, ENUM   , REQUIRED, STATIC  , FIRST, Proto_LightSensor_Command, type, type, 0),
    PB_FIELD(  2, UINT32  , OPTIONAL, STATIC  , OTHER, Proto_LightSensor_Command, calibrationLightLevel, type, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_LightSensor_ReplyData_fields[6] = {
    PB_FIELD(  1, BOOL    , REQUIRED, STATIC  , FIRST, Proto_LightSensor_ReplyData, successful, successful, 0),
    PB_FIELD(  2, UINT32  , OPTIONAL, STATIC  , OTHER, Proto_LightSensor_ReplyData, rawCombinedValue, successful, 0),
    PB_FIELD(  3, UINT32  , OPTIONAL, STATIC  , OTHER, Proto_LightSensor_ReplyData, rawIrValue, rawCombinedValue, 0),
    PB_FIELD(  4, UINT32  , OPTIONAL, STATIC  , OTHER, Proto_LightSensor_ReplyData, calibrationConst, rawIrValue, 0),
    PB_FIELD(  5, UINT32  , OPTIONAL, STATIC  , OTHER, Proto_LightSensor_ReplyData, lux, calibrationConst, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_LightSensor_LightLevel_fields[2] = {
    PB_FIELD(  1, UINT32  , REQUIRED, STATIC  , FIRST, Proto_LightSensor_LightLevel, lux, lux, 0),
    PB_LAST_FIELD
};


/* @@protoc_insertion_point(eof) */
