/**
*  @file      DspTAS5754Drv_priv.h
*  @brief     This file declares the driver for the TAS5754.
*  @modified  Donald Leung/Edmond Sung
*  @date      04-2013
*  @copyright Tymphany Ltd.
*/

#ifndef DSPTAS5754DRV_PRIV_H
#define DSPTAS5754DRV_PRIV_H

#include "DspTunableTab.h"
#include "DspDrv.Conf"

#define TAS5754_ADDRESS_TW  (0x34)
#define TAS5754_ADDRESS_WF  (0x98)

#define CRAM_SWITCH_REQUSTING   (0x01)
#define CRAM_SWITCH_REQUST_DONE (0x00)
#define CRAM_MODE_ENABLED       (0x01)
#define CRAM_REG_PAGE           (0x2c)
#define CRAM_REG_ADDR           (0x01)
#define PAGE_BASE_REG_ADDR      (0x08)          //every page start writing from 0x08, and end 0x7f
#define PAGE_END_REG_ADDR       (0x7F)          //every page address from 0x08 to 0x7f


#define CLOCK_CONTROL_REG               0x00
#define DEVICE_ID_REG                   0x01
#define ERROR_STATUS_REG                0x02
#define SYSTEM_CONTROL_REG1                 0x03
#define SERIAL_DATA_INTERFACE_REG                       0x04
#define SYSTEM_CONTROL_REG2                 0x05
#define SOFT_MUTE_REG                   0x06
#define MASTER_VOLUME_REG               0x07
#define CHANNEL_1_REG                   0x08
#define CHANNEL_2_REG                   0x09
#define CHANNEL_3_REG                   0x0A
#define VOLUME_CONFIGURATION_REG                        0x0E
#define MODULATION_LIMIT_REG                            0x10
#define PWM_SHUTDOWN_GROUP_REG                          0x19
#define START_STOP_PERIOD_REG                           0x1A
#define OSC_TRIM_REG                    0x1B
#define BKND_ERR_REG                                    0x1C
#define INPUT_MULTIPLEXER_REG                           0x20
#define CHANNEL4_SOURCE_SELECT_REG                  0x21
#define PWM_OUTPUT_MIX_REG                  0x25
#define DRC_CONTROL_REG                                 0x46
#define BANK_SWITCH_AND_EQ_CONTROL_REG                  0x50

#define TAS5754_SET_I2C_DELAY_MS                        1     //time in us

typedef enum
{
    CHANNEL_BASS,
    CHANNEL_TREBLE
} eChannel;

typedef enum
{
    EQ_Mode_A =1,
    EQ_Mode_B,
} eEQmode;


uint8 TAS5754_HARD_MUTE[] = {0x05, 0x00};
uint8 TAS5754_SOFT_MUTE[] = {0x06, 0x80};

uint8 TAS5754_MASTER_VOL[] = {MASTER_VOLUME_REG,0xFF};

static void set_page_reg(cDspTas5754 *me, uint8 page);
static void DSP_Write(cDspTas5754 *me, uint8 bytes, const uint8 *data);
static void DSP_Read(cDspTas5754 *me, uint8 * bufptr, uint8 reg_add, uint16 bytes);
static void TAS5754_SoftMute(cDspTas5754 *me, BOOL mute_mode);
static void TAS5754_HardMute(cDspTas5754 *me, BOOL mute_mode);
static void TAS5754_DRC_DefaultGain(cDspTas5754 *me);
static void DSPDrv_InitI2C(void *p);
static void TAS5754_Init(void *p);
static bool TAS5754_AdaptiveFiltering(cDspTas5754 *me);
static void TAS5754_ChannelVol(cDspTas5754 *me, eChannel ch, uint8 vol);
static bool TAS5754_SetTurningData(cDspTas5754 *me, uint8 page, uint8 reg, const uint8* pData, uint8 dataLen);
static bool TAS5754_SetEQData(cDspTas5754 *me, const uint8* pData, uint16 dataLen);
#endif

