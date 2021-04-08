/**
 * @file        AdcDrv_pcm1862_priv.h
 * @brief       The audio adc driver interfaces and implementation
 * @author      Bob.Xu 
 * @date        2015-07-8
 * @copyright   Tymphany Ltd.
 */
#ifndef ADCDRV_PCM1862_PRIVATE_H
#define ADCDRV_PCM1862_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pcm1862Drv.h"
#include "setting_id.h"



/***********************************************************
 * Feature
 ***********************************************************/
/* When enable Auto Gain Mapping, register 0x01 means global gain control
 * When disable, we need to adjust many register
 */
#define PCM1862_AUTO_GAIN_MAPPING
  
  
/***********************************************************
 * Definition
 ***********************************************************/  
#define CFG_META_SWITCH (255)
#define CFG_META_DELAY  (254)
#define CFG_META_BURST  (253)

#define PCM1862_WRITE_LEN    (2)

/* Register address */
#define PAGE_SEL_REG         (0)

/* Page0 */
#define PCM1862_PGA_VAL_CH1_L_ADDR  (0x01)
#define PCM1862_PGA_VAL_CH1_R_ADDR  (0x02)
#define PCM1862_PGA_VAL_CH2_L_ADDR  (0x03)
#define PCM1862_PGA_VAL_CH2_R_ADDR  (0x04)
#define PCM1862_PGA_CTRL_ADDR       (0x05)
#define PCM1862_ADC1L_ADDR          (0x06)
#define PCM1862_ADC1R_ADDR          (0x07)
#define PCM1862_SIGDET_TRIG_MASK    (0x31)
#define PCM1862_SIGDET_LOSS_TIME    (0x34)
#define PCM1862_INT_EN              (0x60)
#define PCM1862_INT_STAT            (0x61)
#define PCM1862_INT_PLS             (0x62)
#define PCM1862_PWRDN_CTRL_ADDR     (0x70)
#define PCM1862_DSP_CTRL_ADDR       (0x71)
#define PCM1862_DEV_STAT            (0x72)

/* Page1 */
#define PCM1862_DSP2_MEM_MAP_ADDR   (0x01)
   
   
#define PCM1862_PGA_ADJUST_STEP     (1) //adjust step is 0.5dB.
/*
 PGA Value Channel 1 Left/Right
: Global Channel gain for ADC1L/ADC1R. (Analog + Digital). Analog gain only, if manual gain mapping is enabled.
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
#define PCM1862_PGA_0dB                 (0)
#define PCM1862_PGA_ADJUST_STEP         (1) //adjust step is 0.5dB.
#define PCM1862_PGA_MIN                 (0xe8) //-12.0dB
#define PCM1862_PGA_MAX                 (0x50) //40.0dB

#define PCM1862_CH1_MUTE                (0x13) // Mute CH1_R&L
#define PCM1862_CH1_UNMUTE           (0x10) // Unmute CH1_R&L
#define PCM1862_CH1_L_MUTE            (0x11) // Mute CH1_L
#define PCM1862_CH1_R_MUTE            (0x12) // Mute CH1_R

/*  The default value of PGA control register: Page 0 / Register 5 (Hex 0x05)
 *  Please refer to pcm1862 Spec.
 *  b7: SMOOTH
 *  b6: LINK
 *  b5: DPGA_CLIP_EN
 *  b4: MAX_ATT1
 *  b3: MAX_ATT0
 *  b2: START_ATT1
 *  b1: START_ATT0
 *  b0: AGC_EN */
#define PCM1862_PGA_CTRL_DISABLE_AGC    (0xfe) // disable Auto Gain Control.
#define PCM1862_PGA_CTRL_ENABLE_AGC     (0xff) // Eable Auto Gain Control.

#define PCM1862_SET_PGA_DELAY_20MS      (2000) //2ms
#define PCM1862_SET_PGA_DELAY_200MS     (200000) //200ms
#define PCM1862_SET_I2C_DELAY_US        (100)
 
#define PCM1862_SET_SLEEP_MODE        (0x72)  // switch to sleep mode to eneble enerysense
#define PCM1862_SET_ACTIVE_MODE       (0x70)  // switch to active
#define PCM1862_CLEAR_INTERRUPTS      (0x00)
#define PCM1862_SET_ENEGYSENSE_INT    (0x11)



static void pcm1862_SwitchToActive(cAdcDrv_pcm1862 * me, bool enable);
static void pcm1862_I2cWrite(cAdcDrv_pcm1862 * me, uint8 bytes, const uint8 *data);
static void pcm1862_I2cRead(cAdcDrv_pcm1862 * me, uint32 regAddr, uint16 bytes, const uint8 *data);
static bool pcm1862_IsCreated(cAdcDrv_pcm1862 * me);
static void pcm1862_SetRegisterPage(cAdcDrv_pcm1862 * me, uint8 page);
static bool pcm1862_coefsUpdating(cAdcDrv_pcm1862 * me, uint8 reg, uint8* pData);

#ifdef __cplusplus
}
#endif

#endif /* ADCDRV_PCM1862_PRIVATE_H */