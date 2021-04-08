/**
*  @file      DrvTAS5754Drv.c
*  @brief     This file implements the driver for the Audio DAC (with DSP) TAS5754.
*  @modified  Donald Leung/Edmond Sung
*  @date      04-2013
*  @copyright Tymphany Ltd.
*/

#include "product.config"
#include "trace.h"

#include "I2CDrv.h"
#include "bsp.h"
#include "DspDrv_tas5754.h"
#include "DspTAS5754Drv_priv.h"

#ifndef NULL
#define NULL                             (0)
#endif

#ifdef DEBUG_DSP
#define DSP_DEBUG(x) DEBUG(x)
#else
#define DSP_DEBUG(x)
#endif

#define DISABLE_EQ
/* Private Variables */
typedef struct
{
    union
    {
        struct
        {
            uint8       bit0 :1;
            uint8       bit1 :1;
            uint8       bit2 :1;
            uint8       bit3 :1;
            uint8       bit4 :1;
            uint8       bit5 :1;
            uint8       bit6 :1;
            uint8       bit7 :1;
        };
        uint8       regData;
    };
    uint8 regAdd;
} tDspReg;

void DspTas5754_Ctor(cDspTas5754* me, cI2CDrv *pI2cObj)
{
    me->i2cObj        = pI2cObj;
    me->max_vol = MAX_VOLUME;
    me->default_vol = DEFAULT_VOLUME;
    me->isCreated = TRUE;
    ASSERT(me->i2cObj);
    /* If the device type is I2C, create the DSP I2C object */
    I2CDrv_Ctor(me->i2cObj, me->i2cObj->pConfig);
}

void DspTas5754_Xtor(cDspTas5754* me)
{
    I2CDrv_Xtor(me->i2cObj);
}

void DspTas5754_Mute(cDspTas5754 *me, eAudioMuteType muteType, BOOL muteEnable)
{

}

void DspTas5754_Volume(cDspTas5754 *me, uint8 vol)
{
    TAS5754_ChannelVol(me, CHANNEL_BASS, vol);
    TAS5754_ChannelVol(me, CHANNEL_TREBLE, vol);
}

uint16  DspTas5754_Init(cDspTas5754* me)
{
    uint16 i;

    ASSERT(me->initData);
    for (i = 0; ; i += 2)
    {
        if ((me->initData[i]) == 0xff)
        {
            break;
        }
        DSP_Write(me, 2, &(me->initData[i]));
//        BSP_BlockingDelayMs(1);
    }
    me->initData = NULL;
    return TRUE;
}

void DspTas5754_EqTurning(cDspTas5754 *me, uint8 id, void *data)
{
    //static uint16 tmp1 =0,tmp2=0;
    // tmp1 =sizeof(EQ_TABLEA)/sizeof(EQ_TABLEA[0]);
    //tmp2=sizeof(EQ_TABLEA);
    switch (id)
    {
        case DSP_EQ_CTRL_PEQ1:
            TAS5754_SetEQData(me,(uint8 *)EQ_TABLEA,sizeof(EQ_TABLEA));
            break;

        case DSP_EQ_CTRL_PEQ2:
            TAS5754_SetEQData(me,(uint8 *)EQ_TABLEB,sizeof(EQ_TABLEB));
            break;
        default:
            break;
    }
}


bool DspTas5754_IsAuxin(cDspTas5754 *me)
{
    return true;
}

bool DspTas5754_HasMusicStream(cDspTas5754 *me)
{
    return TRUE;
}

static void DSP_Write(cDspTas5754 *me, uint8 bytes, const uint8 *data)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = me->i2cObj->pConfig->devAddress,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)data
    };
    I2CDrv_MasterWrite(me->i2cObj, &i2cMsg);
}

static void DSP_Read(cDspTas5754 *me, uint8 * bufptr, uint8 reg_add, uint16 bytes)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = me->i2cObj->pConfig->devAddress,
        .regAddr = reg_add,
        .length = bytes,
        .pMsg = bufptr
    };
    I2CDrv_MasterRead(me->i2cObj, &i2cMsg);
}


static void TAS5754_DRC_DefaultGain(cDspTas5754 *me)
{
    uint8 drc_gain_channelB[2] = {62, DRC_CHANNEL_B_GAIN};      //reg62, 5754 drc channel B gain
    uint8 drc_gain_channelA[2] = {61, DRC_CHANNEL_A_GAIN};      //reg62, 5754 drc channel A gain

    DSP_Write(me, 2,drc_gain_channelA);
    DSP_Write(me, 2,drc_gain_channelB);
}

static void TAS5754_ChannelVol(cDspTas5754 *me, eChannel ch, uint8 vol)
{
    if (CHANNEL_BASS == ch)
    {
        TAS5754_SetTurningData(me, BASS_CHANNEL_VOL_PAGE, BASS_CHANNEL_VOL_BASE_ADDR, VOLUME_TABLE[vol], sizeof(VOLUME_TABLE[vol]));
    }
    else if (CHANNEL_TREBLE == ch)
    {
        TAS5754_SetTurningData(me, TREBLE_CHANNEL_VOL_PAGE, TREBLE_CHANNEL_VOL_BASE_ADDR, VOLUME_TABLE[vol], sizeof(VOLUME_TABLE[vol]));
    }
    else {}
}

static bool TAS5754_SetTurningData(cDspTas5754 *me, uint8 page, uint8 reg, const uint8* pData, uint8 dataLen)
{
    uint8  data[2],i,j;
    bool ret;
    ASSERT(pData);
    //send data to current CRAM buffer
    set_page_reg(me, page);
    for (i = 0; i < dataLen; i ++)
    {
        if ((reg + i) <= PAGE_END_REG_ADDR)
        {
            j = 0;
            data[0] = reg + i;
            data[1] = pData[i];
            DSP_Write(me, sizeof(data),data);
        }
        else
        {
            set_page_reg(me, page + 1);         //support page writing cross one page
            data[0] = PAGE_BASE_REG_ADDR + j;
            data[1] = pData[i];
            DSP_Write(me, sizeof(data),data);
            j++;
        }
    }
    ret = TAS5754_AdaptiveFiltering(me);
    //send data to another CRAM buffer
    set_page_reg(me, page);
    for (i = 0; i < dataLen; i ++)
    {
        if ((reg + i) <= PAGE_END_REG_ADDR)
        {
            j = 0;
            data[0] = reg + i;
            data[1] = pData[i];
            DSP_Write(me, sizeof(data),data);
        }
        else
        {
            set_page_reg(me, page + 1);         //support page writing cross one page
            data[0] = PAGE_BASE_REG_ADDR + j;
            data[1] = pData[i];
            DSP_Write(me, sizeof(data),data);
            j++;
        }
    }
    return ret;
}

static bool TAS5754_AdaptiveFiltering(cDspTas5754 *me)
{
    tDspReg  CRAM = {0x00, 0x00};
    uint8 timerOutCounter = 0xff, data[2];

    //switch CRAM buffer to another
    set_page_reg(me, 0x2c);
    CRAM.bit0 = CRAM_SWITCH_REQUSTING;
    CRAM.bit2 = CRAM_MODE_ENABLED;
    CRAM.regAdd = CRAM_REG_ADDR;
    data[0] = CRAM.regAdd;
    data[1] = CRAM.regData;
    DSP_Write(me, sizeof(data),data);

    //make sure buffer switching done
    do
    {
        DSP_Read(me, &CRAM.regData,0x01,1);
        if (!timerOutCounter --)return FALSE;
    }
    while(CRAM.bit0 != CRAM_SWITCH_REQUST_DONE);
    return TRUE;
}

static void TAS5754_SoftMute(cDspTas5754 *me, BOOL mute_mode)
{
    TAS5754_SOFT_MUTE[1] = (mute_mode) ? 0x07 : 0x00;
    DSP_Write(me, 2, TAS5754_SOFT_MUTE);
}

static void TAS5754_HardMute(cDspTas5754 *me, BOOL mute_mode)
{
    TAS5754_HARD_MUTE[1] = (mute_mode) ? 0x0C : 0x80;
    DSP_Write(me, 2, TAS5754_HARD_MUTE);
}

static void set_page_reg(cDspTas5754 *me, uint8 page)
{
    uint8 data[] = {0x00,0x00};
    data[1] = page;
    DSP_Write(me, 2, data);
}

static bool TAS5754_SetEQData(cDspTas5754 *me, const uint8* pData, uint16 dataLen)
{
    uint8  data[2];
    uint16 i,j;
    bool ret;
    ASSERT(pData);
    //send data to current CRAM buffer
    for (i = 0; i < dataLen; i += 2)
    {
        data[0] = pData[i];
        data[1] = pData[i + 1];
        DSP_Write(me, sizeof(data),data);
    }
    ret = TAS5754_AdaptiveFiltering(me);
    return ret;
}

