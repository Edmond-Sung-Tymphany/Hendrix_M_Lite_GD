#include "trace.h"
#include "projBsp.h"
#include "AudioDrv.h"
#include "attachedDevices.h"
#include "AudioDrv.config"


/* GPIO object */
#ifdef HAS_AUDIO_IO_MONITOR
static cGpioDrv gpioAudio;
#endif
#ifdef HAS_SPDIF_SIGNAL_SENSE
static void AudioDrv_SpdifSignalSenseInit(cGpioDrv *pGpioAudio);
#endif

void AudioDrv_Ctor()
{
#ifdef HAS_AUDIO_IO_MONITOR
    tGPIODevice* pAudioGPIOConf;
    pAudioGPIOConf = (tGPIODevice*)getDevicebyId(AUDIO_DEV_ID,NULL);
    GpioDrv_Ctor(&gpioAudio,pAudioGPIOConf);
#ifdef HAS_SPDIF_SIGNAL_SENSE
    AudioDrv_SpdifSignalSenseInit(&gpioAudio);
#endif
#endif
}

void AudioDrv_ResetAmp(bool reset)
{
    if (reset)
    {
        AMP_RESET_ON;
    }
    else
    {
        AMP_RESET_OFF;
    }
}

void AudioDrv_Mute(bool mute)
{
    if (mute)
    {
        SDZ_PIN_OFF;
        AMP_MUTE;
        RCA_OUT_MUTE;
    }
    else
    {
        SDZ_PIN_ON;
        AMP_UNMUTE;
        RCA_OUT_UNMUTE;
    }
}

#ifdef AUDIO_I2S_CLK_CTRL
void AudioDrv_EnI2sClk(bool enable)
{
    if (enable)
    {
        I2S_CLK_EN_PIN_LOW; //low active
    }
    else
    {
        I2S_CLK_EN_PIN_HIGH;
    }
}
#endif

#ifdef HAS_AUDIO_IO_MONITOR
bool AudioDrv_IsLineInJackPluggedIn(void)
{
    return (IsAuxinPluggedIn(&gpioAudio));
}

bool AudioDrv_IsRcaInJackPluggedIn(void)
{
    return (IsRcaInPluggedIn(&gpioAudio));
}

bool AudioDrv_IsSpdifInJackPluggedIn(void)
{
    return (IsSpdif0InPluggedIn());
}
#endif

#ifdef HAS_SPDIF_SIGNAL_SENSE
static void AudioDrv_SpdifSignalSenseInit(cGpioDrv *pGpioAudio)
{
    uint8 i=0;
    uint8 j=0;
    for (i=0;i<NUM_OF_SPDIF_CHANNEL;i++)
    {
        SpdifObj[i].ringBuf.pAvgBuff= &SpdifObj[i].ringBuf.AvgBuff[0];
        SpdifObj[i].gpio = pGpioAudio;
        for(j=0; j<BUFF_SIZE; j++)
        {
            SpdifObj[i].ringBuf.AvgBuff[j] = SPDIF_SENSE_INVALID_LOWER;
        }
    }
}
/** \brief
*   check if the valueToCheck is within a range.
*   return bool
*/
static bool WithinRange(uint16 valueToCheck, uint16 lowerRange, uint16 UpperRange)
{
    return (valueToCheck>lowerRange && valueToCheck<UpperRange);
}

/** \brief Capture the SPDIF signal to determine if it has a music content
 *      This function should be called every 2ms other SPDIF_SENSE_VALID_COUNT_MIN and SPDIF_SENSE_VALID_COUNT_MAX need to be adjusted.
 *      return a captured averaged SPDIF pulse change count.
*/
static uint32 AudioDrv_CaptureOpticalData(tSpdifObj* SpdifObj)
{
    uint16 AvgData;
    int16 spdifLevel1 = GpioDrv_ReadBit(SpdifObj->gpio, SpdifObj->pin);
    ProjBsp_Delay25ns();   // This delay must be put here because it is time critical
    int16 spdifLevel2 = GpioDrv_ReadBit(SpdifObj->gpio, SpdifObj->pin);
	SpdifObj->digital_data += (spdifLevel1!=spdifLevel2);

    if (SpdifObj->sample_count++>=SPDIF_IN_SAMPLES_BUFFER_SIZE)
    {

        SpdifObj->sample_count=0;
        SpdifObj->isHasMusic = WithinRange(SpdifObj->digital_data, SPDIF_SENSE_VALID_COUNT_MIN, SPDIF_SENSE_VALID_COUNT_MAX);
        SpdifObj->isCableConnected = (SpdifObj->digital_data>SPDIF_SENSE_VALID_COUNT_MIN);
        SpdifObj->digital_data = 0;
    }
    return SpdifObj->digital_data;
}


uint16 AudioDrv_OpticalReading(void)
{
    uint8 i=0;
    for (i=0; i<ArraySize(SpdifObj);i++)
    {
        AudioDrv_CaptureOpticalData(&SpdifObj[i]);
    }
}

#endif

