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
#include "AdcDrv_pcm1862_priv.h"
#include "Pcm1862InitTab.h"
#include "bsp.h"


/***********************************************************
 * Public Function Implemenation
 ***********************************************************/
void AdcDrv_pcm1862_Ctor(cAdcDrv_pcm1862 * me, cI2CDrv *pI2cObj)
{
    me->pAudioAdcI2cObj = pI2cObj;
    me->deviceAddr =  pI2cObj->pConfig->devAddress;
    me->isCreated  = TRUE;
    I2CDrv_Ctor(pI2cObj, me->pAudioAdcI2cObj->pConfig);
    ASSERT(me->pAudioAdcI2cObj);
}

void AdcDrv_pcm1862_Xtor(cAdcDrv_pcm1862 * me)
{
    I2CDrv_Xtor(me->pAudioAdcI2cObj);
    me->isCreated  = FALSE;
}

bool AdcDrv_pcm1862_IsCreated(cAdcDrv_pcm1862 * me)
{
    return me->isCreated;
}

void AdcDrv_pcm1862_Init(cAdcDrv_pcm1862 * me)
{
    int i = 0;
    ASSERT(me && me->isCreated);
    cfg_reg *r = audioAdcInitTab;
    int n = sizeof(audioAdcInitTab)/sizeof(audioAdcInitTab[0]);
    while (i < n)
    {
        switch (r[i].command)
        {
            case CFG_META_SWITCH:
                // Used in legacy applications.  Ignored here.
                break;
            case CFG_META_DELAY:
                BSP_BlockingDelayMs(r[i].param);
                break;
            case CFG_META_BURST:
                pcm1862_I2cWrite(me, r[i].param, (unsigned char *)&r[i+1]);
                i += r[i].param/2 +1;
                break;
            default:
                pcm1862_I2cWrite(me, 2, (unsigned char *)&r[i]);
                break;
        }
        i++;
    }
    pcm1862_SwitchToActive(me, TRUE);
}

void AdcDrv_pcm1862_SetInput(cAdcDrv_pcm1862 * me, ePcm1862AnalogInput inputChannel)
{
    if(!me->isCreated)
    {
        return;
    }
    uint8 inputSetting[PCM1862_WRITE_LEN] = {PCM1862_ADC1L_ADDR, inputChannel};

    ASSERT(me->isCreated);
    pcm1862_SetRegisterPage(me, 0x00);
    /* set ADC1L */
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);

    /* set ADC1R */
    inputSetting[0] = PCM1862_ADC1R_ADDR;
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);
}


/*
* This is an emergency fix for MOFA auxin and RCA in issue.
* PGA: program gain amplify
* Note: before sending the request to change the gain, you need to update the value
* in setting server first
*/
void AdcDrv_pcm1862_SetPGA(cAdcDrv_pcm1862 * me, ePcm1862Gga programGain)
{
    if(!me->isCreated)
    {
        return;
    }
    uint8 data[PCM1862_WRITE_LEN] = {PCM1862_PGA_VAL_CH1_L_ADDR, PCM1862_PGA_MIN};

    ASSERT(me->isCreated);
    /* check if pgaValue is valid.*/
    ASSERT(!((programGain > PCM1862_PGA_MAX) && (programGain < PCM1862_PGA_MIN)));
    pcm1862_SetRegisterPage(me, 0x00);

    data[1] = programGain;
    /* set PGA_VAL_CH1_L */
    data[0] = PCM1862_PGA_VAL_CH1_L_ADDR;
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);

#ifndef PCM1862_AUTO_GAIN_MAPPING
    /* set PGA_VAL_CH1_R */
    data[0] = PCM1862_PGA_VAL_CH1_R_ADDR;
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);

    /* set PGA_VAL_CH2_L */
    data[0] = PCM1862_PGA_VAL_CH2_L_ADDR;
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);

    /* set PGA_VAL_CH2_R */
    data[0] = PCM1862_PGA_VAL_CH2_R_ADDR;
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);
#endif
}

void AdcDrv_pcm1862_SetMixerGain(cAdcDrv_pcm1862 * me, uint16 channel, uint8 programGain)
{
    ASSERT(me->isCreated);
    /* check if pgaValue is valid.*/
//    ASSERT(!((programGain > PCM1862_PGA_MAX) && (programGain < PCM1862_PGA_MIN)));
    pcm1862_coefsUpdating(me, channel, (uint8 *)PCM1862_MIXER_GAIN_TABLE[programGain]);
}

void AdcDrv_pcm1862_enableAGC(cAdcDrv_pcm1862 * me, bool enable)
{
    if(!me->isCreated)
    {
        return;
    }
    uint8 pgaCtrl[PCM1862_WRITE_LEN] = {PCM1862_PGA_CTRL_ADDR, PCM1862_PGA_CTRL_DISABLE_AGC};

    ASSERT(me->isCreated);
    pgaCtrl[1] = (enable)? PCM1862_PGA_CTRL_ENABLE_AGC : PCM1862_PGA_CTRL_DISABLE_AGC;
    pcm1862_SetRegisterPage(me, 0x00);
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)pgaCtrl);
}

void AdcDrv_pcm1862_enableMute(cAdcDrv_pcm1862 * me, bool enable)
{
    if(!me->isCreated)
    {
        return;
    }
    uint8 data[PCM1862_WRITE_LEN] = {PCM1862_DSP_CTRL_ADDR, PCM1862_CH1_MUTE};

    ASSERT(me->isCreated);
    data[1] = (enable)? PCM1862_CH1_MUTE : PCM1862_CH1_UNMUTE;
    pcm1862_SetRegisterPage(me, 0x00);
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);
}

void AdcDrv_pcm1862_dumpReg(cAdcDrv_pcm1862 *me)
{
    if(!me->isCreated)
    {
        return;
    }

    uint8 data= 0;

    pcm1862_I2cRead(me, 0x20, 1, &data);
    TP_PRINTF("PCM1862 REG[0x20]= 0x%02x\r\n", data);

    pcm1862_I2cRead(me, 0x70, 1, &data);
    TP_PRINTF("PCM1862 REG[0x70]= 0x%02x\r\n", data);

    pcm1862_I2cRead(me, 0x72, 1, &data);
    TP_PRINTF("PCM1862 REG[0x72]= 0x%02x\r\n", data);

    pcm1862_I2cRead(me, 0x74, 1, &data);
    TP_PRINTF("PCM1862 REG[0x74]= 0x%02x\r\n", data);

    pcm1862_I2cRead(me, 0x75, 1, &data);
    TP_PRINTF("PCM1862 REG[0x75]= 0x%02x\r\n", data);

    pcm1862_I2cRead(me, 0x78, 1, &data);
    TP_PRINTF("PCM1862 REG[0x78]= 0x%02x\r\n\r\n", data);
}

uint8 AdcDrv_pcm1862_GetMode(cAdcDrv_pcm1862 * me)
{
    if(!me->isCreated)
    {
        return 0;
    }

    uint8 result = 0;
    pcm1862_SetRegisterPage(me, 0x00);
    pcm1862_I2cRead(me, PCM1862_DEV_STAT, 1, &result);
    return result;
}

void AdcDrv_pcm1862_DSP2MemMap(cAdcDrv_pcm1862 * me, uint8 mode)
{
    if(!me->isCreated)
    {
        return;
    }

    uint8 inputSetting[PCM1862_WRITE_LEN] = {PCM1862_DSP2_MEM_MAP_ADDR, 0x00};
    pcm1862_SetRegisterPage(me, 0x01);
    inputSetting[1] = mode;
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);

}

void AdcDrv_pcm1862_SetEnergySenseMask(cAdcDrv_pcm1862 * me, uint8 mask)
{
    if(!me->isCreated)
    {
        return;
    }

    uint8 inputSetting[PCM1862_WRITE_LEN] = {PCM1862_SIGDET_TRIG_MASK, 0x00};
    pcm1862_SetRegisterPage(me, 0x00);
    inputSetting[1] = mask;
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);
}




/***********************************************************
 * Private Function Implemenation
 ***********************************************************/
static void pcm1862_I2cWrite(cAdcDrv_pcm1862 * me, uint8 bytes, const uint8 *data)
{
    ASSERT(me && me->isCreated);
    tI2CMsg i2cMsg=
    {
        .devAddr = me->deviceAddr,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)data
    };
    (I2CDrv_MasterWrite(me->pAudioAdcI2cObj, &i2cMsg));
}

static void pcm1862_I2cRead(cAdcDrv_pcm1862 * me, uint32 regAddr, uint16 bytes, const uint8 *data)
{
    ASSERT(me && me->isCreated);
    tI2CMsg i2cMsg =
    {
        .devAddr = me->deviceAddr,
        .regAddr = regAddr,
        .length  = bytes,
        .pMsg    = (uint8*)data
    };
    I2CDrv_MasterRead(me->pAudioAdcI2cObj, &i2cMsg);
}

/* if enable TRUE -> switch to active, otherwise to sleep to enable energysense */
void pcm1862_SwitchToActive(cAdcDrv_pcm1862 * me, bool enable)
{
    ASSERT(me && me->isCreated);
    uint8 inputSetting[PCM1862_WRITE_LEN] = {PCM1862_PWRDN_CTRL_ADDR, PCM1862_SET_ACTIVE_MODE};

    ASSERT(me->isCreated);
    pcm1862_SetRegisterPage(me, 0x00);

    if (!enable)
    {
        inputSetting[1] = PCM1862_SET_SLEEP_MODE;
    }
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);
}

void pcm1862_SetRegisterPage(cAdcDrv_pcm1862 * me, uint8 page)
{
    ASSERT(me && me->isCreated);
    uint8 tmp[2];
    tmp[0]=PAGE_SEL_REG;
    tmp[1]=page;
    pcm1862_I2cWrite(me, sizeof(tmp), (uint8*)(&tmp));
}

static bool pcm1862_coefsUpdating(cAdcDrv_pcm1862 * me, uint8 reg, uint8* pData)
{
    uint8 tmp[2];
    uint8 ret = 0;

    pcm1862_SetRegisterPage(me, 0x01);
    tmp[0] = 0x02;
    tmp[1] = reg;
    pcm1862_I2cWrite(me, sizeof(tmp), (uint8*)(&tmp));
    tmp[0] = 0x04;
    tmp[1] = pData[0];
    pcm1862_I2cWrite(me, sizeof(tmp), (uint8*)(&tmp));
    tmp[0] = 0x05;
    tmp[1] = pData[1];
    pcm1862_I2cWrite(me, sizeof(tmp), (uint8*)(&tmp));
    tmp[0] = 0x06;
    tmp[1] = pData[2];
    pcm1862_I2cWrite(me, sizeof(tmp), (uint8*)(&tmp));

    tmp[0] = 0x01;
    tmp[1] = 0x01;
    pcm1862_I2cWrite(me, sizeof(tmp), (uint8*)(&tmp));

    pcm1862_I2cRead(me, 0x01, 0x01, &ret);
    pcm1862_I2cRead(me, 0x01, 0x01, &ret);

    return (bool)ret;
}

void AdcDrv_pcm1862_mute_LeftChannel(cAdcDrv_pcm1862 * me, bool enable)
{
    if(!me->isCreated)
    {
        return;
    }
    uint8 data[PCM1862_WRITE_LEN] = {PCM1862_DSP_CTRL_ADDR, PCM1862_CH1_L_MUTE};

    ASSERT(me->isCreated);
    data[1] = (enable)? PCM1862_CH1_L_MUTE : PCM1862_CH1_UNMUTE;
    pcm1862_SetRegisterPage(me, 0x00);
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);
}

void AdcDrv_pcm1862_mute_RightChannel(cAdcDrv_pcm1862 * me, bool enable)
{
    if(!me->isCreated)
    {
        return;
    }
    uint8 data[PCM1862_WRITE_LEN] = {PCM1862_DSP_CTRL_ADDR, PCM1862_CH1_R_MUTE};

    ASSERT(me->isCreated);
    data[1] = (enable)? PCM1862_CH1_R_MUTE : PCM1862_CH1_UNMUTE;
    pcm1862_SetRegisterPage(me, 0x00);
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)data);
}

void AdcDrv_pcm1862_SetInput_L(cAdcDrv_pcm1862 * me, ePcm1862AnalogInput inputChannel)
{
    if(!me->isCreated)
    {
        return;
    }

    uint8 inputSetting[PCM1862_WRITE_LEN] = {PCM1862_ADC1L_ADDR, inputChannel};
    uint8 inputSetting1[PCM1862_WRITE_LEN] = {PCM1862_ADC1R_ADDR, 0x40};

    ASSERT(me->isCreated);
    pcm1862_SetRegisterPage(me, 0x00);
    /* set ADC1L */
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);

    /* set ADC1R */
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting1);
}
void AdcDrv_pcm1862_SetInput_R(cAdcDrv_pcm1862 * me, ePcm1862AnalogInput inputChannel)
{
    if(!me->isCreated)
    {
        return;
    }
    uint8 inputSetting1[PCM1862_WRITE_LEN] = {PCM1862_ADC1L_ADDR, 0x40};
    uint8 inputSetting[PCM1862_WRITE_LEN] = {PCM1862_ADC1L_ADDR, inputChannel};

    ASSERT(me->isCreated);
    pcm1862_SetRegisterPage(me, 0x00);
    /* set ADC1L */
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting1);

    /* set ADC1R */
    inputSetting[0] = PCM1862_ADC1R_ADDR;
    pcm1862_I2cWrite(me, PCM1862_WRITE_LEN, (unsigned char *)inputSetting);
}
