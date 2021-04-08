#ifndef __ADAU1452_DRV_H__
#define __ADAU1452_DRV_H__

typedef struct tagDspInitStage
{
    void (*initSectionFunc)(void);         // function point to a specific initialization section
    uint16_t delay_time;   // time duration to next intialization section in ms
}DspInitStage_t;

typedef enum tagDspAsrcChannel
{
    DSP_Asrc_Codec,    // from CS42528 codec
    DSP_Asrc_A2B,    // from A2B chip
    DSP_Asrc_Spdif,    // from Spdif in
    DSP_Asrc_MAX
}DspAsrcChannel_t;

/**
 * Construct the DSP driver instance.
 * @return : none
 */
void Adau1452Drv_Ctor(void);

/**
 * Exit & clean up the driver.
 */
void Adau1452Drv_Xtor(void);

/**
 * Enable/Disalbe the I2C bus access
 */
void Adau1452Drv_I2cEnable(bool enable);

/**
 * Write the dsp program/parameter data.
 */
uint16_t Adau1452Drv_Init(void);

/*
 * select the Asrc In source
 */
void Adau1452Drv_AsrcInSource(DspAsrcChannel_t asrc_source);

/* 
 * setup the volume, range from 0 ~ (MAX_VOLUME_STEPS-1)
 */
void Adau1452Drv_SetVolume(uint32_t vol_value);

/* 
 * bypass enable/disable
 * NOTE : dsp flow related
 */
void Adau1452Drv_BypassEnable(bool enable);

/* 
 * channel mute control for production test
 * NOTE : dsp flow related
 */
void Adau1452Drv_ChannelMute(uint32_t param);

/* 
 * Delay1 Module
 * NOTE : dsp flow related
 */
void Adau1452Drv_SetupDelay1(uint16_t delay);

/* 
 * Delay2 Module
 * NOTE : dsp flow related
 */
void Adau1452Drv_SetupDelay2(uint16_t delay);

/* 
 * Delay3 Module
 * NOTE : dsp flow related
 */
void Adau1452Drv_SetupDelay3(uint16_t delay);

/* 
 * Delay4 Module
 * NOTE : dsp flow related
 */
void Adau1452Drv_SetupDelay4(uint16_t delay);

/* 
 * Get Dsp version from dsp flow
 * return value : 32.0 format
 * e.g 123 = Ver1.2.3, 5 = Ver0.0.5
 */
uint32_t Adau1452Drv_DspVersion(void);

/*
uint32_t Adau1452Drv_CodecSignalDetect(void);
uint32_t Adau1452Drv_A2BSignalDetect(void);
uint32_t Adau1452Drv_SpdifSignalDetect(void);
*/
#ifdef HAS_AUDIO_MUTE_CHANNEL
void Adau1452Drv_EnableTestTone(bool enable);
#endif
#ifdef HAS_TEMPERATURE_MONITOR
void Adau1452_UpdateNtcValue();
#endif

#ifdef HAS_SOUNDMODE_UPDATE
typedef struct tagSoundModeGainAndDelay {
    uint32_t MT_TW_gain_L_1;
    uint32_t MT_TW_gain_R_1;
    uint32_t MT_TW_gain_L_2;
    uint32_t MT_TW_gain_R_2;
    uint32_t MT_TW_gain_L_3;
    uint32_t MT_TW_gain_R_3;
    uint32_t MT_TW_gain_L_4;
    uint32_t MT_TW_gain_R_4;
    uint32_t MT_TW_delay_L_1;
    uint32_t MT_TW_delay_R_1;
    uint32_t MT_TW_delay_L_2;
    uint32_t MT_TW_delay_R_2;
    uint32_t MT_TW_delay_L_3;
    uint32_t MT_TW_delay_R_3;
    uint32_t MT_TW_delay_L_4;
    uint32_t MT_TW_delay_R_4;
    uint32_t MUTE_L_R;
/* @@protoc_insertion_point(struct:Proto_SoundWall_GainAndDelay) */
}SoundModeGainAndDelay_t;

typedef struct tagSoundModeDriverGain {
    uint32_t Tile_1_B_Cal_Gain;
    uint32_t Tile_1_MT_Cal_Gain;
    uint32_t Tile_2_B_Cal_Gain;
    uint32_t Tile_2_MT_Cal_Gain;
    uint32_t Tile_3_B_Cal_Gain;
    uint32_t Tile_3_MT_Cal_Gain;
    uint32_t Tile_4_B_Cal_Gain;
    uint32_t Tile_4_MT_Cal_Gain;
    uint32_t Tile_1_TW_Cal_Gain;
    uint32_t Tile_2_TW_Cal_Gain;
    uint32_t Tile_3_TW_Cal_Gain;
    uint32_t Tile_4_TW_Cal_Gain;
/* @@protoc_insertion_point(struct:Proto_SoundWall_DriverGain) */
} SoundModeDriverGain_t;


typedef struct tagSoundModeBassAndRoomEQ {
    uint32_t bass_gain;
    Proto_SoundWall_DspEqParam eq[12];
/* @@protoc_insertion_point(struct:Proto_SoundWall_BassAndRoomEQ) */
} SoundModeBassAndRoomEQ_t;

bool Adau1452Drv_UpdateGainAndDelay(Proto_SoundWall_GainAndDelay * p_gain_and_delay);
#endif

#endif  // __ADAU1452_DRV_H__


