#ifndef __CODEC_CS42528_H__
#define __CODEC_CS42528_H__

// get the digital-silence/AES-format/RCVR-CLK status from this register
#define CODEC_RECV_STATUS_REG          0x08
// 
#define CODEC_RECV_CHANNEL_STATUS_REG  0x25

typedef enum tagSAISource
{
    SAI_SOURCE_ADC=0,
    SAI_SOURCE_RX0,
    SAI_SOURCE_RX1,
    SAI_SOURCE_RX2,
    SAI_SOURCE_RX3,
    SAI_SOURCE_RX4,
    SAI_SOURCE_RX5,
    SAI_SOURCE_RX6,
    SAI_SOURCE_RX7,
    SAI_SOURCE_A2B,     // dummy status for A2B slave
    SAI_SOURCE_MAX
}SAISource_t;


typedef struct tagCs42528Drv
{
    bool        isReady;
    bool        i2cEnable;
    cI2CDrv     i2cDrv;
}CS42528Drv_t;

/**
 * Construct the CS42528 driver instance.
 * @return : none
 */
void CS42528Drv_Ctor(void);

/**
 * Exit & clean up the driver.
 */
void CS42528Drv_Xtor(void);

/**
 * Enable/Disalbe the I2C bus access
 */
void CS42528Drv_I2cEnable(bool enable);

/**
 * brief : setup the ADC gain, range : mute/-100dB ~ +20dB
 * @param - gain : adc gain setting
 * @return : none
 */
void CS42528_AdcGain(int8_t gain);

/**
 * brief : SAI output from SPDIF or ADC
 * @param - SAI source 
 * @return : none
 */
void CS42528Drv_SAIOutputSelect(SAISource_t source);

/**
 * brief : CS42528 initial, write the necessary config register data to the chip.
 * @param - instance of the driver
 * @return : none
 */
void CS42528Drv_Init(void);

/**
 * brief : CS42528 mute/unmute the DAC output
 * @param enable / disable
 * @return : none
 */
void CS42528Drv_DacMute(bool enable);


#endif  // __CODEC_CS42528_H__

