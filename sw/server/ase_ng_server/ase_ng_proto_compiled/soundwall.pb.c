/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.7 at Tue May 16 18:29:08 2017. */

#include "soundwall.pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t Proto_SoundWall_A2Bmode_fields[2] = {
    PB_FIELD(  1, ENUM   , OPTIONAL, STATIC  , FIRST, Proto_SoundWall_A2Bmode, mode, mode, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_SoundWall_PowerMode_fields[2] = {
    PB_FIELD(  1, ENUM   , OPTIONAL, STATIC  , FIRST, Proto_SoundWall_PowerMode, mode, mode, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_SoundWall_RespGetTotalNodes_fields[2] = {
    PB_FIELD(  1, UINT32  , OPTIONAL, STATIC  , FIRST, Proto_SoundWall_RespGetTotalNodes, totalNodes, totalNodes, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_SoundWall_NodeIndex_fields[2] = {
    PB_FIELD(  1, UINT32  , OPTIONAL, STATIC  , FIRST, Proto_SoundWall_NodeIndex, nodeIndex, nodeIndex, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_SoundWall_MuteMode_fields[2] = {
    PB_FIELD(  1, ENUM   , OPTIONAL, STATIC  , FIRST, Proto_SoundWall_MuteMode, mode, mode, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_SoundWall_ReqTestTone_fields[3] = {
    PB_FIELD(  1, UINT32  , REQUIRED, STATIC  , FIRST, Proto_SoundWall_ReqTestTone, nodeIndex, nodeIndex, 0),
    PB_FIELD(  2, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_ReqTestTone, speakerTile, nodeIndex, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_SoundWall_GainAndDelay_fields[19] = {
    PB_FIELD(  2, UINT32  , REQUIRED, STATIC  , FIRST, Proto_SoundWall_GainAndDelay, nodeIndex, nodeIndex, 0),
    PB_FIELD(  3, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_gain_L_1, nodeIndex, 0),
    PB_FIELD(  4, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_gain_R_1, MT_TW_gain_L_1, 0),
    PB_FIELD(  5, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_gain_L_2, MT_TW_gain_R_1, 0),
    PB_FIELD(  6, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_gain_R_2, MT_TW_gain_L_2, 0),
    PB_FIELD(  7, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_gain_L_3, MT_TW_gain_R_2, 0),
    PB_FIELD(  8, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_gain_R_3, MT_TW_gain_L_3, 0),
    PB_FIELD(  9, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_gain_L_4, MT_TW_gain_R_3, 0),
    PB_FIELD( 10, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_gain_R_4, MT_TW_gain_L_4, 0),
    PB_FIELD( 11, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_delay_L_1, MT_TW_gain_R_4, 0),
    PB_FIELD( 12, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_delay_R_1, MT_TW_delay_L_1, 0),
    PB_FIELD( 13, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_delay_L_2, MT_TW_delay_R_1, 0),
    PB_FIELD( 14, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_delay_R_2, MT_TW_delay_L_2, 0),
    PB_FIELD( 15, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_delay_L_3, MT_TW_delay_R_2, 0),
    PB_FIELD( 16, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_delay_R_3, MT_TW_delay_L_3, 0),
    PB_FIELD( 17, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_delay_L_4, MT_TW_delay_R_3, 0),
    PB_FIELD( 18, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MT_TW_delay_R_4, MT_TW_delay_L_4, 0),
    PB_FIELD( 19, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_GainAndDelay, MUTE_L_R, MT_TW_delay_R_4, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_SoundWall_DriverGain_fields[14] = {
    PB_FIELD(  2, UINT32  , REQUIRED, STATIC  , FIRST, Proto_SoundWall_DriverGain, nodeIndex, nodeIndex, 0),
    PB_FIELD(  3, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_1_B_Cal_Gain, nodeIndex, 0),
    PB_FIELD(  4, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_1_MT_Cal_Gain, Tile_1_B_Cal_Gain, 0),
    PB_FIELD(  5, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_2_B_Cal_Gain, Tile_1_MT_Cal_Gain, 0),
    PB_FIELD(  6, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_2_MT_Cal_Gain, Tile_2_B_Cal_Gain, 0),
    PB_FIELD(  7, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_3_B_Cal_Gain, Tile_2_MT_Cal_Gain, 0),
    PB_FIELD(  8, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_3_MT_Cal_Gain, Tile_3_B_Cal_Gain, 0),
    PB_FIELD(  9, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_4_B_Cal_Gain, Tile_3_MT_Cal_Gain, 0),
    PB_FIELD( 10, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_4_MT_Cal_Gain, Tile_4_B_Cal_Gain, 0),
    PB_FIELD( 11, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_1_TW_Cal_Gain, Tile_4_MT_Cal_Gain, 0),
    PB_FIELD( 12, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_2_TW_Cal_Gain, Tile_1_TW_Cal_Gain, 0),
    PB_FIELD( 13, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_3_TW_Cal_Gain, Tile_2_TW_Cal_Gain, 0),
    PB_FIELD( 14, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DriverGain, Tile_4_TW_Cal_Gain, Tile_3_TW_Cal_Gain, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_SoundWall_DspEqParam_fields[6] = {
    PB_FIELD(  1, UINT32  , REQUIRED, STATIC  , FIRST, Proto_SoundWall_DspEqParam, param1, param1, 0),
    PB_FIELD(  2, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DspEqParam, param2, param1, 0),
    PB_FIELD(  3, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DspEqParam, param3, param2, 0),
    PB_FIELD(  4, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DspEqParam, param4, param3, 0),
    PB_FIELD(  5, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_DspEqParam, param5, param4, 0),
    PB_LAST_FIELD
};

const pb_field_t Proto_SoundWall_BassAndRoomEQ_fields[4] = {
    PB_FIELD(  2, UINT32  , REQUIRED, STATIC  , FIRST, Proto_SoundWall_BassAndRoomEQ, nodeIndex, nodeIndex, 0),
    PB_FIELD(  3, UINT32  , REQUIRED, STATIC  , OTHER, Proto_SoundWall_BassAndRoomEQ, bass_gain, nodeIndex, 0),
    PB_FIELD(  4, MESSAGE , REPEATED, CALLBACK, OTHER, Proto_SoundWall_BassAndRoomEQ, eqParam, bass_gain, &Proto_SoundWall_DspEqParam_fields),
    PB_LAST_FIELD
};


/* @@protoc_insertion_point(eof) */
