 /**
*  @file      DrvTAS5711Drv.c
*  @brief     This file implements the driver for the Audio DAC (with DSP) TAS5711.
*  @modified  Donald Leung/Edmond Sung
*  @date      04-2013
*  @copyright Tymphany Ltd.
*/

#include "product.config"
#include "trace.h"
#include "DspDrv.h"
#include "DspTAS5711Drv_priv.h"

#include "I2CDrv.h"
#include "signals.h"
#include "bsp.h"

#ifndef NULL
#define NULL                             (0)
#endif

#ifdef DEBUG_DSP
#define DSP_DEBUG(x) DEBUG(x)
#else
#define DSP_DEBUG(x)
#endif

/* Private Variables */

cI2CDrv dspI2c;

static bool bIsHardUnmute = TRUE;

static tCtrIdEQIdMap ctrIdEQIdMap[] =
{
    /* DSP setting ID  index of setting db*/
    {DSP_VOLUME_SETT_ID,  SETID_VOLUME},
    {DSP_BASS_SETT_ID,    SETID_BASS},
    {DSP_TREBLE_SETT_ID,  SETID_TREBLE},
};

static void DSPDrv_InitI2C(void *p)
{
    cDSPDrv *me = (cDSPDrv*)p;
    ASSERT(me->pI2CConfig);
    /* If the device type is I2C, create the DSP I2C object */
    if (me->pI2CConfig->deviceInfo.deviceType==I2C_DEV_TYPE)
        I2CDrv_Ctor(&dspI2c, me->pI2CConfig);
}

static void DSPDrv_InitTas5711(void* p)
{
    TAS5711_Init();
    bIsHardUnmute = TRUE;
}

void DSPDrv_I2cWrite(uint8 device_add, uint8 bytes, const uint8 *data)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = device_add,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)data
    };
    I2CDrv_MasterWrite(&dspI2c, &i2cMsg);
}

void DSPDrv_I2cRead(uint8 * bufptr, uint8 device_add, uint8 reg_add, uint16 bytes)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = device_add,
        .regAddr = reg_add,
        .length = bytes,
        .pMsg = bufptr
    };
    I2CDrv_MasterRead(&dspI2c, &i2cMsg);
}

uint16 DSPDrv_Init(cDSPDrv* me)
{
    uint16 delaytime;

    ASSERT(me &&(me->pInitTable));
    me->pInitTable[me->initPhase].initSectionFunc(me);
    delaytime=me->pInitTable[me->initPhase].delaytime;
    me->initPhase++;
    if (me->initPhase == me->sectionSize)
    {
        me->initPhase = 0;
        delaytime = 0;
    }
    return delaytime;
}

static tDspInitSection DspInitSection[]=
{
    {&DSPDrv_InitI2C, 1},
    {&DSPDrv_InitTas5711, 1},
};

void DSPDrv_Ctor(cDSPDrv* me)
{
    tI2CDevice * conf = (tI2CDevice *) getDevicebyId(DSP_DEV_ID, NULL);
    ASSERT(conf && (conf->address));
    me->pInitTable = DspInitSection;
    me->sectionSize = ArraySize(DspInitSection);
    me->initPhase = 0;
    me->max_vol = MAX_VOLUME;
    me->default_vol = DEFAULT_VOLUME;
    me->pI2CConfig = conf;
    me->isCreated = TRUE;
}

void DSPDrv_Xtor(cDSPDrv* me)
{
    me->pInitTable = NULL;
    me->sectionSize = 0;
    me->initPhase = 0;
    I2CDrv_Xtor(&dspI2c);
}
BOOL DSPDrv_HasMusicStream(cDSPDrv *me)
{
    return TRUE;
}
void DSPDrv_SetVol(cDSPDrv *me, uint8 vol)
{
    TAS5711_Volume(vol);
}
void DSPDrv_SetAudio(cDSPDrv *me, eDspSettId dspSettId, BOOL enable)
{
    uint8 ii;
    for(ii = 0; ii < ArraySize(ctrIdEQIdMap); ii++)
    {
        if(dspSettId == ctrIdEQIdMap[ii].dspSettid)
        {
            break;
        }
    }
    ASSERT(dspSettId < DSP_SETT_ID_MAX);
    switch(dspSettId)
    {
        case DSP_VOLUME_SETT_ID:
        {
            if(enable)
            {
                int8 volumeLevel = 0;
                volumeLevel =  *(int8*)Setting_Get(ctrIdEQIdMap[ii].settingId);
                if(volumeLevel <= MAX_VOLUME && volumeLevel >= MIN_VOLUME)
                {
                    TAS5711_Volume(volumeLevel);
                }
            }
            else
            {
                TAS5711_Volume(0);
            }
            break;
        }
        case DSP_BASS_SETT_ID:
        {
            if(enable)
            {
                int8 bassLevel = 4;
                bassLevel =  *(int8*)Setting_Get(ctrIdEQIdMap[ii].settingId);
                if(bassLevel <= MAX_BASS && bassLevel >= MIN_BASS)
                {
                    TAS5711_Bass(bassLevel);
                }
            }
            else
            {
                TAS5711_Bass(4);
            }
            break;
        }
        case DSP_TREBLE_SETT_ID:
        {
            if(enable)
            {
                int8 trebleLevel = 4;
                trebleLevel =  *(int8*)Setting_Get(ctrIdEQIdMap[ii].settingId);
                if(trebleLevel <= MAX_TREBLE && trebleLevel >= MIN_TREBLE)
                {
                    TAS5711_Treble(trebleLevel);
                }
            }
            else
            {
                TAS5711_Treble(4);
            }
            break;
        }
        default:
            break;
    }
}
void DSPDrv_MuteDACOut(cDSPDrv *me)
{
    TAS5711_SoftMute(TRUE);
}
void DSPDrv_UnMuteDACOut(cDSPDrv *me)
{
    if (bIsHardUnmute)
    {
        bIsHardUnmute = FALSE;
        TAS5711_HardMute(FALSE);
    }
    TAS5711_SoftMute(FALSE);
}
void DSPDrv_set_Input(cDSPDrv *me, eAudioCtrlDriverInput input)
{
}
int DSPDrv_setStereoMux(cDSPDrv *me, eDspMuxChannel ch)
{
    return 0;
}
bool DSPDrv_IsAuxin(cDSPDrv *me)
{
    return true;
}

void TAS5711_SoftMute(BOOL mute_mode)
{
    TAS5711_SOFT_MUTE[1] = (mute_mode) ? 0x07 : 0x00;
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 2, TAS5711_SOFT_MUTE);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 2, TAS5711_SOFT_MUTE);
}

void TAS5711_HardMute(BOOL mute_mode)
{
    TAS5711_HARD_MUTE[1] = (mute_mode) ? 0x48 : 0x08;
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 2, TAS5711_HARD_MUTE);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 2, TAS5711_HARD_MUTE);
}

void TAS5711_EQSwitch(BOOL eq_on)
{
    if(eq_on)
    {
        DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,5, TAS5711_50_EQ_ON);
        DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,5, TAS5711_50_EQ_ON);
    }
    else
    {
        DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,5, TAS5711_50_EQ_OFF);
        DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,5, TAS5711_50_EQ_OFF);
    }
}

void TAS5711_Bass(int8 bass_level)
{
    int8 temp_index = bass_level + MAX_BASS;
    ASSERT(temp_index <= 8);

    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, &EQ_LOW_29_BASS[temp_index][0]);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, &EQ_LOW_30_BASS[temp_index][0]);
}

void TAS5711_Treble(int8 treble_level)
{
    int8 temp_index = treble_level + MAX_TREBLE;
    ASSERT(temp_index <= 8);

    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, &EQ_HIGH_29_TREBLE[temp_index][0]);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, &EQ_HIGH_30_TREBLE[temp_index][0]);
}

void TAS5711_Volume(uint8 volume)
{
    ASSERT(volume <= 29);

    TAS5711_MASTER_VOL[1] = VOLUME_TABLE[volume];
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 2,TAS5711_MASTER_VOL);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 2,TAS5711_MASTER_VOL);
}

void TAS5711_SetGainTwitters(uint8 gain)
{
    TAS5711_GAIN_TW[1]=gain;
    TAS5711_GAIN_TW[2]=gain;
    TAS5711_GAIN_TW[3]=0xFF;

    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 4, TAS5711_GAIN_TW);
}

void TAS5711_SetGainWoofer(uint8 gain)
{
    TAS5711_GAIN_WF[1]=gain;
    TAS5711_GAIN_WF[2]=gain;
    TAS5711_GAIN_WF[3]=0xFF;

    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 4, TAS5711_GAIN_WF);
}

void TAS5711_Load_EQ()
{
    // EQ for twitters
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_29);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_2A);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_2B);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_2C);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_2D);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_2E);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_2F);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_58);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_59);

    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_30);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_31);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_32);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_33);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_34);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_35);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_36);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_5C);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 21, EQ_HIGH_5D);

    // EQ for woofer
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_29);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_2A);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_2B);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_2C);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_2D);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_2E);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_2F);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_58);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_59);

    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_30);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_31);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_32);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_33);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_34);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_35);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_36);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_5C);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 21, EQ_LOW_5D);

    // DRC for twitters
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 9, DRC_HIGH_3A);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 9, DRC_HIGH_3B);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 9, DRC_HIGH_3C);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 5, DRC_HIGH_40);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 5, DRC_HIGH_41);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW, 5, DRC_HIGH_42);

    // DRC for woofer
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 9, DRC_LOW_3A);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 9, DRC_LOW_3B);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 9, DRC_LOW_3C);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 5, DRC_LOW_40);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 5, DRC_LOW_41);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF, 5, DRC_LOW_42);
}

void TAS5711_Init()
{
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,2,TAS5711_1B);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,2,TAS5711_0);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,5,TAS5711_20);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,5,TAS5711_46);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,9,TAS5711_60);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,9,TAS5711_61);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,2,TAS5711_19__BTL);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,5,TAS5711_25__BTL);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_TW,4, TAS5711_GAIN_TW);

    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,2,TAS5711_1B);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,2,TAS5711_0);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,5,TAS5711_20);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,5,TAS5711_46);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,9,TAS5711_60);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,9,TAS5711_61);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,13,TAS5711_51);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,2,TAS5711_19_PBTL);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,5,TAS5711_25_PBTL);
    DSPDrv_I2cWrite(TAS5711_ADDRESS_WF,4, TAS5711_GAIN_WF);

    TAS5711_Load_EQ();

#ifdef DISABLE_EQ
    TAS5711_EQSwitch(FALSE);
#endif
}


