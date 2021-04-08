/**
*  @file      dsp_TLV320AIC3254_driver.h
*  @brief     This file declares the driver for the Audio DSP TLV320AIC3254.
*  @author    Daniel Duan/Edmond
*  @date      04-2013
*  @copyright Tymphany Ltd.
*/


#ifndef DSP_TLV320AIC3254_DRV_H
#define DSP_TLV320AIC3254_DRV_H
#include "SettingSrv.h"
typedef struct tCtrIdEQIdMap
{
    eDspSettId dspSettid;
    eSettingId settingId;  /* setting database index */
}tCtrIdEQIdMap;

typedef struct
{
    const uint8* head;
    uint8        type;
    uint8        id;
    uint8        page;
    uint8        reg;
} tTunableInfo;

/* DSP Status bit masks */
typedef enum
{
    DSP_INIT_NONE = 0,
    DSP_INIT_SEC0_COMPLETED = (1 << 0),
    DSP_INIT_SEC1_COMPLETED = (1 << 1),
    DSP_INIT_SEC2_COMPLETED = (1 << 2),
    DSP_INIT_SEC3_COMPLETED = (1 << 3)
} eDspStatus;


/* CMD structure */
typedef struct
{
    uint8 address;
    uint8 value;
} tDspData;

typedef enum
{
    TLV320AIC_BUFFER_A,
    TLV320AIC_BUFFER_D    
}eTlv320ac3254BufferRegion;

typedef enum
{
    TLV320AC3254_AUDIO_INTERFACE_PRIMARY,
    TLV320AC3254_AUDIO_INTERFACE_SECONDARY
}eTlv320ac3254AudioInterface;

typedef enum
{
    TLV320AC3254_CLK_MCLK,
    TLV320AC3254_CLK_BCLK,
    TLV320AC3254_CLK_GPIO,
    TLV320AC3254_CLK_MPF1
}eTlv320ac3254ClockSource;

typedef enum
{
    TLV320AC3254_IN_NONE,
    TLV320AC3254_IN1,
    TLV320AC3254_IN2,
    TLV320AC3254_IN3,
    TLV320AC3254_IN1_IN2_10K,
    TLV320AC3254_IN1_IN2_20K,
}eTlv320ac3254AnalogInputChannel;




#define NUM_OF_DSP_INIT_SEQUENCE        8
//#define I2cTransfer(addr, data, size, a, b) IIC_communication_TX(addr, size, data)


/* Register address */
#define PAGE_SEL_REG                                       (  0)

/* Page0 */
#define SW_RST_REG                                         (  1)
#define CLK_SET1_REG                                       (  4)
#define CLK_SET2_REG                                       (  5)
#define CLK_SET3_REG                                       (  6)
#define CLK_SET4_REG                                       (  7)
#define CLK_SET5_REG                                       (  8)
#define CLK_SET6_REG                                       ( 11)
#define CLK_SET7_REG                                       ( 12)
#define DAC_OSR_SET1_REG                                   ( 13)
#define DAC_OSR_SET2_REG                                   ( 14)
#define DAC_MINIDSP_INST_CTRL1_REG                         ( 15)
#define DAC_MINIDSP_INST_CTRL2_REG                         ( 16)
#define DAC_MINIDSP_INTPL_FAC_SET_REG                      ( 17)
#define CLK_SET8_REG                                       ( 18)
#define CLK_SET9_REG                                       ( 19)
#define ADC_OVSAMP_REG                                     ( 20)
#define ADC_MINIDSP_INST_CTRL1_REG                         ( 21)
#define ADC_MINIDSP_INST_CTRL2_REG                         ( 22)
#define ADC_MINIDSP_DECM_FAC_SET_REG                       ( 23)
#define CLK_SET9_MUX_REG                                   ( 25)
#define CLK_SET10_REG                                      ( 26)
#define AUD_ITF_SET1_REG                                   ( 27)
#define AUD_ITF_SET2_REG                                   ( 28)
#define AUD_ITF_SET3_REG                                   ( 29)
#define CLK_SET11_REG                                      ( 30)
#define AUD_ITF_SET4_REG                                   ( 31)
#define AUD_ITF_SET5_REG                                   ( 32)
#define AUD_ITF_SET6_REG                                   ( 33)
#define DIG_ITF_MISC_SET_REG                               ( 34)
#define ADC_FLG_REG                                        ( 36)
#define DAC_FLG1_REG                                       ( 37)
#define DAC_FLG2_REG                                       ( 38)
#define STK_FLG1_REG                                       ( 42)
#define INT_FLG1_REG                                       ( 43)
#define STK_FLG2_REG                                       ( 44)
#define STK_FLG3_REG                                       ( 45)
#define INT_FLG2_REG                                       ( 46)
#define INT_FLG3_REG                                       ( 47)
#define INT1_CTRL_REG                                      ( 48)
#define INT2_CTRL_REG                                      ( 49)
#define MFP5_FUNC_CTRL_REG                                 ( 52)
/* Reg52 Bit Masks */
#define MFP5_VAL                                           (1 << 0)
#define MFP2_FUNC_CTRL_REG                                 ( 53)
#define MFP1_FUNC_CTRL_REG                                 ( 54)
/* Reg54 Bit Masks */
#define MFP1_VAL                                           (1 << 0)

#define MFP4_FUNC_CTRL_REG                                 ( 55)
/* Reg55 Bit Masks */
#define MFP4_VAL                                           (1 << 0)

#define MFP3_FUNC_CTRL_REG                                 ( 56)
/* Reg56 Bit Masks */
#define MFP3_VAL                                           (1 << 0)

#define DAC_SIG_PRCB_CTRL_REG                              ( 60)
#define ADC_SIG_PRCB_CTRL_REG                              ( 61)
#define ADDA_MINIDSP_CFG_REG                               ( 62)
#define DAC_CH_SET1_REG                                    ( 63)
#define DAC_CH_SET2_REG                                    ( 64)
/* Reg64 Bit Masks */
#define LDAC_CH_MUTE                                       (1 << 3)
#define RDAC_CH_MUTE                                       (1 << 2)

#define LDAC_CH_DIG_VOL_CTRL_REG                           ( 65)
#define RDAC_CH_DIG_VOL_CTRL_REG                           ( 66)
#define HEADSET_DET_CFG_REG                                ( 67)
#define DRC_CTRL1_REG                                      ( 68)
#define DRC_CTRL2_REG                                      ( 69)
#define DRC_CTRL3_REG                                      ( 70)
#define BEEP_GEN1_REG                                      ( 71)
#define BEEP_GEN2_REG                                      ( 72)
#define BEEP_GEN3_REG                                      ( 73)
#define BEEP_GEN4_REG                                      ( 74)
#define BEEP_GEN5_REG                                      ( 75)
#define BEEP_GEN6_REG                                      ( 76)
#define BEEP_GEN7_REG                                      ( 77)
#define BEEP_GEN8_REG                                      ( 78)
#define BEEP_GEN9_REG                                      ( 79)
#define ADC_CH_SET_REG                                     ( 81)
#define ADC_FINE_GAIN_ADJ_REG                              ( 82)
#define LADC_CH_VOL_CTRL_REG                               ( 83)
#define RADC_CH_VOL_CTRL_REG                               ( 84)
#define ADC_PHASE_ADJ_REG                                  ( 85)
#define LCH_AGC_CTRL1_REG                                  ( 86)
#define LCH_AGC_CTRL2_REG                                  ( 87)
#define LCH_AGC_CTRL3_REG                                  ( 88)
#define LCH_AGC_CTRL4_REG                                  ( 89)
#define LCH_AGC_CTRL5_REG                                  ( 90)
#define LCH_AGC_CTRL6_REG                                  ( 91)
#define LCH_AGC_CTRL7_REG                                  ( 92)
#define LCH_AGC_CTRL8_REG                                  ( 93)
#define RCH_AGC_CTRL1_REG                                  ( 94)
#define RCH_AGC_CTRL2_REG                                  ( 95)
#define RCH_AGC_CTRL3_REG                                  ( 96)
#define RCH_AGC_CTRL4_REG                                  ( 97)
#define RCH_AGC_CTRL5_REG                                  ( 98)
#define RCH_AGC_CTRL6_REG                                  ( 99)
#define RCH_AGC_CTRL7_REG                                  (100)
#define RCH_AGC_CTRL8_REG                                  (101)
#define DC_MEAS1_REG                                       (102)
#define DC_MEAS2_REG                                       (103)
#define LCH_DC_MEAS_OUT1_REG                               (104)
#define LCH_DC_MEAS_OUT2_REG                               (105)
#define LCH_DC_MEAS_OUT3_REG                               (106)
#define RCH_DC_MEAS_OUT1_REG                               (107)
#define RCH_DC_MEAS_OUT2_REG                               (108)
#define RCH_DC_MEAS_OUT3_REG                               (109)

/* Page1 registers */
#define POWER_CFG_REG                                      (  1)
#define LDO_CFG_REG                                        (  2)
#define PLAYBACK_CFG1_REG                                  (  3)
#define PLAYBACK_CFG2_REG                                  (  4)
#define OUT_DRV_POW_CTRL_REG                               (  9)
#define COMM_MODE_CTRL_REG                                 ( 10)
#define OVC_PROT_CFG_REG                                   ( 11)
#define HPL_ROUT_SEL_REG                                   ( 12)
#define HPR_ROUT_SEL_REG                                   ( 13)
#define LOL_ROUT_SEL_REG                                   ( 14)
#define LOR_ROUT_SEL_REG                                   ( 15)
#define HPL_DRV_GAIN_SET_REG                               ( 16)
/* Page1 Reg16 Bit Masks */
#define HPL_DRV_MUTE                                       (1 << 6)

#define HPR_DRV_GAIN_SET_REG                               ( 17)
/* Page1 Reg17 Bit Masks */
#define HPR_DRV_MUTE                                       (1 << 6)

#define LOL_DRV_GAIN_SET_REG                               ( 18)
/* Page1 Reg18 Bit Maks */
#define LOL_DRV_MUTE                                       (1 << 6)

#define LOR_DRV_GAIN_SET_REG                               ( 19)
/* Page1 Reg19 Bit Masks */
#define LOR_DRV_MUTE                                       (1 << 6)

#define HP_DRV_STA_CTRL_REG                                ( 20)
#define IN1L_TO_HPL_VOL_CTRL_REG                           ( 22)
#define IN1R_TO_HPR_VOL_CTRL_REG                           ( 23)
#define MAL_VOL_CTRL_REG                                   ( 24)
#define MAR_VOL_CTRL_REG                                   ( 25)
#define MICBIAS_CFG_REG                                    ( 51)
#define LMICPGA_PTERM_IN_ROUT_CFG_REG                      ( 52)
#define LMICPGA_NTERM_IN_ROUT_CFG_REG                      ( 54)
#define RMICPGA_PTERM_IN_ROUT_CFG_REG                      ( 55)
#define RMICPGA_NTERM_IN_ROUT_CFG_REG                      ( 57)
#define FLOAT_IN_CFG_REG                                   ( 58)
#define LMICPGA_VOL_CTRL_REG                               ( 59)
#define RMICPGA_VOL_CTRL_REG                               ( 60)
#define ADC_POW_TUNE_CFG_REG                               ( 61)
#define ADC_ANG_VOL_CTRL_FLG_REG                           ( 62)
#define DAC_ANG_GAIN_CTRL_FLG_REG                          ( 63)
#define ANG_IN_QCHG_CFG_REG                                ( 71)
#define REF_POWERUP_CFG_REG                                (123)



/* Page8 registers */
#define ADC_ADP_FILT_CFG_REG                               (  1)

/* Bit masks */
#define ADC_ADPT_FILT_BUFFER_SWITCH                        (1 << 0)

/* Page9 and Page27 */
#define ADC_VOL_REG                                        (104)

/* Page44 registers */
#define DAC_ADP_FILT_CFG_REG                               (1)

/* Private Macros */
#define DSP_INIT_CMD_SEC0_LEN                              ( 2)
#define DSP_INIT_CMD_SEC1_LEN                              ( 6)
#define DSP_INIT_CMD_SEC2_LEN                              (76)


/* Below API will be used within "dsp_TLV320AIC3254_driver.c" and "dsp_TLV320AIC3254_customer_driver.c" */

/**
 * Set the register page
 * @param     page - register page number
 */
void DSPDrv_SetRegisterPage(cDSPDrv *me, uint8 page);

/**
 * Swap the Adaptor Filter between A and D
 * @param     buf - BUFFER_A or BUFFER_D
 */
void DSPDrv_SwapAdpFilt(cDSPDrv *me, eTlv320ac3254BufferRegion buf);

void DSPDrv_I2cWrite(uint8 device_add, uint8 bytes, const uint8 *data);

void DSPDrv_I2cRead(uint8 * bufptr, uint8 device_add, uint8 reg_add, uint16 bytes);




#endif /* DSP_TLV320AIC3254_DRV_H */
