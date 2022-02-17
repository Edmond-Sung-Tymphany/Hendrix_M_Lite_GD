/**
* @file AmpDrvRT9114B.h
* @brief The devices attached to the product.
* @author Zorro Lu
* @date 8-March-2019
* @copyright Tymphany Ltd.
*/

#ifndef AMP_DRV_RT9114B_Hs
#define AMP_DRV_RT9114B_H

#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"

/* //Nick--
typedef enum _eAdcPwmRate {
    ADC_PWM_RATE_6LRCK  = 0, //REG[0x06].bit[6:4] = 000
    ADC_PWM_RATE_8LRCK  = 1, //REG[0x06].bit[6:4] = 001
    ADC_PWM_RATE_10LRCK = 2, //REG[0x06].bit[6:4] = 010
    ADC_PWM_RATE_12LRCK = 3, //REG[0x06].bit[6:4] = 011
    ADC_PWM_RATE_14LRCK = 4, //REG[0x06].bit[6:4] = 100
    ADC_PWM_RATE_16LRCK = 5, //REG[0x06].bit[6:4] = 101
    ADC_PWM_RATE_20LRCK = 6, //REG[0x06].bit[6:4] = 110
    ADC_PWM_RATE_24LRCK = 7, //REG[0x06].bit[6:4] = 111
    ADC_PWM_RATE_MAX    = 8
}eAdcPwmRate;
/*
/* //Nick--
typedef enum _eAnalogGain
{
    ANALOG_GAIN_19_2_DBV = 0, //19.2dBV
    ANALOG_GAIN_22_6_DBV = 1, //22.6dBV
    ANALOG_GAIN_25_DBV   = 2, //25dBV
    ANALOG_GAIN_MAX      = 3
}eAnalogGain;
*/
/* //Nick--
typedef enum _ePbtlChSel
{
    PBTL_CH_SEL_L   = 0, //select left channel for PBTL mode
    PBTL_CH_SEL_R   = 1, //select right channel for PBTL mode
    PBTL_CH_SEL_MAX = 2
}ePbtlChSel;
*/

CLASS(cAudioAmpDrv)
    uint8       deviceI2cAddr;
    cI2CDrv     * pAudioAmpI2cObj;
    bool        isCreated;
    bool        i2cEnable;
METHODS

void AudioAmpDrv_Ctor(cAudioAmpDrv * me, cI2CDrv *pI2cObj);
void AudioAmpDrv_Xtor(cAudioAmpDrv * me);
void AudioAmpDrv_setSoftMute(cAudioAmpDrv * me, bool enable);
void AudioAmpDrv_setSoftMuteLeftChannel(cAudioAmpDrv * me, bool enable);
void AudioAmpDrv_setSoftMuteRightChannel(cAudioAmpDrv * me, bool enable);
void AudioAmpDrv_I2cEnable(cAudioAmpDrv *me, bool i2cEnable);

#endif //#ifndef AMP_DRV_TAS5760_H
