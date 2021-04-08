/**
*  @file      Drv_ADAU1761.c
*  @brief     This file declares the driver for the Audio DSP ADAU1761.
*  @modified  Daniel.Duan
*  @date      2014-09-15
*  @copyright Tymphany Ltd.
*/

#ifndef __DRV_ADAU1761_H__
#define __DRV_ADAU1761_H__

#define PARAM_RAM_WORD_LEN            (4)
#define EQ_CTRL_RAM_SLOT_NUM          (5)
#define EQ_CTRL_RAM_BYTE_NUM          (PARAM_RAM_WORD_LEN*EQ_CTRL_RAM_SLOT_NUM)

/* PLL configuration */
/* PLL integer R value, Byte4 from , bit6~3 */
typedef enum
{
    PLL_R_VAL_2 = (0x02 << 3),
    PLL_R_VAL_3 = (0x03 << 3),
    PLL_R_VAL_4 = (0x04 << 3),
    PLL_R_VAL_5 = (0x05 << 3),
    PLL_R_VAL_6 = (0x06 << 3),
    PLL_R_VAL_7 = (0x07 << 3),
    PLL_R_VAL_8 = (0x08 << 3)
} eDspPllRVal;

/* PLL input CLK divider X, byte4, bit2~1 */
typedef enum
{
    PLL_X_VAL_1 = (0x00 << 1),
    PLL_X_VAL_2 = (0x01 << 1),
    PLL_X_VAL_3 = (0x02 << 1),
    PLL_X_VAL_4 = (0x03 << 1)
} eDspPllXVal;

/* PLL type, byte4, bit0 */
typedef enum
{
    PLL_TYPE_INT   = (0x00),
    PLL_TYPE_FRAC  = (0x01)
} eDspPllType;

/* gain from LINP to Mix1, 0x400a, bit6~4 */
typedef enum
{
    LINP_2MX1_GAIN_MUTE      = (0x00 << 4),
    LINP_2MX1_GAIN_M12DB     = (0x01 << 4),
    LINP_2MX1_GAIN_M9DB      = (0x02 << 4),
    LINP_2MX1_GAIN_M6DB      = (0x03 << 4),
    LINP_2MX1_GAIN_M3DB      = (0x04 << 4),
    LINP_2MX1_GAIN_0DB       = (0x05 << 4),
    LINP_2MX1_GAIN_3DB       = (0x06 << 4),
    LINP_2MX1_GAIN_6DB       = (0x07 << 4),
} eDspLinp2MX1Gain;

/* gain from LINP to Mix1, 0x400a, bit3~1 */
typedef enum
{
    LINN_2MX1_GAIN_MUTE      = (0x00 << 1),
    LINN_2MX1_GAIN_M12DB     = (0x01 << 1),
    LINN_2MX1_GAIN_M9DB      = (0x02 << 1),
    LINN_2MX1_GAIN_M6DB      = (0x03 << 1),
    LINN_2MX1_GAIN_M3DB      = (0x04 << 1),
    LINN_2MX1_GAIN_0DB       = (0x05 << 1),
    LINN_2MX1_GAIN_3DB       = (0x06 << 1),
    LINN_2MX1_GAIN_6DB       = (0x07 << 1),
} eDspLinn2MX1Gain;

/* gain of left PGA gain to mix1, 0x400b, bit4~3 */
typedef enum
{
    LPGA_2MX1_GAIN_MUTE    = (0x00 << 3),
    LPGA_2MX1_GAIN_0DB     = (0x01 << 3),
    LPGA_2MX1_GAIN_20DB    = (0x02 << 3),
} eDspLpga2MX1Gain;

/* gain of left AUX gain to mix1, 0x400b, bit2~0 */
typedef enum
{
    LAUX_2MX1_GAIN_MUTE    = (0x00),
    LAUX_2MX1_GAIN_M12DB   = (0x01),
    LAUX_2MX1_GAIN_M9DB    = (0x02),
    LAUX_2MX1_GAIN_M6DB    = (0x03),
    LAUX_2MX1_GAIN_M3DB    = (0x04),
    LAUX_2MX1_GAIN_0DB     = (0x05),
    LAUX_2MX1_GAIN_3DB     = (0x06),
    LAUX_2MX1_GAIN_6DB     = (0x07)
} eDspLaux2MX1Gain;

/* gain of RINP to Mix2, 0x400c, bit6~4 */
typedef enum
{
    RINP_2MX2_GAIN_MUTE    = (0x00 << 4),
    RINP_2MX2_GAIN_M12DB   = (0x01 << 4),
    RINP_2MX2_GAIN_M9DB    = (0x02 << 4),
    RINP_2MX2_GAIN_M6DB    = (0x03 << 4),
    RINP_2MX2_GAIN_M3DB    = (0x04 << 4),
    RINP_2MX2_GAIN_0DB     = (0x05 << 4),
    RINP_2MX2_GAIN_3DB     = (0x06 << 4),
    RINP_2MX2_GAIN_6DB     = (0x07 << 4)
} eDspRinp2MX2Gain;

/* gain of RINN to Mix2, 0x400c, bit3~1 */
typedef enum
{
    RINN_2MX2_GAIN_MUTE    = (0x00 << 1),
    RINN_2MX2_GAIN_M12DB   = (0x01 << 1),
    RINN_2MX2_GAIN_M9DB    = (0x02 << 1),
    RINN_2MX2_GAIN_M6DB    = (0x03 << 1),
    RINN_2MX2_GAIN_M3DB    = (0x04 << 1),
    RINN_2MX2_GAIN_0DB     = (0x05 << 1),
    RINN_2MX2_GAIN_3DB     = (0x06 << 1),
    RINN_2MX2_GAIN_6DB     = (0x07 << 1)
} eDspRinn2MX2Gain;

/* gain of right PGA gain to mix2, 0x400d, bit4~3 */
typedef enum
{
    RPGA_2MX2_GAIN_MUTE    = (0x00 << 3),
    RPGA_2MX2_GAIN_0DB     = (0x01 << 3),
    RPGA_2MX2_GAIN_20DB    = (0x02 << 3),
} eDspRpga2MX2Gain;

/* gain of right AUX gain to mix2, 0x400d, bit2~0 */
typedef enum
{
    RAUX_2MX2_GAIN_MUTE    = (0x00),
    RAUX_2MX2_GAIN_M12DB   = (0x01),
    RAUX_2MX2_GAIN_M9DB    = (0x02),
    RAUX_2MX2_GAIN_M6DB    = (0x03),
    RAUX_2MX2_GAIN_M3DB    = (0x04),
    RAUX_2MX2_GAIN_0DB     = (0x05),
    RAUX_2MX2_GAIN_3DB     = (0x06),
    RAUX_2MX2_GAIN_6DB     = (0x07)
} eDspRaux2MX2Gain;

/* mix3(left) control, 0x401c bit 4~1 */
typedef enum
{
    LAUX_2MX3_GAIN_MUTE   = (0x00 << 1),
    LAUX_2MX3_GAIN_M15DB  = (0x01 << 1),
    LAUX_2MX3_GAIN_M12DB  = (0x02 << 1),
    LAUX_2MX3_GAIN_M9DB   = (0x03 << 1),
    LAUX_2MX3_GAIN_M6DB   = (0x04 << 1),
    LAUX_2MX3_GAIN_M3DB   = (0x05 << 1),
    LAUX_2MX3_GAIN_0DB    = (0x06 << 1),
    LAUX_2MX3_GAIN_3DB    = (0x07 << 1),
    LAUX_2MX3_GAIN_6DB    = (0x08 << 1)
} eDspLaux2MX3Gain;

/* mix3(left) control, 0x401d bit7~4 */
typedef enum
{
    MX2_TO_MX3_GAIN_MUTE     = (0x00 << 4),
    MX2_TO_MX3_GAIN_M15DB    = (0x01 << 4),
    MX2_TO_MX3_GAIN_M12DB    = (0x02 << 4),
    MX2_TO_MX3_GAIN_M9DB     = (0x03 << 4),
    MX2_TO_MX3_GAIN_M6DB     = (0x04 << 4),
    MX2_TO_MX3_GAIN_M3DB     = (0x05 << 4),
    MX2_TO_MX3_GAIN_0DB      = (0x06 << 4),
    MX2_TO_MX3_GAIN_3DB      = (0x07 << 4),
    MX2_TO_MX3_GAIN_6DB      = (0x08 << 4),
} eDspMx2ToMx3Gain;

/* mix3(left) control, 0x401d bit3~0 */
typedef enum
{
    MX1_TO_MX3_GAIN_MUTE     = (0x00),
    MX1_TO_MX3_GAIN_M15DB    = (0x01),
    MX1_TO_MX3_GAIN_M12DB    = (0x02),
    MX1_TO_MX3_GAIN_M9DB     = (0x03),
    MX1_TO_MX3_GAIN_M6DB     = (0x04),
    MX1_TO_MX3_GAIN_M3DB     = (0x05),
    MX1_TO_MX3_GAIN_0DB      = (0x06),
    MX1_TO_MX3_GAIN_3DB      = (0x07),
    MX1_TO_MX3_GAIN_6DB      = (0x08),
} eDspMx1ToMx3Gain;

/* mix4(right) control, 0x401e bit 4~1 */
typedef enum
{
    RAUX_2MX4_GAIN_MUTE   = (0x00 << 1),
    RAUX_2MX4_GAIN_M15DB  = (0x01 << 1),
    RAUX_2MX4_GAIN_M12DB  = (0x02 << 1),
    RAUX_2MX4_GAIN_M9DB   = (0x03 << 1),
    RAUX_2MX4_GAIN_M6DB   = (0x04 << 1),
    RAUX_2MX4_GAIN_M3DB   = (0x05 << 1),
    RAUX_2MX4_GAIN_0DB    = (0x06 << 1),
    RAUX_2MX4_GAIN_3DB    = (0x07 << 1),
    RAUX_2MX4_GAIN_6DB    = (0x08 << 1)
} eDspRaux2MX4Gain;

/* mix4(right) control, 0x401f bit7~4 */
typedef enum
{
    MX2_TO_MX4_GAIN_MUTE     = (0x00 << 4),
    MX2_TO_MX4_GAIN_M15DB    = (0x01 << 4),
    MX2_TO_MX4_GAIN_M12DB    = (0x02 << 4),
    MX2_TO_MX4_GAIN_M9DB     = (0x03 << 4),
    MX2_TO_MX4_GAIN_M6DB     = (0x04 << 4),
    MX2_TO_MX4_GAIN_M3DB     = (0x05 << 4),
    MX2_TO_MX4_GAIN_0DB      = (0x06 << 4),
    MX2_TO_MX4_GAIN_3DB      = (0x07 << 4),
    MX2_TO_MX4_GAIN_6DB      = (0x08 << 4),
} eDspMx2ToMx4Gain;

/* mix4(right) control, 0x401f bit3~0 */
typedef enum
{
    MX1_TO_MX4_GAIN_MUTE     = (0x00),
    MX1_TO_MX4_GAIN_M15DB    = (0x01),
    MX1_TO_MX4_GAIN_M12DB    = (0x02),
    MX1_TO_MX4_GAIN_M9DB     = (0x03),
    MX1_TO_MX4_GAIN_M6DB     = (0x04),
    MX1_TO_MX4_GAIN_M3DB     = (0x05),
    MX1_TO_MX4_GAIN_0DB      = (0x06),
    MX1_TO_MX4_GAIN_3DB      = (0x07),
    MX1_TO_MX4_GAIN_6DB      = (0x08),
} eDspMx1ToMx4Gain;

/* mix5(left) control, 0x4020 bit4~3 */
typedef enum
{
    MX4_TO_MX5_GAIN_MUTE    = (0x00 << 3),
    MX4_TO_MX5_GAIN_M6DB    = (0x01 << 3),
    MX4_TO_MX5_GAIN_0DB     = (0x02 << 3),
} eDspMx4ToMx5Gain;

/* mix5(left) control, 0x4020 bit2~1 */
typedef enum
{
    MX3_TO_MX5_GAIN_MUTE    = (0x00 << 1),
    MX3_TO_MX5_GAIN_M6DB    = (0x01 << 1),
    MX3_TO_MX5_GAIN_0DB     = (0x02 << 1),
} eDspMx3ToMx5Gain;

/* mix6(right) control, 0x4021 bit4~3 */
typedef enum
{
    MX4_TO_MX6_GAIN_MUTE    = (0x00 << 3),
    MX4_TO_MX6_GAIN_M6DB    = (0x01 << 3),
    MX4_TO_MX6_GAIN_0DB     = (0x02 << 3),
} eDspMx4ToMx6Gain;

/* mix6(right) control, 0x4021 bit2~1 */
typedef enum
{
    MX3_TO_MX6_GAIN_MUTE    = (0x00 << 1),
    MX3_TO_MX6_GAIN_M6DB    = (0x01 << 1),
    MX3_TO_MX6_GAIN_0DB     = (0x02 << 1),
} eDspMx3ToMx6Gain;

/* mix7(mono) control, 0x4022 Bit2~1 */
typedef enum
{
    MX7_CTRL_COMMON_OUT       = (0x00 << 1),
    MX7_CTRL_M6DB_EACH        = (0x01 << 1),
    MX7_TRL_0DB_EACH          = (0x02 << 1)
} eDspMx7Ctrl;

/* serial/control port pad pull-up/down setting, 0x402d 0x402f */
typedef enum
{
    PAD_CTRL_PULLUP    = (0x00),
    PAD_CTRL_NONE      = (0x02),
    PAD_CTRL_PULLDOWN  = (0x03)
} eDspPadCtrl;

/* GPIO pins type sel */
typedef enum
{
    GPIO3_TYPE_LRCK     = (0x00 << 3),
    GPIO3_TYPE_GPIO     = (0x01 << 3)
} eDspGpio3Type;

typedef enum
{
    GPIO2_TYPE_BCK      = (0x00 << 2),
    GPIO2_TYPE_GPIO     = (0x01 << 2)
} eDspGpio2Type;

typedef enum
{
    GPIO1_TYPE_DOUT     = (0x00 << 1),
    GPIO1_TYPE_GPIO     = (0x01 << 1)
} eDspGpio1Type;

typedef enum
{
    GPIO0_TYPE_DIN      = (0x00),
    GPIO0_TYPE_GPIO     = (0x01)
} eDspGpio0Type;

/* GPIO pins function sel */
typedef enum
{
    GPIO_FUNC_IN_NO_DEB              = (0x00),
    GPIO_FUNC_IN_D3MS_DEB            = (0x01),
    GPIO_FUNC_IN_D6MS_DEB            = (0x02),
    GPIO_FUNC_IN_D9MS_DEB            = (0x03),
    GPIO_FUNC_IN_5MS_DEB             = (0x04),
    GPIO_FUNC_IN_10MS_DEB            = (0x05),
    GPIO_FUNC_IN_20MS_DEB            = (0x06),
    GPIO_FUNC_IN_40MS_DEB            = (0x07),
    GPIO_FUNC_IN_PORT_CTRL           = (0x08),
    GPIO_FUNC_OUT_MCU_CTRL_PULLUP    = (0x09),
    GPIO_FUNC_OUT_MCU_CTRL_NO_PULLUP = (0x0A),
    GPIO_FUNC_OUT_DSP_CTRL_PULLUP    = (0x0B),
    GPIO_FUNC_OUT_DSP_CTRL_NO_PULLUP = (0x0C),
    GPIO_FUNC_CRC_ERR                = (0x0E),
    GPIO_FUNC_WD_ERR                 = (0x0F)
} eDspGpioFunc;

/* serial audio port setting */
/* 0x4015 bit6 */
typedef enum
{
    SER_PORT_FS_SRC_CONVERTER       = (0x00 << 6),
    SER_PORT_FS_SRC_DSP             = (0x01 << 6),
} eDspSerPortFsSrc;

/* 0x4016 bit5 */
typedef enum
{
    SER_PORT_LRCLK_MODE_HALF             = (0x00 << 5),
    SER_PORT_LRCLK_MODE_BURST            = (0x01 << 5),
} eDspSerPortLrclkMode;

/* bit4 */
typedef enum
{
    SER_PORT_BCLK_POL_FALLING          = (0x00 << 4),
    SER_PORT_BCLK_POL_RISING           = (0x01 << 4),
} eDspSerPortBclkPol;

/* bit3 */
typedef enum
{
    SER_PORT_LRCLK_POL_FALLING          = (0x00 << 3),
    SER_PORT_LRCLK_POL_RISING           = (0x01 << 3),
} eDspSerPortLrclkPol;

/* bit2~1 */
typedef enum
{
    SER_PORT_CHL_STEREO              = (0x00 << 1),
    SER_PORT_CHL_TDM4                = (0x01 << 1),
    SER_PORT_CHL_TDM8                = (0x02 << 1),
} eDspSerPortChlMode;

/* bit0 */
typedef enum
{
    SER_PORT_SLAVE_MODE              = (0x00),
    SER_PORT_MASTER_MODE             = (0x01)
} eDspSerPortBusMode;

/* 0x4016, bit7~5 */
typedef enum
{
    SER_PORT_64_BITS_PER_FRAME       = (0x00 << 5),
    SER_PORT_48_BITS_PER_FRAME       = (0x02 << 5),
    SER_PORT_128_BITS_PER_FRAME      = (0x03 << 5),
    SER_PORT_256_BITS_PER_FRAME      = (0x04 << 5),
} eDspSerPortBitPerFrame;

/* bit4 */
typedef enum
{
    SER_PORT_TDM_DOUT_POS_LEFT_FIRST      = (0x00 << 4),
    SER_PORT_TDM_DOUT_POS_RIGHT_FIRST     = (0x01 << 4)
} eDspSerPortTdmDoutPos;

/* bit3 */
typedef enum
{
    SER_PORT_TDM_DIN_POS_LEFT_FIRST       = (0x00 << 3),
    SER_PORT_TDM_DIN_POS_RIGHT_FIRST      = (0x01 << 3)
} eDspSerPortTdmDinPos;

/* bit2 */
typedef enum
{
    SER_PORT_FRAME_MSB_FIRST        = (0x00 << 2),
    SER_PORT_FRAME_LSB_FIRST        = (0x01 << 2),
} eDspSerPortMsbPos;

/* bit1~0 */
typedef enum
{
    SER_PORT_AUD_DLY_1             = (0x00),
    SER_PORT_AUD_DLY_0             = (0x01),
    SER_PORT_AUD_DLY_8             = (0x02),
    SER_PORT_AUD_DLY_16            = (0x03)
} eDspSerPortDataDly;

/*
 * The filter types we can support
 */
typedef enum
{
    FILTER_TYPE_PEAKING,
    FILTER_TYPE_TONE_HIGH_SHELF,
    FILTER_TYPE_TONE_LOW_SHELF,
    FILTER_TYPE_GENERAL_HIGH_PASS,
    FILTER_TYPE_GENERAL_LOW_PASS,
    FILTER_TYPE_BUTTWORTH_LOW_PASS,
    FILTER_TYPE_BUTTWORTH_HIGH_PASS,
    FILTER_TYPE_BESSEL_LOW_PASS,
    FILTER_TYPE_BESSEL_HIGH_PASS
} eDspFiltTyp;





/* PLL settings */
typedef struct
{
    const uint8* head;
    uint8        type;
    uint8        page;
    uint8        reg;
} tDspTunableInfo;

/* PLL cfg structure */
typedef struct
{
    /* Word0 */
    uint16       pll_denom_val;
    uint16       pll_numer_val;
    /* Word1 */
    eDspPllRVal   pll_r_val: 4;
    eDspPllXVal   pll_x_val: 2;
    eDspPllType    pll_type: 1;
} tDspPllCfg;

/* analog input route */
typedef struct
{
    /* left channel in */
    eDspLinp2MX1Gain        linp_2mx1_gain: 3;
    eDspLinn2MX1Gain        linn_2mx1_gain: 3;
    bool                           mix1_en: 1;
    eDspLpga2MX1Gain        lpga_2mx1_gain: 2;
    eDspLaux2MX1Gain        laux_2mx1_gain: 3;
    /* right channel in */
    eDspRinp2MX2Gain        rinp_2mx2_gain: 3;
    eDspRinn2MX2Gain        rinn_2mx2_gain: 3;
    bool                           mix2_en: 1;
    eDspRpga2MX2Gain        rpga_2mx2_gain: 2;
    eDspRaux2MX2Gain        raux_2mx2_gain: 3;
    /* lpga volume control */
    uint8                    lpga_2mx1_vol: 6;
    bool                   lpga_2mx1_in_en: 1;
    bool                           lpga_en: 1;
    /* rpga volume control */
    uint8                    rpga_2mx2_vol: 6;
    bool                   rpga_2mx2_in_en: 1;
    bool                           rpga_en: 1;
} tDspAnaInCfg;

/* analog HP and Line output route */
typedef struct
{
    /* mix3(left) cfg */
    bool                      mx3_rdac_in: 1;
    bool                      mx3_ldac_in: 1;
    eDspLaux2MX3Gain       laux_2mx3_gain: 4;
    bool                           mx3_en: 1;
    eDspMx2ToMx3Gain        mx2_2mx3_gain: 4;
    eDspMx1ToMx3Gain        mx1_2mx3_gain: 4;
    /* mix4(right) cfg */
    bool                      mx4_rdac_in: 1;
    bool                      mx4_ldac_in: 1;
    eDspRaux2MX4Gain       raux_2mx4_gain: 4;
    bool                           mx4_en: 1;
    eDspMx2ToMx4Gain        mx2_2mx4_gain: 4;
    eDspMx1ToMx4Gain        mx1_2mx4_gain: 4;
    /* mix5(left) cfg */
    eDspMx4ToMx5Gain        mx4_2mx5_gain: 2;
    eDspMx3ToMx5Gain        mx3_2mx5_gain: 2;
    bool                           mx5_en: 1;
    /* mix6(right) cfg */
    eDspMx4ToMx6Gain        mx4_2mx6_gain: 2;
    eDspMx3ToMx6Gain        mx3_2mx6_gain: 2;
    bool                           mx6_en: 1;
    /* mix7(mono) cfg */
    eDspMx7Ctrl                  mx7_ctrl: 2;
    bool                           mx7_en: 1;
    /* left phone cfg */
    uint8                         lhp_vol: 6;
    bool                           lhp_en: 1;
    bool                   hp_vol_ctrl_en: 1;
    /* right phone cfg */
    uint8                         rhp_vol: 6;
    bool                           rhp_en: 1;
    bool                       hpout_mode: 1;
    /* left lo cfg */
    uint8                        lout_vol: 6;
    bool                          lout_en: 1;
    bool                        lout_mode: 1;
    /* right lo cfg */
    uint8                        rout_vol: 6;
    bool                          rout_en: 1;
    bool                        rout_mode: 1;
    /* mono out cfg */
    uint8                        mono_vol: 6;
    bool                          mono_en: 1;
    bool                        mono_mode: 1;
} tDspAnaOutCfg;

/* serial audio(I2S/TDM) cfg */
typedef struct
{
    /* Word0 */
    eDspSerPortFsSrc                 fs_src: 1;
    eDspSerPortLrclkMode         lrclk_mode: 1;
    eDspSerPortBclkPol             bclk_pol: 1;
    eDspSerPortLrclkPol           lrclk_pol: 1;
    eDspSerPortChlMode             chl_mode: 2;
    eDspSerPortBusMode             bus_mode: 1;
    eDspSerPortBitPerFrame    bit_per_frame: 3;
    eDspSerPortTdmDoutPos      tdm_dout_pos: 1;
    eDspSerPortTdmDinPos        tdm_din_pos: 1;
    eDspSerPortMsbPos               msb_pos: 1;
    eDspSerPortDataDly             data_dly: 2;
} tDspDigAudCfg;

/* GPIO0 cfg */
typedef struct
{
    eDspGpio0Type            gpio0_type: 1;
    eDspGpioFunc             gpio0_func: 4;
} tDspGpio0Cfg;

/* GPIO1 cfg */
typedef struct
{
    eDspGpio1Type            gpio1_type: 1;
    eDspGpioFunc             gpio1_func: 4;
} tDspGpio1Cfg;

/* GPIO2 cfg */
typedef struct
{
    eDspGpio2Type            gpio2_type: 1;
    eDspGpioFunc             gpio2_func: 4;
} tDspGpio2Cfg;

/* GPIO3 cfg */
typedef struct
{
    eDspGpio3Type            gpio3_type: 1;
    eDspGpioFunc             gpio3_func: 4;
} tDspGpio3Cfg;

/* For filter, each one is in (4*0 + 5.23 format) */
typedef struct
{
    double            q;
    double        boost;
    double         gain;
    uint16         freq;
    eDspFiltTyp    type: 7;
    bool             en: 1;
} tDspFiltRawParam;

/* EQ ctrl name to filter type and id */
typedef struct
{
    eDspEqCtrl       name;
    eDspFiltTyp      type;
    uint8             id;
} tDspEqTransTab;


/* Dsp cfg parts, all are product-dependent */
#define SAMPLING_FREQ                        (48000)
#define EQ_CTRL_NUM                          (5)


/* PLL cfg */
const tDspPllCfg PLL_CFG0 =
{
    /* Word0 */
    0,                //pll_denom_val;
    0,                //pll_numer_val;
    /* Word1 */
    PLL_R_VAL_4,      //pll_r_val: 4;
    PLL_X_VAL_1,      //pll_x_val: 2;
    PLL_TYPE_INT,     //pll_type: 1;
};

/* analog inputs cfg */
const tDspAnaInCfg ANA_INPUT_CFG0 =
{
    /* left channel in */
    LINP_2MX1_GAIN_MUTE,           //linp_2mx1_gain: 3;
    LINN_2MX1_GAIN_MUTE,           //linn_2mx1_gain: 3;
    TRUE,                          //mix1_en: 1;
    LPGA_2MX1_GAIN_MUTE,           //lpga_2mx1_gain: 2;
    LAUX_2MX1_GAIN_0DB,            //laux_2mx1_gain: 3;
    /* right channel in */
    RINP_2MX2_GAIN_MUTE,           //rinp_2mx2_gain: 3;
    RINN_2MX2_GAIN_MUTE,           //rinn_2mx2_gain: 3;
    TRUE,                          //mix2_en: 1;
    RPGA_2MX2_GAIN_MUTE,           //rpga_2mx2_gain: 2;
    RAUX_2MX2_GAIN_0DB,            //raux_2mx2_gain: 3;
    /* lpga volume control */
    0,                             //lpga_2mx1_vol: 6;
    TRUE,                          //lpga_2mx1_in: 1;
    TRUE,                          //lpga_en: 1;
    /* rpga volume control */
    0,                             //rpga_2mx2_vol: 6;
    TRUE,                          //rpga_2mx2_in: 1;
    TRUE,                          //rpga_en: 1;
};

const tDspAnaInCfg ANA_INPUT_CFG1 =
{
    /* left channel in */
    LINP_2MX1_GAIN_MUTE,           //linp_2mx1_gain: 3;
    LINN_2MX1_GAIN_MUTE,           //linn_2mx1_gain: 3;
    TRUE,                          //mix1_en: 1;
    LPGA_2MX1_GAIN_0DB,            //lpga_2mx1_gain: 2;
    LAUX_2MX1_GAIN_MUTE,           //laux_2mx1_gain: 3;
    /* right channel in */
    RINP_2MX2_GAIN_MUTE,           //rinp_2mx2_gain: 3;
    RINN_2MX2_GAIN_MUTE,           //rinn_2mx2_gain: 3;
    TRUE,                          //mix2_en: 1;
    RPGA_2MX2_GAIN_0DB,            //rpga_2mx2_gain: 2;
    RAUX_2MX2_GAIN_MUTE,           //raux_2mx2_gain: 3;
    /* lpga volume control */
    0x10,                          //lpga_2mx1_vol: 6;
    TRUE,                          //lpga_2mx1_in: 1;
    TRUE,                          //lpga_en: 1;
    /* rpga volume control */
    0x10,                          //rpga_2mx2_vol: 6;
    TRUE,                          //rpga_2mx2_in: 1;
    TRUE,                          //rpga_en: 1;
};

const tDspAnaInCfg ANA_INPUT_CFG2 =
{
    /* left channel in */
    LINP_2MX1_GAIN_MUTE,           //linp_2mx1_gain: 3;
    LINN_2MX1_GAIN_MUTE,           //linn_2mx1_gain: 3;
    FALSE,                         //mix1_en: 1;
    LPGA_2MX1_GAIN_MUTE,           //lpga_2mx1_gain: 2;
    LAUX_2MX1_GAIN_MUTE,           //laux_2mx1_gain: 3;
    /* right channel in */
    RINP_2MX2_GAIN_MUTE,           //rinp_2mx2_gain: 3;
    RINN_2MX2_GAIN_MUTE,           //rinn_2mx2_gain: 3;
    FALSE,                         //mix2_en: 1;
    RPGA_2MX2_GAIN_MUTE,           //rpga_2mx2_gain: 2;
    RAUX_2MX2_GAIN_MUTE,           //raux_2mx2_gain: 3;
    /* lpga volume control */
    0x10,                          //lpga_2mx1_vol: 6;
    FALSE,                         //lpga_2mx1_in: 1;
    FALSE,                         //lpga_en: 1;
    /* rpga volume control */
    0x10,                          //rpga_2mx2_vol: 6;
    FALSE,                         //rpga_2mx2_in: 1;
    FALSE,                         //rpga_en: 1;
};

/* Analog output cfg */
const tDspAnaOutCfg ANA_OUTPUT_CFG0 =
{
    /* mix3(left) cfg */
    FALSE,                            //mx3_rdac_in: 1;
    TRUE,                             //mx3_ldac_in: 1;
    LAUX_2MX3_GAIN_MUTE,              //laux_2mx3_gain: 4;
    TRUE,                             //mx3_en: 1;
    MX2_TO_MX3_GAIN_MUTE,             //mx2_2mx3_gain: 4;
    MX1_TO_MX3_GAIN_MUTE,             //mx1_2mx3_gain: 4;
    /* mix4(right) cfg */
    TRUE,                             //mx4_rdac_in: 1;
    FALSE,                            //mx4_ldac_in: 1;
    RAUX_2MX4_GAIN_MUTE,              //raux_2mx4_gain: 4;
    TRUE,                             //mx4_en: 1;
    MX2_TO_MX4_GAIN_MUTE,             //mx2_2mx4_gain: 4;
    MX1_TO_MX4_GAIN_MUTE,             //mx1_2mx4_gain: 4;
    /* mix5(left) cfg */
    MX4_TO_MX5_GAIN_MUTE,             //mx4_2mx5_gain: 2;
    MX3_TO_MX5_GAIN_0DB,              //mx3_2mx5_gain: 2;
    TRUE,                             //mx5_en: 1;
    /* mix6(right) cfg */
    MX4_TO_MX6_GAIN_0DB,              //mx4_2mx6_gain: 2;
    MX3_TO_MX6_GAIN_MUTE,             //mx3_2mx6_gain: 2;
    TRUE,                             //mx6_en: 1;
    /* mix7(mono) cfg */
    MX7_CTRL_COMMON_OUT,              //mx7_ctrl: 2;
    FALSE,                            //mx7_en: 1;
    /* left phone cfg */
    0,                                //lhp_vol: 6;
    FALSE,                            //lhp_en: 1;
    FALSE,                            //hp_vol_ctrl_en: 1;
    /* right phone cfg */
    0,                                //rhp_vol: 6;
    FALSE,                            //rhp_en: 1;
    FALSE,                            //hpout_mode: 1;
    /* left lo cfg */
    0x39,                             //lout_vol: 6;
    TRUE,                             //lout_en: 1;
    0,                                //lout_mode: 1;
    /* right lo cfg */
    0x39,                             //rout_vol: 6;
    TRUE,                             //rout_en: 1;
    0,                                //rout_mode: 1;
    /* mono out cfg */
    0,                                //mono_vol: 6;
    FALSE,                            //mono_en: 1;
    0,                                //mono_mode: 1;
};

/* serial audio cfg */
const tDspDigAudCfg DIG_AUD_CFG0 =
{
    /* Word0 */
    SER_PORT_FS_SRC_CONVERTER,          //fs_src: 1;
    SER_PORT_LRCLK_MODE_HALF,           //lrclk_mode: 1;
    SER_PORT_BCLK_POL_FALLING,          //bclk_pol: 1;
    SER_PORT_LRCLK_POL_FALLING,         //lrclk_pol: 1;
    SER_PORT_CHL_STEREO,                //chl_mode: 2;
    SER_PORT_MASTER_MODE,               //bus_mode: 1;
    SER_PORT_64_BITS_PER_FRAME,         //bit_per_frame: 3;
    SER_PORT_TDM_DOUT_POS_LEFT_FIRST,   //tdm_dout_pos: 1;
    SER_PORT_TDM_DIN_POS_LEFT_FIRST,    //tdm_din_pos: 1;
    SER_PORT_FRAME_MSB_FIRST,           //msb_pos: 1;
    SER_PORT_AUD_DLY_1,                 //data_dly: 2;
}

/* GPIO setting */
/* GPIO0 cfg */
const tDspGpio0Cfg GPIO0_CFG0 =
{
    GPIO0_TYPE_DIN,          //gpio0_type: 1;
    GPIO_FUNC_IN_NO_DEB,     //gpio0_func: 4;
} ;

const tDspGpio0Cfg GPIO0_CFG1 =
{
    GPIO0_TYPE_GPIO,         //gpio0_type: 1;
    GPIO_FUNC_IN_NO_DEB,     //gpio0_func: 4;
} ;

/* GPIO1 cfg */
const tDspGpio1Cfg GPIO1_CFG0 =
{
    GPIO1_TYPE_DOUT,         //gpio1_type: 1;
    GPIO_FUNC_IN_NO_DEB,     //gpio1_func: 4;
} ;

/* GPIO1 cfg */
const tDspGpio1Cfg GPIO1_CFG1 =
{
    GPIO1_TYPE_GPIO,         //gpio1_type: 1;
    GPIO_FUNC_IN_NO_DEB,     //gpio1_func: 4;
} ;

/* GPIO2 cfg */
const tDspGpio2Cfg GPIO2_CFG0 =
{
    GPIO2_TYPE_BCK,          //gpio2_type: 1;
    GPIO_FUNC_IN_NO_DEB,     //gpio2_func: 4;
} ;

/* GPIO2 cfg */
const tDspGpio2Cfg GPIO2_CFG1 =
{
    GPIO2_TYPE_GPIO,         //gpio2_type: 1;
    GPIO_FUNC_IN_NO_DEB,     //gpio2_func: 4;
} ;

/* GPIO3 cfg */
const tDspGpio3Cfg GPIO3_CFG0 =
{
    GPIO3_TYPE_LRCK,         //gpio3_type: 1;
    GPIO_FUNC_IN_NO_DEB,     //gpio3_func: 4;
} ;

/* GPIO3 cfg */
const tDspGpio3Cfg GPIO3_CFG1 =
{
    GPIO3_TYPE_GPIO,         //gpio3_type: 1;
    GPIO_FUNC_IN_NO_DEB,     //gpio3_func: 4;
} ;

/* translation table, from eDspEqCtrl to type and id */
const tDspEqTransTab EQ_TRANS_TAB[EQ_CTRL_NUM] =
{
    {EQ_CTRL_BGC,       FILTER_TYPE_TONE_LOW_SHELF,         0},
    {EQ_CTRL_USER_LP,   FILTER_TYPE_BUTTWORTH_LOW_PASS,     1},
    {EQ_CTRL_USER_HP,   FILTER_TYPE_BUTTWORTH_HIGH_PASS,    2},
    {EQ_CTRL_TUNING,    FILTER_TYPE_TONE_LOW_SHELF,         3},
    {EQ_CTRL_PEQ,       FILTER_TYPE_PEAKING,                4}
};

/* necessary constants for each EQ part */
/* THE FOLLOWING ENUM IS PRODUCT-RELATED, USER CAN DEFINE THE AUDIO CFG CHOICES HERE */
/* Tihs is just an example, user can modify it according the HW design */
typedef enum
{
    PROD_AUD_IN_OPTION0,
    PROD_AUD_IN_OPTION1,
    PROD_AUD_IN_OPTION2
} eDspAudInOption;

typedef enum
{
    PROD_AUD_OUT_OPTION0
} eDspAudOutOption;

/* the EQ controllers in the DSP algorithm */
typedef enum
{
    EQ_CTRL_BGC,
    EQ_CTRL_USER_LP,
    EQ_CTRL_USER_HP,
    EQ_CTRL_TUNING,
    EQ_CTRL_PEQ
} eDspEqCtrl;

/* BGC */
#define BGC_FREQ                        (30)
#define BGC_GAIN                        (0)
#define BGC_QFACTOR                     (1)

typedef struct
{
    eDspEqCtrl   name;
    int8        boost;
    bool           en;
} tDspBgcInputParam;

/* consts for User LP */
typedef enum
{
    USER_LP_SLOPE_6DB  =  6,
    USER_LP_SLOPE_12DB = 12,
    USER_LP_SLOPE_18DB = 18,
    USER_LP_SLOPE_24DB = 24,
} eUserLpSlope;

#define USER_LP_QFACTOR           (1.41)
#define USER_LP_BOOST             (0)
#define USER_LP_GAIN              (0)
#define USER_LP_STAGE_NUM         (4)

typedef struct
{
    eDspEqCtrl       name;
    uint16           freq;
    eUserLpSlope    slope;
    bool               en;
} tDspUserLpParam;

/* consts for User HP */
typedef enum
{
    USER_HP_SLOPE_6DB  =  6,
    USER_HP_SLOPE_12DB = 12,
    USER_HP_SLOPE_18DB = 18,
    USER_HP_SLOPE_24DB = 24,
} eUserHpSlope;

#define USER_HP_QFACTOR           (1.41)
#define USER_HP_BOOST             (0)
#define USER_HP_GAIN              (0)
#define USER_HP_STAGE_NUM         (4)

typedef struct
{
    eDspEqCtrl       name;
    uint16           freq;
    eUserHpSlope    slope;
    bool               en;
} tDspUserHpParam;

/* consts for TUNING */
typedef enum
{
    TUNING_RANGE_20HZ   = 20,
    TUNING_RANGE_16HZ   = 16,
    TUNING_RANGE_SEALED =  0,
} eTuningRange;

#define TUNING_FREQ               (50)
#define TUNING_QFACTOR            (1)
#define TUNING_BOOST0             (-12)
#define TUNING_BOOST1             (-6)
#define TUNING_GAIN               (0)
#define TUNING_RANGE_NUM          (3)

typedef struct
{
    eDspEqCtrl             name;
    eTuningRange          range;
    bool                     en;
} tDspTuningParam;

/* consts for PEQ */
typedef enum
{
    PARAM_EQ_0,
    PARAM_EQ_1,
    PARAM_EQ_2
} eDspParamEq;

#define PARAM_EQ_GAIN         (0)
#define PARAM_EQ_STAGE_NUM    (3)

const uint8 direct_pass_data[] =
{
    0x00, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

typedef struct
{
    eDspEqCtrl         name;
    eDspParamEq         peq;
    uint16             freq;
    int16             boost;
    double                q;
    bool                 en;
} tDspPeqParam;


/* Vol Table */
const uint8 vol_tab[MAX_VOLUME + 1] =
{
    {0x00, 0x00, 0x01, 0x00},    /* mute */
    {0x00, 0x00, 0x29, 0x41},    /* -58dB */
    {0x00, 0x00, 0x2E, 0x49},    /* -57dB */
    {0x00, 0x00, 0x33, 0xEF},    /* -56dB */
    {0x00, 0x00, 0x3A, 0x45},    /* -55dB */
    {0x00, 0x00, 0x41, 0x61},    /* -54dB */
    {0x00, 0x00, 0x49, 0x5C},    /* -53dB */
    {0x00, 0x00, 0x52, 0x4F},    /* -52dB */
    {0x00, 0x00, 0x5C, 0x5A},    /* -51dB */
    {0x00, 0x00, 0x67, 0x9F},    /* -50dB */
    {0x00, 0x00, 0x74, 0x44},    /* -49dB */
    {0x00, 0x00, 0x82, 0x74},    /* -48dB */
    {0x00, 0x00, 0x92, 0x5E},    /* -47dB */
    {0x00, 0x00, 0xA4, 0x3B},    /* -46dB */
    {0x00, 0x00, 0xB8, 0x45},    /* -45dB */
    {0x00, 0x00, 0xCE, 0xC0},    /* -44dB */
    {0x00, 0x00, 0xE7, 0xFB},    /* -43dB */
    {0x00, 0x01, 0x04, 0x49},    /* -42dB */
    {0x00, 0x01, 0x24, 0x0C},    /* -41dB */
    {0x00, 0x01, 0x47, 0xAE},    /* -40dB */
    {0x00, 0x01, 0x6F, 0xAA},    /* -39dB */
    {0x00, 0x01, 0x9C, 0x86},    /* -38dB */
    {0x00, 0x01, 0xCE, 0xDC},    /* -37dB */
    {0x00, 0x02, 0x07, 0x56},    /* -36dB */
    {0x00, 0x02, 0x46, 0xB5},    /* -35dB */
    {0x00, 0x02, 0x8D, 0xCE},    /* -34dB */
    {0x00, 0x02, 0xDD, 0x96},    /* -33dB */
    {0x00, 0x03, 0x37, 0x18},    /* -32dB */
    {0x00, 0x03, 0x9B, 0x87},    /* -31dB */
    {0x00, 0x04, 0x0C, 0x37},    /* -30dB */
    {0x00, 0x04, 0x8A, 0xA7},    /* -29dB */
    {0x00, 0x05, 0x18, 0x84},    /* -28dB */
    {0x00, 0x05, 0xB7, 0xB1},    /* -27dB */
    {0x00, 0x06, 0x6A, 0x4A},    /* -26dB */
    {0x00, 0x07, 0x32, 0xAE},    /* -25dB */
    {0x00, 0x08, 0x13, 0x85},    /* -24dB */
    {0x00, 0x09, 0x0F, 0xCC},    /* -23dB */
    {0x00, 0x0A, 0x2A, 0xDA},    /* -22dB */
    {0x00, 0x0B, 0x68, 0x73},    /* -21dB */
    {0x00, 0x0C, 0xCC, 0xCC},    /* -20dB */
    {0x00, 0x0E, 0x5C, 0xA1},    /* -19dB */
    {0x00, 0x10, 0x1D, 0x3F},    /* -18dB */
    {0x00, 0x12, 0x14, 0x9A},    /* -17dB */
    {0x00, 0x14, 0x49, 0x61},    /* -16dB */
    {0x00, 0x16, 0xC3, 0x11},    /* -15dB */
    {0x00, 0x19, 0x8A, 0x13},    /* -14dB */
    {0x00, 0x1C, 0xA7, 0xD7},    /* -13dB */
    {0x00, 0x20, 0x26, 0xF3},    /* -12dB */
    {0x00, 0x24, 0x13, 0x47},    /* -11dB */
    {0x00, 0x28, 0x7A, 0x26},    /* -10dB */
    {0x00, 0x2D, 0x6A, 0x86},    /* -9dB */
    {0x00, 0x32, 0xF5, 0x2C},    /* -8dB */
    {0x00, 0x39, 0x2C, 0xEE},    /* -7dB */
    {0x00, 0x40, 0x26, 0xE7},    /* -6dB */
    {0x00, 0x47, 0xFA, 0xCC},    /* -5dB */
    {0x00, 0x50, 0xC3, 0x36},    /* -4dB */
    {0x00, 0x5A, 0x9D, 0xF8},    /* -3dB */
    {0x00, 0x65, 0xAC, 0x8C},    /* -2dB */
    {0x00, 0x72, 0x14, 0x83},    /* -1dB */
    {0x00, 0x80, 0x00, 0x00},    /* 0dB */
};


/**
 * Activate/deactivate ADC out.
 *
 * @param      bool            enable/disable ADC
 *
 * @return     void
 */
void DSPDrv_ActivateADCOut(bool input);

/**
 * Activate/deactivate DAC out.
 *
 * @param      bool            enable/disable DAC
 *
 * @return     void
 */
void DSPDrv_ActivateDACOut(bool input);

/**
 * Activate/deactivate HP out.
 *
 * @param      bool            enable/disable HP
 *
 * @return     void
 */
void DSPDrv_ActivateHPAOut(bool input);

/**
 * Activate/deactivate line out.
 *
 * @param      bool            enable/disable lineout
 *
 * @return     void
 */
void DSPDrv_ActivateLoOut(bool input);

/**
 * Config the PLL according to the input
 *
 * @param      tDspPllCfg*        pointer to the PLL config struct
 *
 * @return     void
 */
void DSPDrv_configPLL(tDspPllCfg *p);

void DSPDrv_Ctor(cDSPDrv* me);

void DSPDrv_Xtor(cDSPDrv* me);

/* PRODUCT-DEPENDENT FUNCTIONS */
/**
 * Swith the audio in path according to product HW
 *
 * @param      eDspAudInOption        target audio in option
 * @return     void
 */
void DSPDrv_switchAudioIn(eDspAudInOption in);

/**
 * Swith the audio out path according to product HW
 *
 * @param      eDspAudOutOption        target audio out option
 * @return     void
 */
void DSPDrv_switchAudioOut(eDspAudOutOption out);

/**
 * Set volume
 *
 * @param      uint8        target volume
 * @return     void
 */
void DSPDrv_setVolume(uint8 vol);


/**
 * Set BGC
 *
 * @param      int8            boost
 * @param      bool            enable or disable
 * @return     void
 */
void DSPDrv_SetBGC(int8 boost, bool en);

/**
 * Set User LP
 *
 * @param      int8            boost
 * @param      eUserLpSlope    slope
 * @param      bool            en
 * @return     void
 */
void DSPDrv_SetUserLp(uint16 freq, eUserLpSlope slope, bool en);

/**
 * Set User HP
 *
 * @param      int8            boost
 * @param      eUserHpSlope    slope
 * @param      bool            en
 * @return     void
 */
void DSPDrv_SetUserHp(uint16 freq, eUserHpSlope slope, bool en);

/**
 * Set Tuning
 *
 * @param      eTuningRange           range
 * @return     void
 */
void DSPDrv_SetTuning(eTuningRange range);

/**
 * Set PEQ
 *
 * @param      ePresetEQ        EQ
 * @param      uint16           freq
 * @param      doubel           gain
 * @param      double           q
 * @return     void
 */
void DSPDrv_SetPeq(ePresetEQ eq);

/**
 * Set delay
 *
 * @param      uint8       phase shift 0-180
 * @return     void
 */
void DSPDrv_SetPol(uint8 shift);

/**
 * Set user-defined EQ
 *
 * @param      void*         pointer to input data structure
 * @return     void
 */
void DSPDrv_SetUserEq(void* ptr);



/* DSP chip address */
#define SAFE_LOAD_MODULO_SIZE_ADD             (0x0000)
#define SAFE_LOAD_DATA_START_ADD              (0x0001)
#define SAFE_LOAD_ADD_FOR_TARGET_ADD          (0x0006)
#define SAFE_LOAD_SET_DATA_SIZE_ADD           (0x0007)
#define SAFE_LOAD_DATA_SIZE_MAX               (5)

/* GPIO pin control mirror register while controlled by MCU */
#define GPIO0_SET_REG                         (0x0620)
#define GPIO1_SET_REG                         (0x0621)
#define GPIO2_SET_REG                         (0x0622)
#define GPIO3_SET_REG                         (0x0623)

/* ClkCtrlRegister  - Registers (IC 1) */
#define CLKCTRLREGISTER_ADDR                  (0x4000)

/* RegPowCtrlRegister  - Registers (IC 1) */
#define REGPOWCTRLREGISTER_ADDR               (0x4001)

/* PLLCrlRegister  - Registers (IC 1) */
#define PLLCTRLREGISTER_ADDR                  (0x4002)

/* MicCtrlRegister  - Registers (IC 1) */
#define MICCTRLREGISTER_ADDR                  (0x4008)

/* Record Pwr Management  - Registers (IC 1) */
#define RECORD_PWR_MANAGEMENT_ADDR            (0x4009)

/* Record Mixer Left Ctrl 0  - Registers (IC 1) */
#define RECORD_MIXER_LEFT_CTRL_0_ADDR         (0x400A)

/* Record Mixer Left Ctrl 1  - Registers (IC 1) */
#define RECORD_MIXER_LEFT_CTRL_1_ADDR         (0x400B)

/* Record Mixer Right Ctrl 0  - Registers (IC 1) */
#define RECORD_MIXER_RIGHT_CTRL_0_ADDR        (0x400C)

/* Record Mixer Right Ctrl 1  - Registers (IC 1) */
#define RECORD_MIXER_RIGHT_CTRL_1_ADDR        (0x400D)

/* Record Volume Ctrl Left  - Registers (IC 1) */
#define RECORD_VOLUME_CTRL_LEFT_ADDR          (0x400E)

/* Record Volume Ctrl Right  - Registers (IC 1) */
#define RECORD_VOLUME_CTRL_RIGHT_ADDR         (0x400F)

/* Record Mic Bias Control  - Registers (IC 1) */
#define RECORD_MIC_BIAS_CONTROL_ADDR          (0x4010)

/* ALC Control 0  - Registers (IC 1) */
#define ALC_CONTROL_0_ADDR                    (0x4011)

/* ALC Control 1  - Registers (IC 1) */
#define ALC_CONTROL_1_ADDR                    (0x4012)

/* ALC Control 2  - Registers (IC 1) */
#define ALC_CONTROL_2_ADDR                    (0x4013)

/* ALC Control 3  - Registers (IC 1) */
#define ALC_CONTROL_3_ADDR                    (0x4014)

/* Serial Port Control 0  - Registers (IC 1) */
#define SERIAL_PORT_CONTROL_0_ADDR            (0x4015)

/* Serail Port Control 1  - Registers (IC 1) */
#define SERIAL_PORT_CONTROL_1_ADDR            (0x4016)

/* Converter Ctrl 0  - Registers (IC 1) */
#define CONVERTER_CTRL_0_ADDR                 (0x4017)

/* Converter Ctrl 1  - Registers (IC 1) */
#define CONVERTER_CTRL_1_ADDR                 (0x4018)

/* ADC Control 0  - Registers (IC 1) */
#define ADC_CONTROL_0_ADDR                    (0x4019)

/* ADC Control 1  - Registers (IC 1) */
#define ADC_CONTROL_1_ADDR                    (0x401A)

/* ADC Control 2  - Registers (IC 1) */
#define ADC_CONTROL_2_ADDR                    (0x401B)

/* Playback Mixer Left Control 0  - Registers (IC 1) */
#define PLAYBACK_MIXER_LEFT_CONTROL_0_ADDR    (0x401C)

/* Plaback Mixer Left Control 1  - Registers (IC 1) */
#define PLAYBACK_MIXER_LEFT_CONTROL_1_ADDR    (0x401D)

/* Plaback Mixer Right Control 0  - Registers (IC 1) */
#define PLAYBACK_MIXER_RIGHT_CONTROL_0_ADDR   (0x401E)

/* Playback Mixer Right Control 1  - Registers (IC 1) */
#define PLAYBACK_MIXER_RIGHT_CONTROL_1_ADDR   (0x401F)

/* Playback LR Left  - Registers (IC 1) */
#define PLAYBACK_LR_LEFT_ADDR                 (0x4020)

/* Playback LR Right  - Registers (IC 1) */
#define PLAYBACK_LR_RIGHT_ADDR                (0x4021)

/* Playback LR Mono Ctrl  - Registers (IC 1) */
#define PLAYBACK_LR_MONO_CTRL_ADDR            (0x4022)

/* Playback Headphone Left  - Registers (IC 1) */
#define PLAYBACK_HEADPHONE_LEFT_ADDR          (0x4023)

/* Playback Headphone Right  - Registers (IC 1) */
#define PLAYBACK_HEADPHONE_RIGHT_ADDR         (0x4024)

/* Playback Line Out Left  - Registers (IC 1) */
#define PLAYBACK_LINE_OUT_LEFT_ADDR           (0x4025)

/* Playback Line Out Right  - Registers (IC 1) */
#define PLAYBACK_LINE_OUT_RIGHT_ADDR          (0x4026)

/* Playback Line Out Mono  - Registers (IC 1) */
#define PLAYBACK_LINE_OUT_MONO_ADDR           (0x4027)

/* Playback Control  - Registers (IC 1) */
#define PLAYBACK_CONTROL_ADDR                 (0x4028)

/* Playback Power Management  - Registers (IC 1) */
#define PLAYBACK_POWER_MANAGEMENT_ADDR        (0x4029)

/* DAC Control 0  - Registers (IC 1) */
#define DAC_CONTROL_0_ADDR                    (0x402A)

/* DAC Control 1  - Registers (IC 1) */
#define DAC_CONTROL_1_ADDR                    (0x402B)

/* DAC Control 2  - Registers (IC 1) */
#define DAC_CONTROL_2_ADDR                    (0x402C)

/* Serial Port Pad Control 0  - Registers (IC 1) */
#define SERIAL_PORT_PAD_CONTROL_0_ADDR        (0x402D)

/* Comm Port Pad Ctrl 0  - Registers (IC 1) */
#define COMM_PORT_PAD_CTRL_0_ADDR             (0x402F)

/* Comm Port Pad Ctrl 1  - Registers (IC 1) */
#define COMM_PORT_PAD_CTRL_1_ADDR             (0x4030)

/* JackRegister  - Registers (IC 1) */
#define JACKREGISTER_ADDR                     (0x4031)

/* Dejitter Register Control  - Registers (IC 1) */
#define DEJITTER_REGISTER_CONTROL_ADDR        (0x4036)

/* CRC Ideal_1  - Registers (IC 1) */
#define CRC_IDEAL_1_ADDR                      (0x40C0)

/* CRC Ideal_2  - Registers (IC 1) */
#define CRC_IDEAL_2_ADDR                      (0x40C1)

/* CRC Ideal_3  - Registers (IC 1) */
#define CRC_IDEAL_3_ADDR                      (0x40C2)

/* CRC Ideal_4  - Registers (IC 1) */
#define CRC_IDEAL_4_ADDR                      (0x40C3)

/* CRC Enable  - Registers (IC 1) */
#define CRC_ENABLE_ADDR                       (0x40C4)

/* GPIO 0 Control  - Registers (IC 1) */
#define GPIO_0_CONTROL_ADDR                   (0x40C6)

/* GPIO 1 Control  - Registers (IC 1) */
#define GPIO_1_CONTROL_ADDR                   (0x40C7)

/* GPIO 2 Control  - Registers (IC 1) */
#define GPIO_2_CONTROL_ADDR                   (0x40C8)

/* GPIO 3 Control  - Registers (IC 1) */
#define GPIO_3_CONTROL_ADDR                   (0x40C9)

/* Watchdog_Enable  - Registers (IC 1) */
#define WATCHDOG_ENABLE_ADDR                  (0x40D0)

/* Watchdog Register Value 1  - Registers (IC 1) */
#define WATCHDOG_REGISTER_VALUE_1_ADDR        (0x40D1)

/* Watchdog Register Value 2  - Registers (IC 1) */
#define WATCHDOG_REGISTER_VALUE_2_ADDR        (0x40D2)

/* Watchdog Register Value 3  - Registers (IC 1) */
#define WATCHDOG_REGISTER_VALUE_3_ADDR        (0x40D3)

/* Watchdog Error  - Registers (IC 1) */
#define WATCHDOG_ERROR_ADDR                   (0x40D4)

/* Non Modulo RAM 1  - Registers (IC 1) */
#define NON_MODULO_RAM_1_ADDR                 (0x40E9)

/* Non Modulo RAM 2  - Registers (IC 1) */
#define NON_MODULO_RAM_2_ADDR                 (0x40EA)

/* Sample Rate Setting  - Registers (IC 1) */
#define SAMPLE_RATE_SETTING_ADDR              (0x40EB)

/* Routing Matrix Inputs  - Registers (IC 1) */
#define ROUTING_MATRIX_INPUTS_ADDR            (0x40F2)

/* Routing Matrix Outputs  - Registers (IC 1) */
#define ROUTING_MATRIX_OUTPUTS_ADDR           (0x40F3)

/* Serial Data/GPIO Pin Config  - Registers (IC 1) */
#define SERIAL_DATAGPIO_PIN_CONFIG_ADDR       (0x40F4)

/* DSP Enable Register  - Registers (IC 1) */
#define DSP_ENABLE_REGISTER_ADDR              (0x40F5)

/* DSP Run Register  - Registers (IC 1) */
#define DSP_RUN_REGISTER_ADDR                 (0x40F6)

/* DSP Slew Modes  - Registers (IC 1) */
#define DSP_SLEW_MODES_ADDR                   (0x40F7)

/* Serial Port Sample Rate Setting  - Registers (IC 1) */
#define SERIAL_PORT_SAMPLE_RATE_SETTING_ADDR  (0x40F8)

/* Clock Enable Reg 0  - Registers (IC 1) */
#define CLOCK_ENABLE_REG_0_ADDR               (0x40F9)

/* Clock Enable Reg 1  - Registers (IC 1) */
#define CLOCK_ENABLE_REG_1_ADDR               (0x40FA)

#endif
