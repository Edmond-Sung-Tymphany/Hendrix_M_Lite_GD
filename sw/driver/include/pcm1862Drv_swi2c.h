/**
* @file pcm1862Drv_swi2c.h
* @brief The devices attached to the product.
* @date 11-Oct-2017
* @copyright Tymphany Ltd.
*/

#ifndef PCM1862DRV_SWI2C_H
#define PCM1862DRV_SWI2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "SWi2c_Drv.h"

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

CLASS(cPcm1862Drv)
uint8       deviceAddr;
cSWi2cDrv_t     *pAudioAdcI2cObj;
bool        isCreated;
METHODS

void pcm1862Drv_swi2c_Ctor(cPcm1862Drv * me, cSWi2cDrv_t *pI2cObj);
void pcm1862Drv_swi2c_Xtor(cPcm1862Drv * me);
void pcm1862Drv_swi2c_Init(cPcm1862Drv * me);
void pcm1862Drv_swi2c_SetInput(cPcm1862Drv * me, ePcm1862AnalogInput input);
void pcm1862Drv_swi2c_SetPGA(cPcm1862Drv * me, ePcm1862Gga programGain);
void pcm1862Drv_swi2c_SetMixerGain(cPcm1862Drv * me, uint16 channel, uint8 programGain);
void pcm1862Drv_swi2c_EnableAGC(cPcm1862Drv * me, bool enable);
void pcm1862Drv_swi2c_EnableMute(cPcm1862Drv * me, bool enable);
void pcm1862Drv_swi2c_DumpReg(cPcm1862Drv *me);
uint8 pcm1862Drv_swi2c_GetMode(cPcm1862Drv * me);
void pcm1862Drv_swi2c_Dsp2MemMap(cPcm1862Drv * me, uint8 mode);
void pcm1862Drv_swi2c_SetEnergySenseMask(cPcm1862Drv * me, uint8 mask);

void pcm1862Drv_swi2c_Mute_LeftChannel(cPcm1862Drv * me, bool enable);
void pcm1862Drv_swi2c_Mute_RightChannel(cPcm1862Drv * me, bool enable);
void pcm1862Drv_swi2c_SetInput_L(cPcm1862Drv * me, ePcm1862AnalogInput inputChannel);
void pcm1862Drv_swi2c_SetInput_R(cPcm1862Drv * me, ePcm1862AnalogInput inputChannel);


#ifdef __cplusplus
}
#endif


#endif //#ifndef PCM1862DRV_SWI2C_H
