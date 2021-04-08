/**
* @file AudioAdcDrv.h
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 26-Mar-2015
* @copyright Tymphany Ltd.
*/

#ifndef AUDIO_ADC_DRV_H
#define AUDIO_ADC_DRV_H

#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"

/*
 * The below info is from PCM1862 spec: Page 0 / Register 6/7 (Hex 0x06/0x07)
 * ADC Input Channel Select (ADC1L/R)
    Default value: 000001
    00_0000: No Select
    00_0001: VINL1[SE] (Default)
    00_0010: VINL2[SE]
    00_0011: VINL2[SE] + VINL1[SE]
    00_0100: VINL3[SE]
    00_0101: VINL3[SE] + VINL1[SE]
    00_0110: VINL3[SE] + VINL2[SE]
    00_0111: VINL3[SE] + VINL2[SE] + VINL1[SE]
    00_1000: VINL4[SE]
    00_1001: VINL4[SE] + VINL1[SE]
    00_1010: VINL4[SE] + VINL2[SE]
    00_1011: VINL4[SE] + VINL2[SE] + VINL1[SE]
    00_1100: VINL4[SE] + VINL3[SE]
    00_1101: VINL4[SE] + VINL3[SE] + VINL1[SE]
    00_1110: VINL4[SE] + VINL3[SE] + VINL2[SE]
    00_1111: VINL4[SE] + VINL3[SE] + VINL2[SE] + VINL1[SE]
    01_0000: {VIN1P, VIN1M}[DIFF]
    10_0000: {VIN4P, VIN4M}[DIFF]
    11_0000: {VIN1P, VIN1M}[DIFF] + {VIN4P, VIN4M}[DIFF]
 */
typedef enum
{
    AUDIO_ADC_ANALOG_NO_INPUT  = 0x00,
    AUDIO_ADC_ANALOG_INPUT1    = 0x01,
    AUDIO_ADC_ANALOG_INPUT2    = 0x02,
    AUDIO_ADC_ANALOG_INPUT3    = 0x04,
    AUDIO_ADC_ANALOG_INPUT4    = 0x08,
    AUDIO_ADC_ANALOG_DIFF_INPUT1 = 0x50, // Page[0].Reg[6]:{VIN1P,VIN1M}[DIFF], Page[0].Reg[7]:{VIN2P,VIN2M}[DIFF]
    AUDIO_ADC_ANALOG_DIFF_INPUT2 = 0x60, // Page[0].Reg[6]:{VIN4P,VIN4M}[DIFF], Page[0].Reg[7]:{VIN3P,VIN3M}[DIFF]
}eAudioAdcAnalogInput;


#define AUDIO_ADC_ANALOG_INPUT1_MASK        (0xfc)
#define AUDIO_ADC_ANALOG_INPUT2_MASK        (0xf3)
#define AUDIO_ADC_ANALOG_DIFF_INPUT3_MASK   (0x0f)


CLASS(cAudioAdcDrv)
    uint8       deviceI2cAddr;
    tI2CDevice  *pI2CConfig;
    cI2CDrv     * pAudioAdcI2c;
    bool        isCreated;
METHODS

void AudioAdcDrv_Ctor(cAudioAdcDrv * me);
void AudioAdcDrv_Xtor(cAudioAdcDrv * me);
void AudioAdcDrv_setInput(cAudioAdcDrv * me, uint8 input);
void AudioAdcDrv_setPGA(cAudioAdcDrv * me, uint8 pgaValue);
void AudioAdcDrv_enableAGC(cAudioAdcDrv * me, bool enable);
void AudioAdcDrv_enableMute(cAudioAdcDrv * me, bool enable);
void AudioAdcDrv_SwitchToActive(cAudioAdcDrv * me, bool enable);
uint8 AudioAdcDrv_GetMode(cAudioAdcDrv * me);

#ifdef HAS_ENERGYSENSE_MODE
void AudioAdcDrv_SetEnergySenseMask(cAudioAdcDrv * me, uint8 mask);
void AudioAdcDrv_EnergySenseDSPMemMapCoefs(cAudioAdcDrv * me, bool bIsTurnOnSigDetection);
#endif


#endif //#ifndef AUDIO_ADC_DRV_H