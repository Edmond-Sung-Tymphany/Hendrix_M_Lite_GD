#ifndef __ADAU1761_DRV_H__
#define __ADAU1761_DRV_H__

typedef struct tagDspInitStage
{
    void (*initSectionFunc)(void);         // function point to a specific initialization section
    uint16_t delay_time;   // time duration to next intialization section in ms
}DspInitStage_t;

typedef enum tagDspAnalogIn
{
    DSP_ANALOG_AUX_IN,  // LAUX/RAUX in
    DSP_ANALOG_LINE_IN, // LINEN/LINEP in
    DSP_ANALOG_IN_MAX
}DspAnalogIn_t;

typedef enum tagDspI2SChannel
{
    DSP_AUXIN,
    DSP_RCA,
    DSP_I2S,
    DSP_I2S_MAX
}DspI2SChannel_t;

/**
 * Construct the DSP driver instance.
 * @return : none
 */
void Adau1761Drv_Ctor(void);

/**
 * Exit & clean up the driver.
 */
void Adau1761Drv_Xtor(void);

/**
 * Enable/Disalbe the I2C bus access
 */
void Adau1761Drv_I2cEnable(bool enable);

/**
 * Write the dsp program/parameter data.
 */
uint16_t Adau1761Drv_Init(void);

/* 
 * mute the LOUTP/N & ROUTP/N
 */
void Adau1761Drv_DacMute(bool mute_enable);

/*
 * setup the DSP to AUX-in or LINE-in
 */
void Adau1761Drv_AnalogInSource(DspAnalogIn_t analog_in);

/*
 * select the I2S source, ADC-in or I2S in
 */
void Adau1761Drv_I2SInSource(DspI2SChannel_t i2s_source);

/* 
 * mute the I2S input in Sigma studio data flow
 * dsp project need the 'mute' module to support
 */
void Adau1761Drv_I2SMute(bool mute_enable);

/* 
 * setup the volume, range from 0 ~ (MAX_VOLUME_STEPS-1)
 * NOTE : the volume control module must be "shared slider" mode.
 */
void Adau1761Drv_SetVolume(uint32_t vol_value);

/* 
 * setup the bass, range from 0 ~ (MAX_BASS_STEPS-1)
 * NOTE : dsp flow related
 */
void Adau1761Drv_SetBass(uint32_t bass_level);

/* 
 * setup the bass, range from 0 ~ (MAX_TREBLE_STEPS-1)
 * NOTE : dsp flow related
 */
void Adau1761Drv_SetTreble(uint32_t treble_level);

/* 
 * music detect on current source
 * NOTE : dsp flow related
 */
bool Adau1761Drv_MusicDetected(void);

/* 
 * bypass enable/disable
 * NOTE : dsp flow related
 */
void Adau1761Drv_BypassEnable(bool enable);

#ifdef HAS_SHAPE_EQ_SWITCH
/* shape EQ enable/disable */
void Adau1761Drv_EnableShapeEQ(bool enable);
#endif


void Adau1761Drv_TestMicLoop(bool enable);
#endif  // __ADAU1761_DRV_H__
