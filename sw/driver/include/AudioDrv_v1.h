#ifndef AMPDRV_H
#define AMPDRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "GpioDrv.h"
#include "AdcDrv.h"

typedef enum
{
    SPDIF_CHANNEL_0,
    SPDIF_CHANNEL_1,
    END_SPDIF_CHANNEL
}eSpdifChannel;

typedef enum
{
    ANALOG_CHANNEL_0,
    ANALOG_CHANNEL_1,
    END_ANALOG_CHANNEL
}eAnalogChannel;

#define BUFF_SIZE       5

typedef struct
{
    uint16 AvgBuff[BUFF_SIZE];
    uint16* pAvgBuff;
}tRingBuf;

typedef struct
{
    bool old_bit:1;
    bool isHasMusic:1;
    bool isCableConnected;
    uint16 digital_data;
    uint16 sample_count;
    tRingBuf ringBuf;
    //uint16 AvgBuff[BUFF_SIZE];
    eGPIOId pin;
    eSpdifChannel channel;
    //uint16* pAvgBuff;
    cGpioDrv* gpio;
}tSpdifObj;

typedef struct
{
    bool isHasMusic:1;
    uint16 rawData;
    uint16 sample_count;
    tRingBuf ringBuf;
    //uint16 AvgBuff[BUFF_SIZE];
    //uint16* pAvgBuff;
    eAdcPin pin;
    eAnalogChannel channel;
    cADCDrv* adc;
}tAnalogObj;

void AudioDrv_Ctor();

/**
 * Function to reset the amplifier.
 * @param     reset on or reset off
 */
void AudioDrv_ResetAmp(bool reset);

/**
 * Function to mute the amplifier.
 * @param     mute or unmute
 */
void AudioDrv_Mute(bool mute);

#ifdef AUDIO_I2S_CLK_CTRL
/**
 * Function to enable/disable I2S clock.
 * @param     enable or disable
 */
void AudioDrv_EnI2sClk(bool enable);
#endif
/**
 * Function to detect the line-in jack state.
 * @param     
 * @return line-in jack state 
 */
bool AudioDrv_IsLineInJackPluggedIn(void);

#ifdef HAS_AUDIO_IO_MONITOR
bool AudioDrv_IsRcaInJackPluggedIn(void);

bool AudioDrv_IsSpdifInJackPluggedIn(void);
bool AudioDrv_IsSpdif1InJackPluggedIn(void);
bool AudioDrv_IsRJ45InJackPluggedIn(void);

#endif


#ifdef MUTIPLE_SOURCE_MUSIC_DETECTION
bool AudioDrv_IsLineInMusicDetected(void);
bool AudioDrv_IsSpdif0InMusicDetected(void);
bool AudioDrv_IsSpdif1MusicDetected(void);
bool AudioDrv_IsRj45_MusicDetected(void);


#endif

uint16 AudioDrv_OpticalReading(void);
uint16 AudioDrv_AnalogReading(void);
void AudioDrv_Spdif_SignalSense_Init();


#endif /* AMPDRV_H */
