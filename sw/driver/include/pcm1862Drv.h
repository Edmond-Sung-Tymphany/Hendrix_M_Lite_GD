/**
* @file pcm1862Drv.h
* @brief The devices attached to the product.
* @date 26-June-2015
* @copyright Tymphany Ltd.
*/

#ifndef PCM1862_DRV_H
#define PCM1862_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "I2CDrv.h"

typedef enum
{
    PCM1862_ANALOG_NO_INPUT  = 0x40,
    PCM1862_ANALOG_INPUT1    = 0x41,
    PCM1862_ANALOG_INPUT2    = 0x42,
    PCM1862_ANALOG_INPUT3    = 0x44,
    PCM1862_ANALOG_INPUT4    = 0x48,
    PCM1862_ANALOG_DIFF_INPUT1 = 0x50, // Page[0].Reg[6]:{VIN1P,VIN1M}[DIFF], Page[0].Reg[7]:{VIN2P,VIN2M}[DIFF]
    PCM1862_ANALOG_DIFF_INPUT2 = 0x60, // Page[0].Reg[6]:{VIN4P,VIN4M}[DIFF], Page[0].Reg[7]:{VIN3P,VIN3M}[DIFF]
} ePcm1862AnalogInput;


/*
 PGA Value Channel 1 Left/Right
 Input parameter of AdcDrv_pcm1862_SetPGA()
 Global Channel gain for ADC1L/ADC1R. (Analog + Digital). Analog gain only, if manual gain mapping is enabled.
(0x19)
  Default value: 00000000
  Specify 2s complement value with 7.1 format.
  1110100_0: -12.0dB (Min)
  :
  1111111_0: -1.0dB
  1111111_1: 0.5dB
  0000000_0: 0.0dB
  0000000_1: +0.5dB
  0000001_0: +1.0dB
  :
  0001100_0: +12.0dB
  :
  0010100_0: +20.0dB
  :
  0100000_0: +32.0dB
  :
  0101000_0: +40.0dB (Max)
 */
typedef enum
{
    PCM1862_PGA_MIN = 0xe8, //-12.0dB
    PCM1862_PGA_0dB = 0x0,
    PCM1862_PGA_4dB = 0x08,
    PCM1862_PGA_6dB = 0x0c,
    PCM1862_PGA_9dB = 0x12,
    PCM1862_PGA_MAX = 0x50, //40.0dB
} ePcm1862Gga;


typedef enum
{
    /*ADC MIXER GAIN mapping*/
    PCM1862_MIX1_CH1R            =           0x01,
    PCM1862_MIX1_CH1L            =           0x00,
    PCM1862_MIX1_CH2R            =           0x03,
    PCM1862_MIX1_CH2L            =           0x02,

    PCM1862_MIX2_CH1R            =           0x07,
    PCM1862_MIX2_CH1L            =           0x06,
    PCM1862_MIX2_CH2R            =           0x09,
    PCM1862_MIX2_CH2L            =           0x08
} eMixerChannelMapping;

CLASS(cAdcDrv_pcm1862)
uint8       deviceAddr;
cI2CDrv     *pAudioAdcI2cObj;
bool        isCreated;
METHODS

void AdcDrv_pcm1862_Ctor(cAdcDrv_pcm1862 * me, cI2CDrv *pI2cObj);
void AdcDrv_pcm1862_Xtor(cAdcDrv_pcm1862 * me);
void AdcDrv_pcm1862_Init(cAdcDrv_pcm1862 * me);
void AdcDrv_pcm1862_SetInput(cAdcDrv_pcm1862 * me, ePcm1862AnalogInput input);
void AdcDrv_pcm1862_SetPGA(cAdcDrv_pcm1862 * me, ePcm1862Gga programGain);
void AdcDrv_pcm1862_SetMixerGain(cAdcDrv_pcm1862 * me, uint16 channel, uint8 programGain);
void AdcDrv_pcm1862_Mute(cAdcDrv_pcm1862 * me, bool enable);
void AdcDrv_pcm1862_dumpReg(cAdcDrv_pcm1862 *me);
void AdcDrv_pcm1862_enableAGC(cAdcDrv_pcm1862 * me, bool enable);
void AdcDrv_pcm1862_mute_LeftChannel(cAdcDrv_pcm1862 * me, bool enable);
void AdcDrv_pcm1862_mute_RightChannel(cAdcDrv_pcm1862 * me, bool enable);
void AdcDrv_pcm1862_enableMute(cAdcDrv_pcm1862 * me, bool enable);
void AdcDrv_pcm1862_SetInput_L(cAdcDrv_pcm1862 * me, ePcm1862AnalogInput inputChannel);
void AdcDrv_pcm1862_SetInput_R(cAdcDrv_pcm1862 * me, ePcm1862AnalogInput inputChannel);

#ifdef __cplusplus
}
#endif


#endif //#ifndef PCM1862_DRV_H
