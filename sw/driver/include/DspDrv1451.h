#ifndef DSPDRV_V2_H
#define DSPDRV_V2_H
#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "I2CDrv.h"

typedef struct
{
    void (*initSectionFunc)(void* me);         // function point to a specific initialization section
    uint16 delaytime;   // time duration to next intialization section in ms
}tDspInitSection;

CLASS(cDSPDrv1451)
    bool            isCreated:1;
    tDspInitSection const *pInitTable;
    uint8           sectionSize;
    uint8           initPhase;
    uint16          max_vol;
    uint16          default_vol;
    cI2CDrv         *i2cObj;  /* A i2c obj should be ctored before ctor dspDrv*/
    uint8           deviceAddr;
    bool            i2cEnable; /* When DSP cable insert, disable I2C */
METHODS


typedef enum _tDspAsrc
{
    DSP_ASRC_MIN= 0,
    DSP_ASRC0= DSP_ASRC_MIN,
    DSP_ASRC1,
    DSP_ASRC2,
    DSP_ASRC3,
    DSP_ASRC4,
    DSP_ASRC5,
    DSP_ASRC6,
    DSP_ASRC7,
    DSP_ASRC_MAX= DSP_ASRC7,
}tDspAsrc;


//refer to PositionSoundMode_Position on GPB
typedef enum _tDspPosition
{
    DSP_POS_FREE= 1,
    DSP_POS_WALL= 2,
    DSP_POS_CORNER= 3
}tDspPosition; 


typedef enum _tDspChannel
{
    DSP_CH_NORMAL= 0,
    DSP_CH_LEFT_ONLY,
    DSP_CH_RIGHT_ONLY,
    DSP_CH_MAX
}tDspChannel;


#define DSP1451_LINEAR_GAIN_0DB   1.0
#define DSP1451_LINEAR_GAIN_N6DB  0.5



/**
 * Construct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv1451_Ctor(cDSPDrv1451* me, cI2CDrv *pI2cObj);

/**
 * Destruct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv1451_Xtor(cDSPDrv1451* me);


/**
 * Set DSP initialization function
 * @param  -    init section state
 * @return -    return the delay time following if the init is not over
 *              return zero if the init is over
 */
uint16 DSPDrv1451_Init(cDSPDrv1451 * me);

/**
 * Enable/Disalbe DSP I2C access
 */
void DSPDrv1451_I2cEnable(cDSPDrv1451 *me, bool i2CEnable);

/**
* Audio related setting intrance
* @param[in]    me             dsp object
* @param[in]    dspSettId      dsp setting ID
* @param[in]    enable         disable or enable the settings
*/
//void DSPDrv1451_SetAudio(cDSPDrv1451 *me, eAudioSettId dspSettId, BOOL enable);

/**
 * Un-Mutes the Lineout
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_Mute(cDSPDrv1451 *me, eAudioMuteType muteType, BOOL muteEnable);

/**
 * Gets the status of the aux-in jack status
 *
 * @param      void
 * @return     eAuxinStatus         the auxin jack is plugged or not
 */
bool DSPDrv1451_IsAuxin(cDSPDrv1451 *me);

/**
 * Gets the status of the music streaming status
 *
 * @param      void
 * @return     bool         if there is music or not
 */
BOOL DSPDrv1451_HasMusicStream(cDSPDrv1451 *me);

#ifdef HAS_AUXIN
/**
 * Detect whether auxin has music streaming.
 *
 * @param
 * @return     bool    If the music is present either source depending on the DSP flow design
 */
bool DSPDrv1451_AuxinHasMusicStream(cDSPDrv1451 *me);
#endif //#ifdef HAS_AUXIN

#ifdef HAS_SPDIF_IN
/**
 * Detect whether SPDIF in has music streaming.
 *
 * @param
 * @return     bool    If the music is present either source depending on the DSP flow design
 */
bool DSPDrv1451_SpdifInHasMusicStream(cDSPDrv1451 *me);
#endif //#ifdef HAS_SPDIF_IN

/**
 * Select channel whether I2S or Analog
 * @param  -    channel
 */
void DSPDrv1451_SetInputChannel(cDSPDrv1451 *me, eAudioChannel inputChannel);

#ifdef HAS_LINE_IN_MULTI_ROOM
/**
 * Select the line-in multi-room source
 * @param  -    channel
 */
void DSPDrv1451_SetLineInMultiRoomChannel(cDSPDrv1451 *me, eAudioChannel input);
#endif

#ifdef HAS_LINE_IN_SENSITIVITY_SETTING
/**
 * Set line-in sensitivity
 * @param  -    Enable
 */
void DSPDrv1451_SetLineInSensitivity(cDSPDrv1451 *me, eLineinSensitivity level);
#endif

/**
 * Enable pass-through or not
 * @param  -    passEnable
 */
void DSPDrv1451_SetPassthrough(cDSPDrv1451 *me, bool passEnable);

#ifdef HAS_AUDIO_MUTE_CHANNEL
/**
 * Mute Woofer or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteWoofer(cDSPDrv1451 *me, bool bIsMute);

/**
 * Mute MiddleA or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteMiddleA(cDSPDrv1451 *me, bool bIsMute);

/**
 * Mute MiddleB or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteMiddleB(cDSPDrv1451 *me, bool bIsMute);

/**
 * Mute Tweeter or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteTweeter(cDSPDrv1451 *me, bool bIsMute);
#endif


/**
 * Set volume
 * @param  -    vol
 */
void DSPDrv1451_SetVol(cDSPDrv1451 *me, uint8 vol);

float DSPDrv1451_GetDspVer(cDSPDrv1451 *me);
float DSPDrv1451_ReadAsrcConversionRate(cDSPDrv1451 *me, tDspAsrc asrc);
void DSPDrv1451_WriteDcValue32_0(cDSPDrv1451 *me, uint16 reg_addr, uint32 data);
void DSPDrv1451_WriteValue8_24(cDSPDrv1451 *me, uint16 reg_addr, float fValue);
uint32 DSPDrv1451_ReadDcValue32_0(cDSPDrv1451 *me, uint16 reg_addr);
float DSPDrv1451_ReadInputVolume(cDSPDrv1451 *me);
float DSPDrv1451_ReadOutputVolume(cDSPDrv1451 *me);
float DSPDrv1451_ReadValue8_24(cDSPDrv1451 *me, uint16 reg_addr);
void DSPDrv1451_SetSpdifAutoRestart(cDSPDrv1451 *me, bool enable);

//in config
void DSPDrv1451_SetCalibrateGain(cDSPDrv1451 *me, eAudioSettId audioSettId, float fGainDb);
float DSPDrv1451_ReadInputVolume2(cDSPDrv1451 *me);
float DSPDrv1451_ReadInputAuxinVolume(cDSPDrv1451 *me);
float DSPDrv1451_ReadInputAuxinVolume2(cDSPDrv1451 *me);
float DSPDrv1451_ReadWooferOutputVolume(cDSPDrv1451 *me);
float DSPDrv1451_ReadTweeterOutputVolume(cDSPDrv1451 *me);
float DSPDrv1451_ReadMiddleAOutputVolume(cDSPDrv1451 *me);
float DSPDrv1451_ReadMiddleBOutputVolume(cDSPDrv1451 *me);
void DSPDrv1451_SetChannel_L_R(cDSPDrv1451 *me, tDspChannel channel);
void DSPDrv1451_SetTreble(cDSPDrv1451 *me, uint32 value);
void DSPDrv1451_SetBase(cDSPDrv1451 *me, uint32 value);
void DSPDrv1451_SetLoudness(cDSPDrv1451 *me, bool enable);
void DSPDrv1451_SetPosition(cDSPDrv1451 *me, eSpeakerPosition position);

#ifdef HAS_DSP_DYNA_BOOST
uint32 DSPDrv1451_GetDynaBoostLevel(cDSPDrv1451 *me);
void DSPDrv1451_SetDcStatus(cDSPDrv1451 *me, bool dc_on);
#endif /* HAS_DSP_DYNA_BOOST */


END_CLASS

#endif /* DSPDRV__V2_H */

