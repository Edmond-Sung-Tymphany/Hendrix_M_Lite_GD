#ifndef __SSM3582_DRV_H__
#define __SSM3582_DRV_H__

typedef enum tagSSM3582Reg
{
    SSM3582_REG_START,
        
    /* 0x00 */
    SSM3582_REG_VENDOR_ID = SSM3582_REG_START,
    SSM3582_REG_DEVICE_ID1,
    SSM3582_REG_DEVICE_ID2,
    SSM3582_REG_REVISION,
    
    SSM3582_REG_CONFIG_START,
    
    SSM3582_REG_POWER_CTRL=SSM3582_REG_CONFIG_START,
    SSM3582_REG_AMP_DAC_CTRL,
    SSM3582_REG_DAC_CTRL,
    SSM3582_REG_VOL_LEFT_CTRL,

    /* 0x08 */
    SSM3582_REG_VOL_RIGHT_CTRL,
    SSM3582_REG_SAI_CTRL1,
    SSM3582_REG_SAI_CTRL2,
    SSM3582_REG_SLOT_LEFT_CTRL,
    SSM3582_REG_SLOT_RIGHT_CTRL,
    SSM3582_REG_RESERVE1,
    SSM3582_REG_LIM_LEFT_CTRL1,
    SSM3582_REG_LIM_LEFT_CTRL2,

    /* 0x10 */
    SSM3582_REG_LIM_LEFT_CTRL3,
    SSM3582_REG_LIM_RIGHT_CTRL1,
    SSM3582_REG_LIM_RIGHT_CTRL2,
    SSM3582_REG_LIM_RIGHT_CTRL3,
    SSM3582_REG_CLIP_LEFT_CTRL,
    SSM3582_REG_CLIP_RIGHT_CTRL,
    SSM3582_REG_FAULT_CTRL1,
    SSM3582_REG_FAULT_CTRL2,

    SSM3582_REG_CONFIG_END, 
    
    /* 0x18 */
    SSM3582_REG_STATUS_START=0x18,
    SSM3582_REG_STATUS1=SSM3582_REG_STATUS_START,
    SSM3582_REG_STATUS2,
    SSM3582_REG_VBAT,
    SSM3582_REG_TEMP,
    SSM3582_REG_STATUS_END,
    SSM3582_REG_SOFT_RESET=0x1c,
    SSM3582_REG_END
}eSSM3582Reg_t;

typedef struct tagSsm3582Drv {
    bool        isReady;
    uint8_t     *p_regMap;
    eDeviceID   deviceID;
    eDeviceType deviceType;
    cSWi2cDrv_t     swi2cDrv;
}stSsm3582Drv_t;

/* POWER_CTRL setting */
// auto power down control
#define SSM3582_APWDN_ENABLE        0x80
#define SSM3582_APWDN_DISABLE       0x00
// temperature sensor on/off
#define SSM3582_TEMP_SENSOR_ON      0x00
#define SSM3582_TEMP_SENSOR_OFF     0x20
// mono mode selection
#define SSM3582_OUTPUT_STEREO       0x00
#define SSM3582_OUTPUT_MONO         0x10
// right channel power down
#define SSM3582_R_CH_POWER_ON       0x00
#define SSM3582_R_CH_POWER_OFF      0x08
// left channel power down
#define SSM3582_L_CH_POWER_ON       0x00
#define SSM3582_L_CH_POWER_OFF      0x04
// software power down
#define SSM3582_NORMAL_OPERATION    0x00
#define SSM3582_SW_POWER_DOWN       0x01
#define SSM3582_POWER_ON_MASK       0xfe
#define SSM3582_POWER_ON_ENABLE     (SSM3582_R_CH_POWER_ON|SSM3582_L_CH_POWER_ON|SSM3582_NORMAL_OPERATION)

/* AMP_DAC_CTRL setting */
// dac low power mode
#define SSM3582_DAC_LPM_ON          0x80
#define SSM3582_DAC_LPM_OFF         0x00
// right channel dac output polarity
#define SSM3582_DAC_POL_R_NORMAL    0x00           
#define SSM3582_DAC_POL_R_INVERT    0x20           
// left channel dac output polarity
#define SSM3582_DAC_POL_L_NORMAL    0x00           
#define SSM3582_DAC_POL_L_INVERT    0x10
#define SSM3582_DAC_POL_NORMAL      (SSM3582_DAC_POL_L_NORMAL|SSM3582_DAC_POL_R_NORMAL)
// EDGE rate control
#define SSM3582_EDGE_NORMAL         0x00
#define SSM3582_EDGE_LOW_EMI        0x08
// analog gain 
#define SSM3582_ANA_GAIN_13dB       0x00
#define SSM3582_ANA_GAIN_16dB       0x01
#define SSM3582_ANA_GAIN_19dB       0x02
#define SSM3582_ANA_GAIN_21dB       0x03

/* DAC_CTRL setting */
#define SSM3582_VOL_RAMPING_ON      0x00
#define SSM3582_VOL_RAMPING_OFF     0x80
// dac mute L/R channel
#define SSM3582_DAC_MUTE_MASK       0x9f
#define SSM3582_DAC_MUTE_R          0x40
#define SSM3582_DAC_UNMUTE_R        0x00
#define SSM3582_DAC_MUTE_L          0x20
#define SSM3582_DAC_UNMUTE_L        0x00
#define SSM3582_DAC_UNMUTE          (SSM3582_DAC_UNMUTE_L|SSM3582_DAC_UNMUTE_R)
#define SSM3582_DAC_MUTE            (SSM3582_DAC_MUTE_L|SSM3582_DAC_MUTE_R)
// dac high pass filter
#define SSM3582_DAC_HPF_ON          0x10
#define SSM3582_DAC_HPF_OFF         0x00
// dac sample rate select
#define SSM3582_DAC_FS_MASK         0xf8
#define SSM3582_DAC_FS_8_12K        0x00
#define SSM3582_DAC_FS_16_24K       0x01
#define SSM3582_DAC_FS_32_48K       0x02
#define SSM3582_DAC_FS_64_96K       0x03
#define SSM3582_DAC_FS_128_192K     0x04
#define SSM3582_DAC_FS_48_72K       0x05

/* volume control setting */
/* step = 3/8dB = 0.375dB, 0x40=0dB
 0x00 : +24dB
 0x01 : +23.625dB
 ...
 0x3f : +0.375dB
 0x40 : 0dB
 0x41 : -0.375dB
 ...
 0xfe : -71.25dB
 0xff : mute
 */
#define SSM3582_DAC_VOL_0dB         0x40

/* SAI_CTRL1 setting */
#define SSM3582_SAI_I2S                     0x10
#define SSM3582_SAI_I2S_WOOF_LEFT           0x14
#define SSM3582_SAI_I2S_WOOF_CENTER         0x14
#define SSM3582_SAI_TDM             0x11

/**
 * Construct the Ssm3582 driver instance.
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_Ctor(stSsm3582Drv_t * me);

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void Ssm3582Drv_Xtor(stSsm3582Drv_t * me);

/**
 * brief : soft reset the SSM3582.
 * @param me - instance of the driver
 * @param reset - 1:reset, 0: normal operation
 * @return : none
 */
void Ssm3582Drv_SoftReset(stSsm3582Drv_t * me, bool reset);

/**
 * brief : power on/off the SSM3582.
 * @param me - instance of the driver
 * @param reset - 1:power on, 0:power off
 * @return : none
 */
void Ssm3582Drv_PowerControl(stSsm3582Drv_t * me, bool on);

/**
 * brief : SSM3582 mute on/off.
 * @param me - instance of the driver
 * @param mute_on - 1:mute on, 0:mute off
 * @return : none
 */
void Ssm3582Drv_MuteControl(stSsm3582Drv_t * me, bool mute_on);

/**
 * brief : SSM3582 sample rate setup.
 * @param me - instance of the driver
 * @param sample_rate - SSM3582_DAC_FS_xxxxxxxx
 * @return : none
 */
void Ssm3582Drv_SampleRateControl(stSsm3582Drv_t * me, uint8_t sample_rate);

/**
 * brief : SSM3582 initial, write the all config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_Init(stSsm3582Drv_t * me);

/**
 * brief : read all the SSM3582 register
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_RegDump(stSsm3582Drv_t * me);

/**
 * brief : read the SSM3582 status register only
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_RegGetStatus(stSsm3582Drv_t * me);

#endif  // __SSM3582_DRV_H__

