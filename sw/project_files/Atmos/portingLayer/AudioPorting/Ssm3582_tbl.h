#ifndef __SSM3582_TBL_H__
#define __SSM3582_TBL_H__

/* the default register value after reset */
const static uint8_t ssm_reg_default[SSM3582_REG_END]=
{
    0x41, 0x35, 0x82, 0x01, 0xa1, 0x8a, 0x02, 0x40,
    0x40, 0x11, 0x07, 0x00, 0x01, 0x00, 0xa0, 0x51,
    0x22, 0xa8, 0x51, 0x22, 0xff, 0xff, 0x00, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t ssm_reg_woofer_left[SSM3582_REG_END] = 
{
    0x41, 0x35, 0x82, 0x01,     // vendor id, device id, revision
    (SSM3582_APWDN_ENABLE|SSM3582_TEMP_SENSOR_ON|SSM3582_OUTPUT_MONO|SSM3582_POWER_ON_ENABLE), // POWER_CTRL, default = 0xa1
    (SSM3582_DAC_LPM_ON|SSM3582_DAC_POL_NORMAL|SSM3582_EDGE_LOW_EMI|SSM3582_ANA_GAIN_19dB), // AMP_DAC_CTRL, default = 0x8a
    (SSM3582_VOL_RAMPING_ON|SSM3582_DAC_UNMUTE|SSM3582_DAC_HPF_OFF|SSM3582_DAC_FS_32_48K), // DAC_CTRL, default = 0x02
    SSM3582_DAC_VOL_0dB, // VOL_LEFT_CTRL, default = 0x40

    /* 0x08 */
    SSM3582_DAC_VOL_0dB, // VOL_RIGHT_CTRL, default = 0x40
    SSM3582_SAI_I2S_WOOF_LEFT, // SAI_CTRL1, default = 0x11
    0x07, // SAI_CTRL2, default = 0x07
    0x00, // SLOT_LEFT_CTRL, default = 0x00
    0x01, // SLOT_RIGHT_CTRL, default = 0x01
    0x00, // RESERVE1, default = ???????
    0xa0, // LIM_LEFT_CTRL1, default = 0xa0
    0x51, // LIM_LEFT_CTRL2, default = 0x51

    /* 0x10 */
    0x22, // LIM_LEFT_CTRL3, default = 0x22
    0xa8, // LIM_RIGHT_CTRL1, default = 0xa8
    0x51, // LIM_RIGHT_CTRL2, default = 0x51
    0x22, // LIM_RIGHT_CTRL3, default = 0x22
    0xff, // CLIP_LEFT_CTRL, default = 0xff
    0xff, // CLIP_RIGHT_CTRL, default = 0xff
    0x22, // -3dB when OTW // FAULT_CTRL1, default = 0x00
    0x30, // FAULT_CTRL2, default = 0x30

    /* 0x18 */
    0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t ssm_reg_tweeter_left_leftUP[SSM3582_REG_END] = 
{
    0x41, 0x35, 0x82, 0x01,     // vendor id, device id, revision
    (SSM3582_APWDN_ENABLE|SSM3582_TEMP_SENSOR_ON|SSM3582_OUTPUT_STEREO|SSM3582_POWER_ON_ENABLE), // POWER_CTRL, default = 0xa1
    (SSM3582_DAC_LPM_ON|SSM3582_DAC_POL_NORMAL|SSM3582_EDGE_LOW_EMI|SSM3582_ANA_GAIN_16dB), // AMP_DAC_CTRL, default = 0x8a
    (SSM3582_VOL_RAMPING_ON|SSM3582_DAC_UNMUTE|SSM3582_DAC_HPF_OFF|SSM3582_DAC_FS_32_48K), // DAC_CTRL, default = 0x02
    SSM3582_DAC_VOL_0dB, // VOL_LEFT_CTRL, default = 0x40

    /* 0x08 */
    SSM3582_DAC_VOL_0dB, // VOL_RIGHT_CTRL, default = 0x40
    SSM3582_SAI_I2S, // SAI_CTRL1, default = 0x11
    0x07, // SAI_CTRL2, default = 0x07
    0x00, // SLOT_LEFT_CTRL, default = 0x00
    0x01, // SLOT_RIGHT_CTRL, default = 0x01
    0x00, // RESERVE1, default = ???????
    0xa0, // LIM_LEFT_CTRL1, default = 0xa0
    0x51, // LIM_LEFT_CTRL2, default = 0x51

    /* 0x10 */
    0x22, // LIM_LEFT_CTRL3, default = 0x22
    0xa8, // LIM_RIGHT_CTRL1, default = 0xa8
    0x51, // LIM_RIGHT_CTRL2, default = 0x51
    0x22, // LIM_RIGHT_CTRL3, default = 0x22
    0xff, // CLIP_LEFT_CTRL, default = 0xff
    0xff, // CLIP_RIGHT_CTRL, default = 0xff
    0x22, // -3dB when OTW // FAULT_CTRL1, default = 0x00
    0x30, // FAULT_CTRL2, default = 0x30

    /* 0x18 */
    0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t ssm_reg_tweeter_center[SSM3582_REG_END] = 
{
    0x41, 0x35, 0x82, 0x01,     // vendor id, device id, revision
    (SSM3582_APWDN_ENABLE|SSM3582_TEMP_SENSOR_ON|SSM3582_OUTPUT_MONO|SSM3582_POWER_ON_ENABLE), // POWER_CTRL, default = 0xa1
    (SSM3582_DAC_LPM_ON|SSM3582_DAC_POL_NORMAL|SSM3582_EDGE_LOW_EMI|SSM3582_ANA_GAIN_16dB), // AMP_DAC_CTRL, default = 0x8a
    (SSM3582_VOL_RAMPING_ON|SSM3582_DAC_UNMUTE|SSM3582_DAC_HPF_OFF|SSM3582_DAC_FS_32_48K), // DAC_CTRL, default = 0x02
    SSM3582_DAC_VOL_0dB, // VOL_LEFT_CTRL, default = 0x40

    /* 0x08 */
    SSM3582_DAC_VOL_0dB, // VOL_RIGHT_CTRL, default = 0x40
    SSM3582_SAI_I2S, // SAI_CTRL1, default = 0x11
    0x07, // SAI_CTRL2, default = 0x07
    0x00, // SLOT_LEFT_CTRL, default = 0x00
    0x01, // SLOT_RIGHT_CTRL, default = 0x01
    0x00, // RESERVE1, default = ???????
    0xa0, // LIM_LEFT_CTRL1, default = 0xa0
    0x51, // LIM_LEFT_CTRL2, default = 0x51

    /* 0x10 */
    0x22, // LIM_LEFT_CTRL3, default = 0x22
    0xa8, // LIM_RIGHT_CTRL1, default = 0xa8
    0x51, // LIM_RIGHT_CTRL2, default = 0x51
    0x22, // LIM_RIGHT_CTRL3, default = 0x22
    0xff, // CLIP_LEFT_CTRL, default = 0xff
    0xff, // CLIP_RIGHT_CTRL, default = 0xff
    0x22, // -3dB when OTW // FAULT_CTRL1, default = 0x00
    0x30, // FAULT_CTRL2, default = 0x30

    /* 0x18 */
    0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t ssm_reg_woofer_center[SSM3582_REG_END] = 
{
    0x41, 0x35, 0x82, 0x01,     // vendor id, device id, revision
    (SSM3582_APWDN_ENABLE|SSM3582_TEMP_SENSOR_ON|SSM3582_OUTPUT_MONO|SSM3582_POWER_ON_ENABLE), // POWER_CTRL, default = 0xa1
    (SSM3582_DAC_LPM_ON|SSM3582_DAC_POL_NORMAL|SSM3582_EDGE_LOW_EMI|SSM3582_ANA_GAIN_19dB), // AMP_DAC_CTRL, default = 0x8a
    (SSM3582_VOL_RAMPING_ON|SSM3582_DAC_UNMUTE|SSM3582_DAC_HPF_OFF|SSM3582_DAC_FS_32_48K), // DAC_CTRL, default = 0x02
    SSM3582_DAC_VOL_0dB, // VOL_LEFT_CTRL, default = 0x40

    /* 0x08 */
    SSM3582_DAC_VOL_0dB, // VOL_RIGHT_CTRL, default = 0x40
    SSM3582_SAI_I2S_WOOF_CENTER, // SAI_CTRL1, default = 0x11
    0x07, // SAI_CTRL2, default = 0x07
    0x00, // SLOT_LEFT_CTRL, default = 0x00
    0x01, // SLOT_RIGHT_CTRL, default = 0x01
    0x00, // RESERVE1, default = ???????
    0xa0, // LIM_LEFT_CTRL1, default = 0xa0
    0x51, // LIM_LEFT_CTRL2, default = 0x51

    /* 0x10 */
    0x22, // LIM_LEFT_CTRL3, default = 0x22
    0xa8, // LIM_RIGHT_CTRL1, default = 0xa8
    0x51, // LIM_RIGHT_CTRL2, default = 0x51
    0x22, // LIM_RIGHT_CTRL3, default = 0x22
    0xff, // CLIP_LEFT_CTRL, default = 0xff
    0xff, // CLIP_RIGHT_CTRL, default = 0xff
    0x22, // -3dB when OTW // FAULT_CTRL1, default = 0x00
    0x30, // FAULT_CTRL2, default = 0x30

    /* 0x18 */
    0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t ssm_reg_tweeter_right_rightUP[SSM3582_REG_END] = 
{
    0x41, 0x35, 0x82, 0x01,     // vendor id, device id, revision
    (SSM3582_APWDN_ENABLE|SSM3582_TEMP_SENSOR_ON|SSM3582_OUTPUT_STEREO|SSM3582_POWER_ON_ENABLE), // POWER_CTRL, default = 0xa1
    (SSM3582_DAC_LPM_ON|SSM3582_DAC_POL_NORMAL|SSM3582_EDGE_LOW_EMI|SSM3582_ANA_GAIN_16dB), // AMP_DAC_CTRL, default = 0x8a
    (SSM3582_VOL_RAMPING_ON|SSM3582_DAC_UNMUTE|SSM3582_DAC_HPF_OFF|SSM3582_DAC_FS_32_48K), // DAC_CTRL, default = 0x02
    SSM3582_DAC_VOL_0dB, // VOL_LEFT_CTRL, default = 0x40

    /* 0x08 */
    SSM3582_DAC_VOL_0dB, // VOL_RIGHT_CTRL, default = 0x40
    SSM3582_SAI_I2S, // SAI_CTRL1, default = 0x11
    0x07, // SAI_CTRL2, default = 0x07
    0x00, // SLOT_LEFT_CTRL, default = 0x00
    0x01, // SLOT_RIGHT_CTRL, default = 0x01
    0x00, // RESERVE1, default = ???????
    0xa0, // LIM_LEFT_CTRL1, default = 0xa0
    0x51, // LIM_LEFT_CTRL2, default = 0x51

    /* 0x10 */
    0x22, // LIM_LEFT_CTRL3, default = 0x22
    0xa8, // LIM_RIGHT_CTRL1, default = 0xa8
    0x51, // LIM_RIGHT_CTRL2, default = 0x51
    0x22, // LIM_RIGHT_CTRL3, default = 0x22
    0xff, // CLIP_LEFT_CTRL, default = 0xff
    0xff, // CLIP_RIGHT_CTRL, default = 0xff
    0x22, // -3dB when OTW // FAULT_CTRL1, default = 0x00
    0x30, // FAULT_CTRL2, default = 0x30

    /* 0x18 */
    0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t ssm_reg_woofer_right[SSM3582_REG_END] = 
{
    0x41, 0x35, 0x82, 0x01,     // vendor id, device id, revision
    (SSM3582_APWDN_ENABLE|SSM3582_TEMP_SENSOR_ON|SSM3582_OUTPUT_MONO|SSM3582_POWER_ON_ENABLE), // POWER_CTRL, default = 0xa1
    (SSM3582_DAC_LPM_ON|SSM3582_DAC_POL_NORMAL|SSM3582_EDGE_LOW_EMI|SSM3582_ANA_GAIN_19dB), // AMP_DAC_CTRL, default = 0x8a
    (SSM3582_VOL_RAMPING_ON|SSM3582_DAC_UNMUTE|SSM3582_DAC_HPF_OFF|SSM3582_DAC_FS_32_48K), // DAC_CTRL, default = 0x02
    SSM3582_DAC_VOL_0dB, // VOL_LEFT_CTRL, default = 0x40

    /* 0x08 */
    SSM3582_DAC_VOL_0dB, // VOL_RIGHT_CTRL, default = 0x40
    SSM3582_SAI_I2S, // SAI_CTRL1, default = 0x11
    0x07, // SAI_CTRL2, default = 0x07
    0x00, // SLOT_LEFT_CTRL, default = 0x00
    0x01, // SLOT_RIGHT_CTRL, default = 0x01
    0x00, // RESERVE1, default = ???????
    0xa0, // LIM_LEFT_CTRL1, default = 0xa0
    0x51, // LIM_LEFT_CTRL2, default = 0x51

    /* 0x10 */
    0x22, // LIM_LEFT_CTRL3, default = 0x22
    0xa8, // LIM_RIGHT_CTRL1, default = 0xa8
    0x51, // LIM_RIGHT_CTRL2, default = 0x51
    0x22, // LIM_RIGHT_CTRL3, default = 0x22
    0xff, // CLIP_LEFT_CTRL, default = 0xff
    0xff, // CLIP_RIGHT_CTRL, default = 0xff
    0x22, // -3dB when OTW // FAULT_CTRL1, default = 0x00
    0x30, // FAULT_CTRL2, default = 0x30

    /* 0x18 */
    0x00, 0x00, 0x00, 0x00, 0x00
};

#endif  // __SSM3582_TBL_H__

