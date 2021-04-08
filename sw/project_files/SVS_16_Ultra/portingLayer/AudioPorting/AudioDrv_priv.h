/**
 * @file        AudioDrv_priv.h
 * @brief       This file implementes the middle layer of the audio service 
 * @author      Bob.Xu 
 * @date        2015-06-15
 * @copyright   Tymphany Ltd.
 */
#ifndef AUDIODRV_PRIV_H
#define AUDIODRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AudioDrv.h"
#define AMP_MUTE_IO                (gpioAmpMuteDrv.gpioConfig->pGPIOPinSet[0].gpioId)
#define AMP_MUTE                    MainApp_AmpMute(TRUE)
#define AMP_UNMUTE                  MainApp_AmpMute(FALSE)
#define TEMPERATURE_ADC_PIN         ADC_PIN5
#define MUSIC_STATUS_SAMPLES        100
#define HAS_MUSIC_THRESHOLD         60
// temperature and limiter map struct
typedef struct tTempLimiterCtrl
{
    uint16  adcValueMin;
    uint16  adcValueMax;
    int16   limiterLevel;
}tTempLimiterCtrl;
  
/* private functions / data */
static void MainApp_AmpMute(BOOL muteEnable);
static void AMP_TempMonitor();
#ifdef __cplusplus
}
#endif

#endif /* AUDIODRV_PRIV_H */