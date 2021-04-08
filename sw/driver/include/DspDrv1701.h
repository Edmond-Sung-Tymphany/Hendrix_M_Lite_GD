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

CLASS(cDSPDrv1701)
    bool            isCreated:1;
    tDspInitSection const *pInitTable;
    uint8           sectionSize;
    uint8           initPhase;
    uint16          max_vol;
    uint16          default_vol;
    cI2CDrv         *i2cObj;  /* A i2c obj should be ctored before ctor dspDrv*/
    uint8           deviceAddr;
METHODS

/**
 * Construct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv1701_Ctor(cDSPDrv1701* me, cI2CDrv *pI2cObj);

/**
 * Destruct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv1701_Xtor(cDSPDrv1701* me);


/**
 * Set DSP initialization function
 * @param  -    init section state
 * @return -    return the delay time following if the init is not over
 *              return zero if the init is over
 */
uint16 DSPDrv1701_Init(cDSPDrv1701 * me);

/**
* Audio related setting intrance
* @param[in]    me             dsp object
* @param[in]    dspSettId      dsp setting ID
* @param[in]    enable         disable or enable the settings
*/
void DSPDrv1701_SetAudio(cDSPDrv1701 *me, eAudioSettId dspSettId, BOOL enable);

/**
 * Un-Mutes the Lineout
 *
 * @param      void
 * @return     void
 */
void DSPDrv1701_Mute(cDSPDrv1701 *me, eAudioMuteType muteType, BOOL muteEnable);

/**
 * Gets the status of the aux-in jack status
 *
 * @param      void
 * @return     eAuxinStatus         the auxin jack is plugged or not
 */
bool DSPDrv1701_IsAuxin(cDSPDrv1701 *me);

/**
 * Gets the status of the music streaming status
 *
 * @param      void
 * @return     bool         if there is music or not
 */
BOOL DSPDrv1701_HasMusicStream(cDSPDrv1701 *me);

/**
 * Select channel whether I2S or Analog
 * @param  -    channel
 */
void DSPDrv1701_SetInputChannel(cDSPDrv1701 *me, eAudioChannel inputChannel);

END_CLASS

#endif /* DSPDRV__V2_H */
