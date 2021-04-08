#ifndef __PCM9211_DRV_H__
#define __PCM9211_DRV_H__

/* reg:0x34, DIR input biphase source select, coaxial amplifier control */
#define PCM9211_REG_DIR_SOURCE      0x34
    #define RX0_AMP_ENABLE      0x00
    #define RX0_AMP_DISABLE     0x80
    #define RX1_AMP_ENABLE      0x00
    #define RX1_AMP_DISABLE     0x40
    #define DIR_SOURCE_RX0      0
    #define DIR_SOURCE_RX1      1
    #define DIR_SOURCE_RX2      2
    #define DIR_SOURCE_RX3      3
    #define DIR_SOURCE_RX4      4
    #define DIR_SOURCE_RX5      5
    #define DIR_SOURCE_RX6      6
    #define DIR_SOURCE_RX7      7
    #define DIR_SOURCE_RX8      8
    #define DIR_SOURCE_RX9      9
    #define DIR_SOURCE_RX10     10
    #define DIR_SOURCE_RX11     11
    #define DIR_SOURCE_TXOUT    15

/*
 1111 1111 : +20.0dB
 1111 1110 : +19.5dB
 ...
 1101 0111 : 0dB
 ...
 0000 1111 : -100dB
 others : mute 
 */
#define PCM9211_REG_ADC_L_GAIN        0x46
#define PCM9211_REG_ADC_R_GAIN        0x47
    #define ADC_GAIN_0dB        0xd7

/* main output select */
#define PCM9211_REG_MAIN_OUTPUT       0x6b
    #define MAIN_OUTPUT_SOURCE_AUTO        0x00    // DIR/ADC auto switch
    #define MAIN_OUTPUT_SOURCE_DIR         0x11
    #define MAIN_OUTPUT_SOURCE_ADC         0x22
    #define MAIN_OUTPUT_SOURCE_AUXIN0      0x33
    #define MAIN_OUTPUT_SOURCE_AUXIN1      0x44
    #define MAIN_OUTPUT_SOURCE_AUXIN2      0x55

#define PCM9211_REG_SAMPLE_RATE     0x38
    #define SAMPLE_RATE_OutOfRange      0x00
    #define SAMPLE_RATE_8_KHz           0x01
    #define SAMPLE_RATE_11P025_KHz      0x02
    #define SAMPLE_RATE_12_KHz          0x03
    #define SAMPLE_RATE_16_KHz          0x04
    #define SAMPLE_RATE_22P05_KHz       0x05
    #define SAMPLE_RATE_24_KHz          0x06
    #define SAMPLE_RATE_32_KHz          0x07
    #define SAMPLE_RATE_44P1_KHz        0x08
    #define SAMPLE_RATE_48_KHz          0x09
    #define SAMPLE_RATE_64_KHz          0x0a
    #define SAMPLE_RATE_88P2_KHz        0x0b
    #define SAMPLE_RATE_96_KHz          0x0c
    #define SAMPLE_RATE_128_KHz         0x0d
    #define SAMPLE_RATE_176P4_KHz       0x0e
    #define SAMPLE_RATE_192_KHz         0x0f
    #define SAMPLE_RATE_NOT_READY       0xfe

typedef struct tagPcm9211Drv
{
    bool        isReady;
    bool        i2cEnable;
    eDeviceID   deviceID;
    eDeviceType deviceType;
    cI2CDrv     i2cDrv;
}stPcm9211Drv_t;

/**
 * Construct the Pcm9211 driver instance.
 * @return : none
 */
void Pcm9211Drv_Ctor(void);

/**
 * Exit & clean up the driver.
 */
void Pcm9211Drv_Xtor(void);

/**
 * Enable/Disalbe the I2C bus access
 */
void Pcm9211Drv_I2cEnable(bool enable);

/**
 * brief : setup the ADC gain, range : mute/-100dB ~ +20dB
 * @param - gain : adc gain setting
 * @return : none
 */
void Pcm9211_AdcGain(int8_t gain);

/**
 * brief : select the DIR0..11 source
 * @param - DIRx : RX port
 * @return : none
 */
void Pcm9211_DirSourceSelect(uint8_t DIRx);

/**
 * brief : get the sample rate of input audio
 * @param - none
 * @return : sample rate value:SAMPLE_RATE_xxxxx
 */
uint8_t Pcm9211_GetSampleRate(void);

/**
 * brief : main output select, DIR, ADC, RX4~7/AUXIN0, MPIO_C:AUXIN1, MPIO_B:AUXIN2
 * @param - setting -> refer to pcm9211_drv.h
 * @return : none
 */
void Pcm9211_MainOutputSetting(uint8_t setting);

/**
 * brief : Pcm9211 initial, write the necessary config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void Pcm9211_Init(void);


#endif  // __PCM9211_DRV_H__

