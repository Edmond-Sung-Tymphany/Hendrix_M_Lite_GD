#ifndef DSPDRV_V1_H
#define DSPDRV_V1_H
#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"

/* Line-Out jack status */
typedef enum
{
    LINEOUT_UNKNOWN,
    LINEOUT_UNPLUGGED,
    LINEOUT_PLUGGED
} eLineoutStatus;

/* the EQ controllers in the DSP algorithm */
typedef enum
{
    DSP_VOLUME_SETT_ID,  //Using this ID, user must update volume in setting database and then send the event
    DSP_BASS_SETT_ID,
    DSP_TREBLE_SETT_ID,
    DSP_EQ_CTRL_RGC,
    DSP_EQ_CTRL_USER_LP,
    DSP_EQ_CTRL_USER_HP,
    DSP_EQ_CTRL_TUNING,
    DSP_EQ_CTRL_PEQ1,
    DSP_EQ_CTRL_PEQ2,
    DSP_EQ_CTRL_PEQ3,
    DSP_PHASE_SETT_ID,
    DSP_POLARITY_SETT_ID,
    DSP_TUNNING_SETT_ID,
    DSP_DELAY_SETT_ID,
    DSP_PLAINEQ_SETT_ID,
    DSP_SETT_ID_MAX
}eDspSettId;

typedef struct
{
    void (*initSectionFunc)(void* me);         // function point to a specific initialization section
    uint16 delaytime;   // time duration to next intialization section in ms
}tDspInitSection;

CLASS(cDSPDrv)
    bool            isCreated:1;
    tDspInitSection const *pInitTable;
    uint8           sectionSize;
    uint8           initPhase;
    uint16          max_vol;
    uint16          default_vol;
    tI2CDevice      *pI2CConfig;
METHODS
END_CLASS


typedef enum
{
    DSP_BCLK_LRCLK_OUTPUT,
    DSP_BCLK_LRCLK_INPUT,
    DSP_BCLK_LRCLK_MAX
}eDspBclkLrclkDirection;

typedef enum
{
    AUDIOCTRL_DRIVER_INPUT_I2S_1,
    AUDIOCTRL_DRIVER_INPUT_I2S_2,
    AUDIOCTRL_DRIVER_INPUT_I2S_3,
    AUDIOCTRL_DRIVER_INPUT_I2S_4,
    AUDIOCTRL_DRIVER_INPUT_ANALOG_1,
    AUDIOCTRL_DRIVER_INPUT_ANALOG_2,
    AUDIOCTRL_DRIVER_INPUT_ANALOG_3,
    AUDIOCTRL_DRIVER_INPUT_ANALOG_1_2,
    AUDIOCTRL_DRIVER_INPUT_SPDIF_0,
    AUDIOCTRL_DRIVER_INPUT_SPDIF_1,
    AUDIOCTRL_DRIVER_INPUT_MAX
}eAudioCtrlDriverInput;

typedef enum
{
    DSP_POWER_SETTINGS_ON,
    DSP_POWER_SETTINGS_OFF
}eDspPowerSetting;

typedef enum
{
    DSP_MUX_CH1,
    DSP_MUX_CH2
}eDspMuxChannel;



/**
 * Mutes the DSP DAC output
 *
 * @param      void
 * @return     void
 */
void DSPDrv_MuteDACOut(cDSPDrv *me);

/**
 * Unmutes the DSP DAC output, by clearing the bit3 and bit2 of Page0 Reg64.
 *
 * @param      void
 * @return     void
 */
void DSPDrv_UnMuteDACOut(cDSPDrv *me);

/**
 * Mutes the Headphone output
 *
 * @param      void
 * @return     void
 */
void DSPDrv_MuteHPAOut(cDSPDrv *me);

/**
 * Un-Mutes the Headphone output
 *
 * @param      void
 * @return     void
 */
void DSPDrv_UnMuteHPAOut(cDSPDrv *me);

/**
 * Mutes the Lineout
 *
 * @param      void
 * @return     void
 */
void DSPDrv_MuteLineOut(cDSPDrv *me);

/**
 * Un-Mutes the Lineout
 *
 * @param      void
 * @return     void
 */
void DSPDrv_UnMuteLoOut(cDSPDrv *me);

/**
 * Mutes the amplifier output, it is referring to the amplifier outside the DSP.
 * This pin is not necessarily handled by DSP driver
 *
 * @param      void
 * @return     void
 */
void DSPDrv_MuteAmp(cDSPDrv *me);

/**
 * Un-Mutes the amplifier output, it is referring to the amplifier outside the DSP.
 * This pin is not necessarily handled by DSP driver
 *
 * @param      void
 * @return     void
 */
void DSPDrv_UnMuteAmp(cDSPDrv *me);

/**
 * Gets the status of the lineout jack status
 *
 * @param      void
 * @return     eLineoutStatus         the lineout jack is plugged or not
 */
eLineoutStatus DSPDrv_GetLineoutStatus(cDSPDrv *me);

/**
 * Gets the status of the aux-in jack status
 *
 * @param      void
 * @return     eAuxinStatus         the auxin jack is plugged or not
 */
bool DSPDrv_IsAuxin(cDSPDrv *me);

/**
 * Construct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv_Ctor(cDSPDrv* me);

/**
 * Destruct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv_Xtor(cDSPDrv* me);

/**
 * Set the DAC gain
 * @param      dB_onetenth - gain level in one tenth
 */
void DSPDrv_setDACgain(cDSPDrv *me, int dB_onetenth);

/**
 * Set the DAC mute
 * @param      bmute - mute or not
 */
void DSPDrv_DAC_Mute(cDSPDrv *me, bool bmute);


/**
 * Set DSP initialization function
 * @param  -    init section state
 * @return -    return the delay time following if the init is not over
 *              return zero if the init is over
 */
uint16 DSPDrv_Init(cDSPDrv* me);


/**
 * Select channel whether I2S or Analog
 * @param  -    channel
 */
void DSPDrv_set_Input(cDSPDrv *me, eAudioCtrlDriverInput input);

/**
 * Select channel on DSP mux which it a customer added on DSP flow
 * @param  -    channel
 */
int DSPDrv_setStereoMux(cDSPDrv *me, eDspMuxChannel ch);


#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
/**
 * Adjust gain for different battery level
 * reference on the coeffs in gain_coefs in DspDrv.config
 *
 * @param      level
 * @return     void
 */

void DSPDrv_SetGain(cDSPDrv *me, uint8 level);

#endif



/**
 * Sets the DSP volume, by writing the L/R channels PGA Gain registers. Please
 * reference on the volume table to check the real gain.
 *
 * @param      vol             The volume step that will be set
 * @return     void
 */
void DSPDrv_SetVol(cDSPDrv *me, uint8 vol);

/**
 * Detect whether there is a input present
 *
 * @param      
 * @return     bool    If the music is present either source depending on the DSP flow design
 */
bool DSPDrv_HasMusicStream(cDSPDrv *me);

void DSPDrv_SetAudio(cDSPDrv *me, eDspSettId dspSettId, BOOL enable);

#ifdef REDUCE_TOTAL_GAIN_WHILE_LOW_POWER
/**
 * Refresh volume when battery is low, reduce it by 6dB
 *
 * @param      uint8        target volume
 * @return     void
 */
void DSPDrv_SetVolForLowPower(cDSPDrv *me, uint8 vol);
#endif

#ifdef AUDIO_LIMITER_FOR_LOW_POWER
/**
 * Refresh DRCs when power is low
 *
 * @param      void
 * @return     void
 */
void DSPDrv_SetDrcForLowPower(cDSPDrv *me);

/**
 * Refresh DRCs for normal power
 *
 * @param      void
 * @return     void
 */
void DSPDrv_SetDrcForNormalPower(cDSPDrv *me);

/**
 * Refresh DRCs for adaptor mode
 *
 * @param      void
 * @return     void
 */
void DSPDrv_SetDrcForAdaptorMode(cDSPDrv *me);

#endif

bool DSPDrv_IsPCM(cDSPDrv *me);
#endif /* DSPDRV_V1_H */
