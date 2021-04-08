/**
*  @file      cs42526_drv_config.inc
*  @brief     This file stores all the configuration value for speicfic project.
*             different project had different configuration
*  @author    Edmond Sung
*  @date      01-2015
*  @copyright Tymphany Ltd.
*/


/* Init commands */
/* CMD to set other parts */

      // analog
const tDspData dsp_init_cmd_sec[/*DSP_INIT_CMD_SEC_LEN*/] =
{  
    {REG_POWER_CONTROL,             0xFF},
    {REG_FUNCTION_MODE,             0x08},        // Analog IN
    {REG_MISC_CONTROL,              0x09},
    //{REG_CLOCK_CONTROL,             0x0c},        //original
    {REG_CLOCK_CONTROL,             0x0e},
    {REG_RECEIVER_MODE_CONTROL_1,   0x02},
    {REG_RECEIVER_MODE_CONTROL_2,   0x71},        // RXP1
    {REG_VOL_TRANSITION_,           0x7b},
    {REG_CHANNEL_STAT_BUF_CONTROL,  0x80},
    {REG_POWER_CONTROL,             0x00},
    
    
};

const spdif_mode_cfg_t spdif_mode_cfg=
{
    .eMclk = BOARDCFG_MCLK_256FS,
    //.eSclk = BOARDCFG_SCLK_64FS,
    .eDigFormat = BOARDCFG_DIGFORMAT_I2S,
    .eDacMode = BOARDCFG_DACMOD_SSM,
    .eDacFormat = BOARDCFG_DACFORMAT_I2S,
    //.eSpdifTx = BOARDCFG_SPDIFTX_RXP0,
};


const adc_mode_cfg_t adc_mode_cfg=
{
    .eInFs = BOARDCFG_INFS_48K,
    .eMclk = BOARDCFG_MCLK_OMCK,
    .eDigFormat = BOARDCFG_DIGFORMAT_I2S,
    .eAdcMode = BOARDCFG_ADCMODE_SSM,
    .eDacFormat = BOARDCFG_DACFORMAT_I2S,
    .eDacMode = BOARDCFG_DACMOD_SSM,
};

static const uint8 MASTER_VOL[MAX_VOLUME+1] =
{
    127,        // 0
    60,         // 1
    58,         // 2
    56,         // 3
    54,         // 4
    52,         // 5
    50,         // 6
    48,         // 7
    46,         // 8
    44,         // 9
    42,         // 10
    40,         // 11
    38,         // 12
    36,         // 13
    34,         // 14
    32,         // 15
    30,         // 16
    28,         // 17
    26,         // 18
    24,         // 19
    22,         // 20
    20,         // 21
    18,          // 22
    16,          // 23
    14,          // 24
    12,          // 25
    10,          // 26
    8,          // 27
    6,          // 28
    4,          // 29
    2,          // 30
    0           // 31
    
};
