#ifndef DSPDRV_TAS5754_H
#define DSPDRV_TAS5754_H
#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "I2CDrv.h"

enum eEqTurningId
{
    BASS_SHELF_TURNING_ID,
    TREBLE_SHELF_TURNING_ID
};

CLASS(cDspTas5754)
    bool            isCreated:1;
    uint8           *initData;
    uint16          max_vol;
    uint16          default_vol;
    cI2CDrv         *i2cObj;  /* A i2c obj should be ctored before ctor dspDrv*/
METHODS

/**
 * Construct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DspTas5754_Ctor(cDspTas5754* me, cI2CDrv *pI2cObj);

/**
 * Destruct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DspTas5754_Xtor(cDspTas5754* me);


/**
 * Set DSP initialization function
 * @param  -    init section state
 * @return -    return the delay time following if the init is not over
 *              return zero if the init is over
 */
uint16 DspTas5754_Init(cDspTas5754 * me);

/**
 * Un-Mutes the Lineout
 *
 * @param      void
 * @return     void
 */
void DspTas5754_Mute(cDspTas5754 *me, eAudioMuteType muteType, BOOL muteEnable);

/**
 * Gets the status of the aux-in jack status
 *
 * @param      void
 * @return     eAuxinStatus         the auxin jack is plugged or not
 */
bool DspTas5754_IsAuxin(cDspTas5754 *me);

/**
 * Gets the status of the music streaming status
 *
 * @param      void
 * @return     bool         if there is music or not
 */
BOOL DspTas5754_HasMusicStream(cDspTas5754 *me);

/**
 * Select channel whether I2S or Analog
 * @param  -    channel
 */
void DspTas5754_Volume(cDspTas5754 *me, uint8 vol);

void DspTas5754_EqTurning(cDspTas5754 *me, uint8 id, void *data);
END_CLASS

#endif /* DSPDRV__V2_H */
