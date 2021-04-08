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
 * Get Dsp version from dsp flow
 * return value : 32.0 format
 * e.g 123 = Ver1.2.3, 5 = Ver0.0.5
 */
uint32_t Adau1452Drv_DspVersion(void);

#endif  // __ADAU1452_DRV_H__


