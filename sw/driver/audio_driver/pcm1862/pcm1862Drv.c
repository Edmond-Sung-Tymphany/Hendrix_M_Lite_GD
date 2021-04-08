/**
* @file pcm1862Drv.c
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 26-Mar-2015
* @copyright Tymphany Ltd.
*/
#include "product.config"
#include "trace.h"
#include "I2CDrv.h"
#include "AudioAdcDrv.h"
#include "Pcm1862InitTab.h"
#include "bsp.h"

#define CFG_META_SWITCH (255)
#define CFG_META_DELAY  (254)
#define CFG_META_BURST  (253)

#define PCM1862_WRITE_LEN       (2)

/* Register address */
#define PAGE_SEL_REG                (0)

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
#define PCM1862_PGA_0dB             (0)
#define PCM1862_PGA_ADJUST_STEP     (1) //adjust step is 0.5dB.
#define PCM1862_PGA_MIN             (0xe8) //-12.0dB
#define PCM1862_PGA_MAX             (0x50) //40.0dB

#define PCM1862_CH1_MUTE            (0x13) // Mute CH1_R&L
#define PCM1862_CH1_UNMUTE          (0x10) // Unmute CH1_R&L

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
#define PCM1862_PGA_CTRL_DISABLE_AGC  (0xfe) // disable Auto Gain Control.
#define PCM1862_PGA_CTRL_ENABLE_AGC   (0xff) // Eable Auto Gain Control.

#define PCM1862_SET_PGA_DELAY_20MS       (2000) //2ms
#define PCM1862_SET_PGA_DELAY_200MS       (200000) //200ms
static void AudioAdcDrv_Init(cAudioAdcDrv * me);

#define PCM1862_SET_SLEEP_MODE          (0x72)  // switch to sleep mode to eneble enerysense
#define PCM1862_SET_ACTIVE_MODE         (0x70)   // switch to active
#define PCM1862_CLEAR_INTERRUPTS        (0x00)
#define PCM1862_SET_ENEGYSENSE_INT      (0x01)


void AudioAdcDrv_I2cWrite(cAudioAdcDrv * me, uint8 bytes, const uint8 *data)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = me->deviceI2cAddr,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)data
    };
    I2CDrv_MasterWrite(me->pAudioAdcI2c, &i2cMsg);
}

static void AudioAdcDrv_I2cRead(cAudioAdcDrv * me, uint32 regAddr, uint16 bytes, const uint8 *data)
{
    tI2CMsg i2cMsg =
    {
        .devAddr = me->deviceI2cAddr,
        .regAddr = regAddr,
        .length  = bytes,
        .pMsg    = (uint8*)data
    };
    I2CDrv_MasterRead(me->pAudioAdcI2c, &i2cMsg);
}

void AudioAdcDrv_SetRegisterPage(cAudioAdcDrv * me, uint8 page)
{
    uint8 tmp[2];
    tmp[0]=PAGE_SEL_REG;
    tmp[1]=page;
    AudioAdcDrv_I2cWrite(me, sizeof(tmp), (uint8*)(&tmp));
}

void AudioAdcDrv_Ctor(cAudioAdcDrv * me)
{
    me->isCreated  = TRUE;
    I2CDrv_Ctor(me->pAudioAdcI2c, me->pI2CConfig);
    AudioAdcDrv_Init(me);
}

void AudioAdcDrv_Xtor(cAudioAdcDrv * me)
{
    I2CDrv_Xtor(me->pAudioAdcI2c);
    me->pI2CConfig = NULL;
    me->isCreated  = FALSE;
}

static bool AudioAdcDrv_IsCreated(cAudioAdcDrv * me)
{
    return me->isCreated;
}

static void AudioAdcDrv_Init(cAudioAdcDrv * me)
{
    int i = 0;
    cfg_reg *r = audioAdcInitTab;
    int n = sizeof(audioAdcInitTab)/sizeof(audioAdcInitTab[0]);
    while (i < n) {
        switch (r[i].command) {
        case CFG_META_SWITCH:
            // Used in legacy applications.  Ignored here.
            break;
        case CFG_META_DELAY:
            BSP_BlockingDelayUs(r[i].param * 1000);
            break;
        case CFG_META_BURST:
            AudioAdcDrv_I2cWrite(me, r[i].param, (unsigned char *)&r[i+1]);
            i += r[i].param/2 +1;
            break;
        default:
            AudioAdcDrv_I2cWrite(me, 2, (unsigned char *)&r[i]);
            break;
        }
        i++;
    }
}

void AudioAdcDrv_setInput(cAudioAdcDrv * me, uint8 input)
{
    uint8 inputSetting[PCM1862_WRITE_LEN] = {PCM1862_ADC1L_ADDR, PCM1862_ANALOG_NO_INPUT};

    if(!AudioAdcDrv_IsCreated(me))
    {
        return;
    }
    AudioAdcDrv_SetRegisterPage(me, 0x00);

    inputSetting[1] = input;
    /* set ADC1L */
    AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);

    /* set ADC1R */
    inputSetting[0] = PCM1862_ADC1R_ADDR;
    AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);
}

void AudioAdcDrv_setPGA(cAudioAdcDrv * me, uint8 pgaValue)
{
    uint8 data[PCM1862_WRITE_LEN] = {PCM1862_PGA_VAL_CH1_L_ADDR, PCM1862_PGA_MIN};

    if(!AudioAdcDrv_IsCreated(me))
    {
        return;
    }
    /* check if pgaValue is valid.*/
    ASSERT(!((pgaValue > PCM1862_PGA_MAX) && (pgaValue < PCM1862_PGA_MIN)));
    AudioAdcDrv_SetRegisterPage(me, 0x00);

    data[1] = pgaValue;
    /* set PGA_VAL_CH1_L */
    data[0] = PCM1862_PGA_VAL_CH1_L_ADDR;
    AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);

    /* set PGA_VAL_CH1_R */
    data[0] = PCM1862_PGA_VAL_CH1_R_ADDR;
    //AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);

    /* set PGA_VAL_CH2_L */
    data[0] = PCM1862_PGA_VAL_CH2_L_ADDR;
    //AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);

    /* set PGA_VAL_CH2_R */
    data[0] = PCM1862_PGA_VAL_CH2_R_ADDR;
    //AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);
}

void AudioAdcDrv_enableAGC(cAudioAdcDrv * me, bool enable)
{
    uint8 pgaCtrl[PCM1862_WRITE_LEN] = {PCM1862_PGA_CTRL_ADDR, PCM1862_PGA_CTRL_DISABLE_AGC};
    if(!AudioAdcDrv_IsCreated(me))
    {
        return;
    }
    pgaCtrl[1] = (enable)? PCM1862_PGA_CTRL_ENABLE_AGC : PCM1862_PGA_CTRL_DISABLE_AGC;
    AudioAdcDrv_SetRegisterPage(me, 0x00);
    AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)pgaCtrl);
}

void AudioAdcDrv_enableMute(cAudioAdcDrv * me, bool enable)
{
    uint8 data[PCM1862_WRITE_LEN] = {PCM1862_DSP_CTRL_ADDR, PCM1862_CH1_MUTE};

    if(!AudioAdcDrv_IsCreated(me))
    {
        return;
    }
    data[1] = (enable)? PCM1862_CH1_MUTE : PCM1862_CH1_UNMUTE;
    AudioAdcDrv_SetRegisterPage(me, 0x00);
    AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);
}

/* if enable TRUE -> switch to active, otherwise to sleep to enable energysense */
void AudioAdcDrv_SwitchToActive(cAudioAdcDrv * me, bool enable)
{
    uint8 inputSetting[PCM1862_WRITE_LEN] = {PCM1862_PWRDN_CTRL_ADDR, PCM1862_SET_ACTIVE_MODE};
    if(!AudioAdcDrv_IsCreated(me))
    {
        return;
    }
    AudioAdcDrv_SetRegisterPage(me, 0x00);

    if (!enable)
    {
        inputSetting[1] = PCM1862_SET_SLEEP_MODE;
    }
    AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);
}

uint8 AudioAdcDrv_GetMode(cAudioAdcDrv * me)
{
    uint8 result = 0;
    if(!AudioAdcDrv_IsCreated(me))
    {
        return 0;
    }
    AudioAdcDrv_SetRegisterPage(me, 0x00);

    AudioAdcDrv_I2cRead(me, PCM1862_DEV_STAT, 1, &result);

    return result;
}

#ifdef HAS_ENERGYSENSE_MODE
void AudioAdcDrv_EnergySenseDSPMemMapCoefs(cAudioAdcDrv * me, bool bIsTurnOnSigDetection)
{
    uint8 inputSetting[PCM1862_WRITE_LEN] = {PCM1862_DSP2_MEM_MAP_ADDR, 0x00};
    uint8 result = 0;
    uint8 i = 0;

    if(!AudioAdcDrv_IsCreated(me))
    {
        return;
    }
    AudioAdcDrv_SetRegisterPage(me, 0x01);
    if (bIsTurnOnSigDetection)
    { /* setup coefs for signal detection */
        for (i = 0; i < COEFS_NUM; i++)
        {
            inputSetting[0] = PCM1862_DSP_COEFS_REGS[i];
            inputSetting[1] = PCM1862_SIG_DETECT_COEFS[i];

            AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);
        }
    }
    else
    { /* setup coefs for signal loss */
        for (i = 0; i < COEFS_NUM; i++)
        {
            inputSetting[0] = PCM1862_DSP_COEFS_REGS[i];
            inputSetting[1] = PCM1862_SIG_LOSS_COEFS[i];

            AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);
        }
    }
        inputSetting[0] = 0x01;
        inputSetting[1] = 0x01;    // # execute write operation

    AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);

        AudioAdcDrv_I2cRead(me, 0x01, 1, &result);  // # Read to see if ready (dummy read)
        AudioAdcDrv_I2cRead(me, 0x01, 1, &result);  // # Read again to make sure that system is ready for next coeff.

}

void AudioAdcDrv_SetEnergySenseMask(cAudioAdcDrv * me, uint8 mask)
{
    uint8 inputSetting[PCM1862_WRITE_LEN] = {PCM1862_SIGDET_TRIG_MASK, 0x00};

    if(!AudioAdcDrv_IsCreated(me))
    {
        return;
    }
    AudioAdcDrv_SetRegisterPage(me, 0x00);

    inputSetting[1] = mask;
    AudioAdcDrv_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);
}
#endif

