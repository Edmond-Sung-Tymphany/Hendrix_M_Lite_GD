/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  dsp 1451 driver
                  -------------------------

                  SW Module Document




@file        dsp_adau1451_driver.c
@brief       This file implements the drivers for adau1451 
@author      Daniel Qin
@date        2015-12-15
@copyright (c) Tymphany Ltd. All rights reserved. 
-------------------------------------------------------------------------------
*/
#include <stdio.h>
#include "trace.h"
#include "cplus.h"
#include "commonTypes.h"
#include "I2CDrv.h"
#include "SettingSrv.h"
#include "dspAdau1451Drv_priv.h"
#include "dspDrv.conf"
#include "GpioDrv.h"
#include "DspDrv1451.h"
#include "adau1451_config.h"

/* Note if do not include math.h, pow() result will be wrong */
#include "math.h"


/***********************************************************/
/********************* Definition **************************/
/***********************************************************/
#ifndef NULL
#define NULL          (0)
#endif

/*#ifndef DSPDRV_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif*/

#define ON_STATE      (1)
#define OFF_STATE     (0)

/* For version compare */
#define SIGMASTUDIOTYPE_8_24_CONVERT(fval)       ((float)(fval))
#define DSP_VERSION_FLOAT_COMPARE(fver1,fver2)   (fabs(fver1-fver2)<0.001)

/* After I2C write, read back to check */
//#define DSP_READ_BACK_VERIFY


/***********************************************************/
/****************** Global Variable ************************/
/***********************************************************/
cGpioDrv gpioDsp;


/***********************************************************/
/****************** PUBLIC FUNCTION ************************/
/***********************************************************/
/**
 * Construct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv1451_Ctor(cDSPDrv1451* me, cI2CDrv *pI2cObj)
{
    me->i2cObj        = pI2cObj;
    me->pInitTable    = DspInitSection;
    me->sectionSize   = ArraySize(DspInitSection);
    me->initPhase     = 0;
    me->max_vol       = MAX_VOLUME;
    me->default_vol   = DEFAULT_VOLUME;
    me->isCreated     = TRUE;
    me->deviceAddr    = pI2cObj->pConfig->devAddress;
    me->i2cEnable     = TRUE;
    
    ASSERT(pI2cObj->pConfig->regAddrLen==REG_LEN_16BITS);
    ASSERT(me->deviceAddr==DEVICE_ADDR_IC_1);
}

/**
 * Destruct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv1451_Xtor(cDSPDrv1451* me)
{  
    ASSERT(me && me->isCreated);
    DSPDrv1451_Xtor_Cust(me);
    
    I2CDrv_Xtor(me->i2cObj);
    me->pInitTable    = NULL;
    me->sectionSize   = 0;
    me->initPhase     = 0;
    me->isCreated     = FALSE;

    DSP_RESET_PIN_LOW();
}

void DSPDrv1451_Xtor_ex(cDSPDrv1451* me)
{
    ASSERT(me && me->isCreated);
    DSPDrv1451_Xtor_Cust(me);

    I2CDrv_Xtor(me->i2cObj);
    me->pInitTable    = NULL;
    me->sectionSize   = 0;
    me->initPhase     = 0;
    me->isCreated     = TRUE;
}

/**
 * Set DSP initialization function
 * @param  -    init section state
 * @return -    return the delay time following if the init is not over
 *              return zero if the init is over
 */
uint16 DSPDrv1451_Init(cDSPDrv1451* me)
{
    uint16 delaytime;
    ASSERT(me && me->pInitTable && me->pInitTable[me->initPhase].initSectionFunc);
    me->pInitTable[me->initPhase].initSectionFunc(me);
    delaytime = me->pInitTable[me->initPhase].delaytime;
    me->initPhase++;
    if (me->initPhase == me->sectionSize)
    {
        me->initPhase = 0;
        return 0;
    }
    return (delaytime);
}



/**
 * Enable/Disalbe DSP I2C access
 */
void DSPDrv1451_I2cEnable(cDSPDrv1451 *me, bool i2cEnable)
{
    me->i2cEnable= i2cEnable;
}



/**
 * Mutes the DSP DAC output
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_MuteDACOut(cDSPDrv1451 *me)
{

}

/**
 * Unmutes the DSP DAC output, by clearing the bit3 and bit2 of Page0 Reg64.
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_UnMuteDACOut(cDSPDrv1451 *me)
{

}

/**
 * Mutes the Headphone output
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_MuteHPAOut(cDSPDrv1451 *me)
{

}

/**
 * Un-Mutes the Headphone output
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_UnMuteHPAOut(cDSPDrv1451 *me)
{

}

/**
 * Mutes the Lineout
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_MuteLineOut(cDSPDrv1451 *me)
{

}

/**
 * Un-Mutes the Lineout
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_UnMuteLoOut(cDSPDrv1451 *me)
{

}

/**
 * Mutes the amplifier output, it is referring to the amplifier outside the DSP.
 * This pin is not necessarily handled by DSP driver
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_MuteAmp(cDSPDrv1451 *me)
{

}

/**
 * Un-Mutes the amplifier output, it is referring to the amplifier outside the DSP.
 * This pin is not necessarily handled by DSP driver
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_UnMuteAmp(cDSPDrv1451 *me)
{

}

/**
 * Gets the status of the lineout jack status
 *
 * @param      void
 * @return     eLineoutStatus         the lineout jack is plugged or not
eLineoutStatus DSPDrv1451_GetLineoutStatus(cDSPDrv1451 *me)
{

}
 */

/**
 * Gets the status of the aux-in jack status
 *
 * @param      void
 * @return     eAuxinStatus         the auxin jack is plugged or not
 */
bool DSPDrv1451_IsAuxin(cDSPDrv1451 *me)
{
    return FALSE;
}


/**
 * Set the DAC gain
 * @param      dB_onetenth - gain level in one tenth
 */
void DSPDrv1451_setDACgain(cDSPDrv1451 *me, int dB_onetenth)
{

}

/**
 * Set the DAC mute
 * @param      bmute - mute or not
 */
void DSPDrv1451_DAC_Mute(cDSPDrv1451 *me, bool bmute)
{

}


/**
 * Select DSP input source according to channel
 * @param  -    channel
 */
#ifdef SOURCE_SELECT_REGISTER_LEN 
void DSPDrv1451_SetInputChannel(cDSPDrv1451 *me, eAudioChannel input)
{
    TP_PRINTF("DSPDrv1451_SetInputChannel: input=%d\r\n", input);
    const uint8 * ctrData = NULL;
    switch (input)
    {
        case AUDIO_CHANNEL_I2S_1:
        {
            ctrData = ADAU1451_DIGITAL_INPUT_1_SELECT;
            break;
        }
        case AUDIO_CHANNEL_I2S_2:
        {
            ctrData = ADAU1451_DIGITAL_INPUT_2_SELECT;
            break;
        }
#ifdef HAS_SPDIF_IN
        case AUDIO_CHANNEL_SPDIF_0:
        {
            ctrData = ADAU1451_DIGITAL_INPUT_SPDIF_SELECT;
            break;
        }
#endif //#ifdef HAS_SPDIF_IN
        default:
        {
            ASSERT(0);
            break;
        }
    }

    if(ctrData)
    {
        DSPDrv1451_I2cWrite(me, SOURCE_SELECT_REGISTER_LEN, (uint8*)ctrData);
    }
}
#endif

/**
 * Set line-in sensitivity
 * @param  -    Enable
 */
#ifdef ADAU1451_LINE_IN_SENSITIVITY_HIGH
void DSPDrv1451_SetLineInSensitivity(cDSPDrv1451 *me, eLineinSensitivity level)
{
    TP_PRINTF("DSPDrv1451_SetLineInSensitivity: level=%d\r\n", level);
    const uint8 * ctrData = NULL;
    switch (level)
    {
        case LINE_IN_SENSITIVITY_HIGH:
        {
            ctrData = ADAU1451_LINE_IN_SENSITIVITY_HIGH;
            break;
        }
        case LINE_IN_SENSITIVITY_MEDIUM:
        {
            ctrData = ADAU1451_LINE_IN_SENSITIVITY_MEDIUM;
            break;
        }
        case LINE_IN_SENSITIVITY_LOW:
        {
            ctrData = ADAU1451_LINE_IN_SENSITIVITY_LOW;
            break;
        }
        case LINE_IN_SENSITIVITY_DISABLE:
        {
            ctrData = ADAU1451_LINE_IN_SENSITIVITY_DISABLE;
            break;
        }        default:
        {
            ASSERT(0);
            break;
        }
    }

    if(ctrData)
    {
        DSPDrv1451_I2cWrite(me, SOURCE_SELECT_REGISTER_LEN, (uint8*)ctrData);
    }
}
#endif


#ifdef HAS_LINE_IN_MULTI_ROOM
/**
 * Select the line-in multi-room source
 * @param  -    channel
 */
void DSPDrv1451_SetLineInMultiRoomChannel(cDSPDrv1451 *me, eAudioChannel input)
{
    TP_PRINTF("DSPDrv1451_SetLineInMultiRoomChannel: input=%d\r\n", input);
    const uint8 * ctrData = NULL;
    switch (input)
    {
        case AUDIO_CHANNEL_I2S_1:
        {
            ctrData = NULL;
            break;
        }
        case AUDIO_CHANNEL_I2S_2: //Auxin source from external ADC chip: PCM1862
        {
            ctrData = ADAU1451_LINE_IN_2_SELECT;
            break;
        }
#ifdef HAS_SPDIF_IN
        case AUDIO_CHANNEL_SPDIF_0:
        {
            ctrData = ADAU1451_LINE_IN_1_SELECT;
            break;
        }
#endif //#ifdef HAS_SPDIF_IN
        default:
        {
            ASSERT(0);
            break;
        }
    }

    if(ctrData)
    {
        DSPDrv1451_I2cWrite(me, LINE_IN_SOURCE_SELECT_REGISTER_LEN, (uint8*)ctrData);
    }
}

/**
 * Mute line-in multi room or not
 * @param  -    Enable
 */
void DSPDrv1451_MuteLineInToAsetk(cDSPDrv1451 *me, bool bMute)
{
    TP_PRINTF("DSPDrv1451_MuteLineInMultiRoom: mute=%d \r\n", bMute);

    const uint8 * ctrData = NULL;
    if(bMute)
        ctrData= ADAU1451_LINE_IN_TO_ASETK_MUTE_ENABLE;
    else
        ctrData= ADAU1451_LINE_IN_TO_ASETK_MUTE_DISABLE;

    DSPDrv1451_I2cWrite(me, LINE_IN_TO_ASETK_MUTE_REGISTER_LEN, (uint8*)ctrData);
}

#endif //#ifdef HAS_LINE_IN_MULTI_ROOM

/**
 * Enable pass-through or not
 * @param  -    passEnable
 */
#ifdef PASS_THROUGH_REGISTER_LEN 
void DSPDrv1451_SetPassthrough(cDSPDrv1451 *me, bool passEnable)
{
    TP_PRINTF("DSPDrv1451_SetPassthrough: passEnable=%d \r\n", passEnable);
    
    const uint8 * ctrData = NULL;
    if(passEnable)
        ctrData= ADAU1451_PASS_THROUGH_ENABLE;
    else
        ctrData= ADAU1451_PASS_THROUGH_DISABLE;
    
    DSPDrv1451_I2cWrite(me, PASS_THROUGH_REGISTER_LEN, (uint8*)ctrData);
}
#endif


#ifdef HAS_AUDIO_MUTE_CHANNEL
/**
 * Mute Woofer or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteWoofer(cDSPDrv1451 *me, bool bIsMute)
{
    TP_PRINTF("DSPDrv1451_MuteWoofer: bIsMute=%d \r\n", bIsMute);

    const uint8 * ctrData = NULL;
    if(bIsMute)
        ctrData= ADAU1451_MUTE_WOOFER_ENABLE;
    else
        ctrData= ADAU1451_MUTE_WOOFER_DISABLE;

    DSPDrv1451_I2cWrite(me, MUTE_WOOFER_REGISTER_LEN, (uint8*)ctrData);
}

/**
 * Mute MiddleA or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteMiddleA(cDSPDrv1451 *me, bool bIsMute)
{
    TP_PRINTF("DSPDrv1451_MuteMiddleA: bIsMute=%d \r\n", bIsMute);

    const uint8 * ctrData = NULL;
    if(bIsMute)
        ctrData= ADAU1451_MUTE_MIDDLEA_ENABLE;
    else
        ctrData= ADAU1451_MUTE_MIDDLEA_DISABLE;

    DSPDrv1451_I2cWrite(me, MUTE_MIDDLEA_REGISTER_LEN, (uint8*)ctrData);
}

/**
 * Mute MiddleB or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteMiddleB(cDSPDrv1451 *me, bool bIsMute)
{
    TP_PRINTF("DSPDrv1451_MuteMiddleB: bIsMute=%d \r\n", bIsMute);

    const uint8 * ctrData = NULL;
    if(bIsMute)
        ctrData= ADAU1451_MUTE_MIDDLEB_ENABLE;
    else
        ctrData= ADAU1451_MUTE_MIDDLEB_DISABLE;

    DSPDrv1451_I2cWrite(me, MUTE_MIDDLEB_REGISTER_LEN, (uint8*)ctrData);
}

/**
 * Mute Tweeter or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteTweeter(cDSPDrv1451 *me, bool bIsMute)
{
    TP_PRINTF("DSPDrv1451_MuteTweeter: bIsMute=%d \r\n", bIsMute);

    const uint8 * ctrData = NULL;
    if(bIsMute)
        ctrData= ADAU1451_MUTE_TWEETER_ENABLE;
    else
        ctrData= ADAU1451_MUTE_TWEETER_DISABLE;

    DSPDrv1451_I2cWrite(me, MUTE_TWEETER_REGISTER_LEN, (uint8*)ctrData);
}
#endif


/**
 * Write to DC value with 32.0 format
 * @param  -    passEnable
 */
void DSPDrv1451_WriteDcValue32_0(cDSPDrv1451 *me, uint16 reg_addr, uint32 data)
{
    //TP_PRINTF("DSPDrv1451_WriteDcValue32_0: reg=%d, data=%d\r\n", reg_addr, data);   
       
    uint8 data_iic[DC_REGISTER_LEN] = {0}; /* write DSP register */
    data_iic[0] = reg_addr >> 8;
    data_iic[1] = reg_addr & 0x00FF;
    data_iic[2] = UINT32_GET_BIT_RANGE(data, 31, 24);
    data_iic[3] = UINT32_GET_BIT_RANGE(data, 23, 16);
    data_iic[4] = UINT32_GET_BIT_RANGE(data, 15,  8);
    data_iic[5] = UINT32_GET_BIT_RANGE(data,  7,  0);
    DSPDrv1451_I2cWrite(me, DC_REGISTER_LEN, (uint8*)data_iic);
}


/**
 * Write to DC value with 8.24 format
 * @param  -    passEnable
 */
void DSPDrv1451_WriteValue8_24(cDSPDrv1451 *me, uint16 reg_addr, float fValue)
{
    uint32 data= DSPDrv1451_FloatTo8_24Data(me, fValue);
    //TP_PRINTF("DSPDrv1451_WriteValue8_24: reg=%d, dValue=%f => data=%d\r\n", reg_addr, fValue, data);
       
    uint8 data_iic[DC_REGISTER_LEN] = {0}; /* write DSP register */
    data_iic[0] = reg_addr >> 8;
    data_iic[1] = reg_addr & 0x00FF;
    data_iic[2] = UINT32_GET_BIT_RANGE(data, 31, 24);
    data_iic[3] = UINT32_GET_BIT_RANGE(data, 23, 16);
    data_iic[4] = UINT32_GET_BIT_RANGE(data, 15,  8);
    data_iic[5] = UINT32_GET_BIT_RANGE(data,  7,  0);
    DSPDrv1451_I2cWrite(me, DC_REGISTER_LEN, (uint8*)data_iic);
}


float DSPDrv1451_ReadValue8_24(cDSPDrv1451 *me, uint16 reg_addr)
{
    uint8   data[DC_REGISTER_LEN] = {0};
    float   fValue= 0.0;

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, reg_addr, sizeof(data), data);
    fValue= DSPDrv1451_DataToFloat(me, UINT32_BYTE_INV(data), 8, 24); //format 8.24
    //TP_PRINTF("\r\n\r\nvolInput= %02X %02X %02X %02X = %f\r\n", volData[0], volData[1], volData[2], volData[3], fVol);
    
    return fValue;
}



/**
 * Write to Read value with 32.0 format
 * @param  -    passEnable
 */
uint32 DSPDrv1451_ReadDcValue32_0(cDSPDrv1451 *me, uint16 reg_addr)
{
    ASSERT(me->isCreated);
    //TP_PRINTF("DSPDrv1451_ReadDcValue32_0: reg=%d\r\n", reg_addr, data);
    uint8 data[4];
    DSPDrv1451_I2cRead(me, reg_addr, sizeof(data), (void*)&data);
    uint32 ret= (data[0]<<24) | (data[1]<<16) | (data[2]<<8) | data[3];
    return ret;
}



/**
 * Sets the DSP volume, by writing the L/R channels PGA Gain registers. Please
 * reference on the volume table to check the real gain.
 *
 * @param      vol             The volume step that will be set
 * @return     void
 */
#ifdef ADAU1451_VOLUME_ADDR
void DSPDrv1451_SetVol(cDSPDrv1451 *me, uint8 vol)
{
    //TP_PRINTF("DSPDrv1451_SetVol: vol=%d \r\n", vol);
    
    uint8 vol_iic[ADAU1451_UPDATE_VOLUME_LEN] = {0,0,0,0,0,0}; /* write DSP register */
    uint32 reg_addr= ADAU1451_VOLUME_ADDR;
    
    if(vol > MAX_VOLUME)
    {
        vol = MAX_VOLUME;
        ASSERT(0);
    }
    else if(vol < MIN_VOLUME)
    {
        vol = MIN_VOLUME;
        ASSERT(0);
    }

    vol_iic[0] = reg_addr >> 8;
    vol_iic[1] = reg_addr & 0x00FF;
    vol_iic[2] = 0;
    vol_iic[3] = 0;
    vol_iic[4] = 0;
    vol_iic[5] = vol;

    DSPDrv1451_I2cWrite(me, ADAU1451_UPDATE_VOLUME_LEN, (uint8*)&vol_iic);
}
#endif

/**
 * Detect whether there is a input present
 *
 * @param
 * @return     bool    If the music is present either source depending on the DSP flow design
 */
#ifdef ADAU1451_SIGNAL_DETECTION_ADDR
bool DSPDrv1451_HasMusicStream(cDSPDrv1451 *me)
{
    bool    ret = FALSE;
    uint8   detectData[ADAU1451_SIGNAL_DETECTION_LEN] = {0};

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, ADAU1451_SIGNAL_DETECTION_ADDR, sizeof(detectData), detectData);
    ret = detectData[0]?FALSE:TRUE; /* low active */
    return ret;
}
#endif


#ifdef VOL_INPUT_REGISTER
float DSPDrv1451_ReadInputVolume(cDSPDrv1451 *me)
{
    uint8   volData[VOL_REGISTER_LEN] = {0};
    float   fVol= 0.0;

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, VOL_INPUT_REGISTER, sizeof(volData), volData);
    fVol= DSPDrv1451_DataToFloat(me, UINT32_BYTE_INV(volData), 8, 24); //format 8.24
    //TP_PRINTF("\r\n\r\nvolInput= %02X %02X %02X %02X = %f\r\n", volData[0], volData[1], volData[2], volData[3], fVol);
    
    return fVol;
}
#endif


#ifdef VOL_INPUT2_REGISTER
float DSPDrv1451_ReadInputVolume2(cDSPDrv1451 *me)
{
    uint8   volData[VOL_REGISTER_LEN] = {0};
    float   fVol= 0.0;

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, VOL_INPUT2_REGISTER, sizeof(volData), volData);
    fVol= DSPDrv1451_DataToFloat(me, UINT32_BYTE_INV(volData), 8, 24); //format 8.24
    //TP_PRINTF("\r\n\r\nvolInput= %02X %02X %02X %02X = %f\r\n", volData[0], volData[1], volData[2], volData[3], fVol);
    
    return fVol;
}
#endif


#ifdef VOL_AUXIN_INPUT_REGISTER
float DSPDrv1451_ReadInputAuxinVolume(cDSPDrv1451 *me)
{
    uint8   volData[VOL_REGISTER_LEN] = {0};
    float   fVol= 0.0;

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, VOL_AUXIN_INPUT_REGISTER, sizeof(volData), volData);
    fVol= DSPDrv1451_DataToFloat(me, UINT32_BYTE_INV(volData), 8, 24); //format 8.24
    //TP_PRINTF("\r\n\r\nvolInput= %02X %02X %02X %02X = %f\r\n", volData[0], volData[1], volData[2], volData[3], fVol);
    
    return fVol;
}

float DSPDrv1451_ReadInputAuxinVolume2(cDSPDrv1451 *me)
{
    uint8   volData[VOL_REGISTER_LEN] = {0};
    float   fVol= 0.0;

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, VOL_AUXIN_INPUT2_REGISTER, sizeof(volData), volData);
    fVol= DSPDrv1451_DataToFloat(me, UINT32_BYTE_INV(volData), 8, 24); //format 8.24
    //TP_PRINTF("\r\n\r\nvolInput= %02X %02X %02X %02X = %f\r\n", volData[0], volData[1], volData[2], volData[3], fVol);
    
    return fVol;
}
#endif


#ifdef VOL_OUTPUT_WOOFER_REGISTER
float DSPDrv1451_ReadWooferOutputVolume(cDSPDrv1451 *me)
{
    uint8   volData[VOL_REGISTER_LEN] = {0};
    float   fVol= 0.0;

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, VOL_OUTPUT_WOOFER_REGISTER, sizeof(volData), volData);
    fVol= DSPDrv1451_DataToFloat(me, UINT32_BYTE_INV(volData), 8, 24); //format 8.24
    //TP_PRINTF("volWooferOutput= %02X %02X %02X %02X = %f\r\n", volData[0], volData[1], volData[2], volData[3], fVol);
    return fVol;
}
#endif

#ifdef VOL_OUTPUT_MIDDLE_A_REGISTER
float DSPDrv1451_ReadMiddleAOutputVolume(cDSPDrv1451 *me)
{
    uint8   volData[VOL_REGISTER_LEN] = {0};
    float   fVol= 0.0;

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, VOL_OUTPUT_MIDDLE_A_REGISTER, sizeof(volData), volData);
    fVol= DSPDrv1451_DataToFloat(me, UINT32_BYTE_INV(volData), 8, 24); //format 8.24
    //TP_PRINTF("volTweeterOutput= %02X %02X %02X %02X = %f\r\n", volData[0], volData[1], volData[2], volData[3], fVol);
    return fVol;
}
#endif


#ifdef VOL_OUTPUT_MIDDLE_B_REGISTER
float DSPDrv1451_ReadMiddleBOutputVolume(cDSPDrv1451 *me)
{
    uint8   volData[VOL_REGISTER_LEN] = {0};
    float   fVol= 0.0;

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, VOL_OUTPUT_MIDDLE_B_REGISTER, sizeof(volData), volData);
    fVol= DSPDrv1451_DataToFloat(me, UINT32_BYTE_INV(volData), 8, 24); //format 8.24
    //TP_PRINTF("volTweeterOutput= %02X %02X %02X %02X = %f\r\n", volData[0], volData[1], volData[2], volData[3], fVol);
    return fVol;
}
#endif


#ifdef VOL_OUTPUT_TWEETER_REGISTER
float DSPDrv1451_ReadTweeterOutputVolume(cDSPDrv1451 *me)
{
    uint8   volData[VOL_REGISTER_LEN] = {0};
    float   fVol= 0.0;

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, VOL_OUTPUT_TWEETER_REGISTER, sizeof(volData), volData);
    fVol= DSPDrv1451_DataToFloat(me, UINT32_BYTE_INV(volData), 8, 24); //format 8.24
    //TP_PRINTF("volTweeterOutput= %02X %02X %02X %02X = %f\r\n", volData[0], volData[1], volData[2], volData[3], fVol);
    return fVol;
}
#endif



#ifdef HAS_SPDIF_IN
/**
 * Detect whether auxin has music streaming.
 *
 * @param
 * @return     bool    If the music is present either source depending on the DSP flow design
 */
bool DSPDrv1451_SpdifInHasMusicStream(cDSPDrv1451 *me)
{
    bool    ret = FALSE;
    uint8   detectData[ADAU1451_SIGNAL_DETECTION_LEN] = {0};

    ASSERT(me->isCreated);
    DSPDrv1451_I2cRead(me, ADAU1451_SPDIF_IN_SIGNAL_DETECTION_ADDR, sizeof(detectData), detectData);
    ret = detectData[0]?FALSE:TRUE; /* low active */
    return ret;
}
#endif //#ifdef HAS_SPDIF_IN

//void DSPDrv1451_SetAudio(cDSPDrv1451 *me, eAudioSettId dspSettId, BOOL enable)
//{
//    uint8 i;
//    for(i = 0; i < ArraySize(ctrIdEQIdMap); i++)
//    {
//        if(dspSettId == ctrIdEQIdMap[i].dspSettid)
//        {
//            break;
//        }
//    }
//    ASSERT(dspSettId < DSP_SETT_ID_MAX);
//    switch(dspSettId)
//    {
//        case DSP_VOLUME_SETT_ID:
//        {
//            if(enable)
//            {
//                /* Convert it to positive */
//                uint8 volume = (pSettData[ctrIdEQIdMap[i].dbIndex])/VALUE_MAGNIFICATION+MAX_VOLUME;
//                DSPDrv1451_SetVol(me,volume);
//            }
//            else
//            {
//                DSPDrv1451_SetVol(me,0);
//            }
//        }
//        break;
//        case DSP_PASSTHROUGH_SETT_ID:
//        {
//            DSPDrv1451_SetPassthrough(me, enable);
//        }
//        break;
//        default:
//            break;
//    }
//}


/**
 * Enter/exit low power sleep (hibernation) mode
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_SleepModeEnable(cDSPDrv1451 *me, bool enable)
{
    TP_PRINTF("DSPDrv1451_SleepModeEnable: enable=%d \r\n", enable);
    
    uint8 buffer[4];
    buffer[0]=(REG_ADDR_HIBERNATE>>8);
    buffer[1]=(REG_ADDR_HIBERNATE&0xff);
    
    if(enable) {
        buffer[2]= DATA_HIBERNATE_ENABLE>>8;
        buffer[3]= DATA_HIBERNATE_ENABLE&0xff;
    }
    else {
        buffer[2]= DATA_HIBERNATE_DISABLE>>8;
        buffer[3]= DATA_HIBERNATE_DISABLE&0xff;
    }
    
    DSPDrv1451_I2cWrite(me, sizeof(buffer), (uint8*)buffer);  
}


/**
 * Write Loudness value
 */
#ifdef ADAU1451_PARAM_POSITION_REGISTER_LEN
void DSPDrv1451_SetPosition(cDSPDrv1451 *me, eSpeakerPosition position)
{
    TP_PRINTF("DSPDrv1451_SetPosition: position=%d \r\n", position);
    switch(position)
    {
    case SPEAKER_POSITION_FREE:
        DSPDrv1451_I2cWrite(me, ADAU1451_PARAM_POSITION_REGISTER_LEN, ADAU1451_PARAM_POSITION_FREE_SELECT);
        break;
    case SPEAKER_POSITION_WALL:
        DSPDrv1451_I2cWrite(me, ADAU1451_PARAM_POSITION_REGISTER_LEN, ADAU1451_PARAM_POSITION_WALL_SELECT);
        break;
    case SPEAKER_POSITION_CORNER:
        DSPDrv1451_I2cWrite(me, ADAU1451_PARAM_POSITION_REGISTER_LEN, ADAU1451_PARAM_POSITION_CORNER_SELECT);
        break;
    default:
        ASSERT(0);
        break;
    }    
}
#endif

/**
 * Read ASRC Conversion rate
 *
 * @param      asrc: ASRC index
 * @return     conversion rate
 */
float DSPDrv1451_ReadAsrcConversionRate(cDSPDrv1451 *me, tDspAsrc asrc)
{    
    ASSERT(me && me->isCreated);
    ASSERT(asrc>=DSP_ASRC_MIN && asrc<=DSP_ASRC_MAX);

    uint8   data[ADAU1451_ASRC_RATIO_LEN]= {0};
    const uint32 reg_asrc= REG_ADDR_ASRC0_RATIO + asrc;
    DSPDrv1451_I2cRead(me, reg_asrc, ADAU1451_ASRC_RATIO_LEN, data);
    
    float asrc_float= DSPDrv1451_DataToFloat(me, UINT16_BYTE_INV(data), 4, 12); //format 4.12

    //TP_PRINTF("DSPDrv1451_ReadAsrcConversionRate: asrc=%d, rate=%f\r\n", asrc, asrc_float);
    return asrc_float;
}


/**
 * Auto restart SPDIF when remove SPDIF cable and re-insert
 *
 * @param      N/A
 * @return     float float version
 */
void DSPDrv1451_SetSpdifAutoRestart(cDSPDrv1451 *me, bool enable)
{
    TP_PRINTF("DSPDrv1451_SetSpdifAutoRestart: enable=%d \r\n", enable);
    
    uint8 buffer[4];
    buffer[0]=(REG_ADDR_SPDIF_AUTO_RESTART>>8);
    buffer[1]=(REG_ADDR_SPDIF_AUTO_RESTART&0xff);
    
    if(enable) {
        buffer[2]= DATA_SPDIF_AUTO_RESTART_ENABLE>>8;
        buffer[3]= DATA_SPDIF_AUTO_RESTART_ENABLE&0xff;
    }
    else {
        buffer[2]= DATA_SPDIF_AUTO_RESTART_DISABLE>>8;
        buffer[3]= DATA_SPDIF_AUTO_RESTART_DISABLE&0xff;
    }
    
    DSPDrv1451_I2cWrite(me, sizeof(buffer), (uint8*)buffer);  

}


/**
 * Read DSP Version
 *
 * @param      N/A
 * @return     float float version
 */
float DSPDrv1451_GetDspVer(cDSPDrv1451 *me)
{
    /* Note DSP header file also include versoon:
     *   #define MOD_DC1_DCINPALG145X1VALUE_VALUE    SIGMASTUDIOTYPE_8_24_CONVERT(1.4)
     * We could consider to use this version later
     */
    ASSERT(me && me->isCreated);
      
    float fVer1= 0.0;
    uint8   data[DSP_VER_REGISTER_LEN] = {0};
    DSPDrv1451_I2cRead(me, DSP_VER_REGISTER, sizeof(data), data);             
    fVer1= DSPDrv1451_DataToFloat(me, UINT32_BYTE_INV(data), 8, 24); //format 8.24

    //Verify version
    float fVer2= DSP_VER_FLOAT_VALUE;
    ASSERT( DSP_VERSION_FLOAT_COMPARE(fVer1,fVer2) );

    //TP_PRINTF("\r\nDSP Version: %02f, 0x%x%x%x%x\r\n\r\n", fVer1, data[0], data[1], data[2], data[3]);
    TP_PRINTF("\r\nDSP Version: %02.2f\r\n\r\n", fVer1);
    return fVer1;
}



/**
 * Convert standard float number to DSP float raw data 8.24
 *
 * @param      float_val: float value
 *
 * @return     starnard float number
 * 
 * @example
 *   float_val=16.000000   ==>  return 0x10000000
 *   float_val=1.000000    ==>  return 0x01000000
 *   float_val=0.800000    ==>  return 0x00cccccd
 *   float_val=0.500000    ==>  return 0x00800000
 *   float_val=0.250000    ==>  return 0x00400000
 *   float_val=0.000000    ==>  return 0x00000000
 *   float_val=-16.000000  ==>  return 0xf0000000
 *   float_val=-15.000000  ==>  return 0xf1000000
 *   float_val=-1.000000   ==>  return 0xff000000
 */
static uint32 DSPDrv1451_FloatTo8_24Data(cDSPDrv1451 *me, float float_val)
{
    //Check invalid 8.24 float
    if(float_val>=128.0 || float_val<-128.0) {
        //ASSERT(0);
        TP_PRINTF("\r\n*** DSPDrv1451_FloatTo8_24Data: invalid floating nubmer %f\r\n\r\n", float_val );
        return 0;
    }
    int64_t param_1;
    param_1 = float_val * (1<<24);
    uint32 data = (uint32)param_1;

    return data;
}



/**
 * Convert DSP float raw data to float number
 *
 * @param      data: float data with DSP format
 *             fmt_num: Numeric nubmer of DSP format
 *             fmt_frac: Fraction nubmer of DSP format
 *
 * @return     starnard float number
 * 
 * @Example, 
 * Numerical Format: (fmt_num).(fmt_frac)
 *  Range: -16.0 to (+16.0 - 1 LSB)
 *  0000 1000  0000 0000  0000 0000  0000 0000 =  -16.0 
 *  0000 1110  0000 0000  0000 0000  0000 0000 =  -4.0 
 *  0000 1111  1000 0000  0000 0000  0000 0000 =  -1.0 
 *  0000 1111  1110 0000  0000 0000  0000 0000 =  -0.25 
 *  0000 1111  1111 1111  1111 1111  1111 1111 =  (1 LSB below 0.0) 
 *  0000 0000  0000 0000  0000 0000  0000 0000 =  0.0 
 *  0000 0000  0010 0000  0000 0000  0000 0000 =  0.25 
 *  0000 0000  1000 0000  0000 0000  0000 0000 =  1.0 (0 dB full scale)
 *  0000 0010  0000 0000  0000 0000  0000 0000 =  4.0 
 *  0000 0111  1111 1111  1111 1111  1111 1111 =  (16.0 - 1 LSB)
 */
static float DSPDrv1451_DataToFloat(cDSPDrv1451 *me, uint32 data, uint8 fmt_num, uint8 fmt_frac)
{ 
    ASSERT((fmt_num+fmt_frac) <= 32);
    uint8 sign_bit= fmt_num + fmt_frac - 1;
    uint32 sign_value= UINT32_GET_BIT(data, sign_bit);
    ASSERT(sign_bit > fmt_frac);
        
    uint32 value_uint= UINT32_GET_BIT_RANGE(data, sign_bit-1, fmt_frac);
    float value_float= 0.0;
    if(sign_value==0)
    {   //positive number
        value_float= (float)value_uint;
    }
    else
    {   //negative number
        uint32 value_max= 2<<(fmt_num-2);  //2^fmt_num
        value_float= -(float)(value_max-value_uint);
    }
    
    float f= 1;
    for(int32 i=fmt_frac-1 ; i>=0 ; i--)
    {
        f/= 2;
        if(UINT32_GET_BIT_RANGE(data, i, i)) {
            value_float+= f;
            //TP_PRINTF("value_float=%f\r\n", value_float);
        }
    }
      
    return value_float;
}


#ifdef REDUCE_TOTAL_GAIN_WHILE_LOW_POWER
/**
 * Refresh volume when battery is low, reduce it by 6dB
 *
 * @param      uint8        target volume
 * @return     void
 */
void DSPDrv1451_SetVolForLowPower(cDSPDrv1451 *me, uint8 vol)
{
    
}
#endif

#ifdef REDUCE_DYNAMIC_RANGE_WHILE_LOW_POWER
/**
 * Refresh DRCs when power is low
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_SetDrcForLowPower(cDSPDrv1451 *me)
{

}

/**
 * Refresh DRCs for normal power
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_SetDrcForNormalPower(cDSPDrv1451 *me)
{

}

/**
 * Refresh DRCs for adaptor mode
 *
 * @param      void
 * @return     void
 */
void DSPDrv1451_SetDrcForAdaptorMode(cDSPDrv1451 *me)
{
}

#endif






/***********************************************************/
/***************** PRIVATE FUNCTION **************************/
/***********************************************************/
static void DSPDrv1451_SetReset(bool reset)
{
    if(reset)
    {
        DSP_RESET_PIN_LOW();
    }
    else
    {
        DSP_RESET_PIN_HIGH();
    }
}

static void DSPDrv1451_Reset1(void *p)
{
    TP_PRINTF("DSPDrv1451_Reset1\r\n");
    
    //GPIO pins init
    tGPIODevice* pDspGPIOConf = NULL;
    pDspGPIOConf= (tGPIODevice*)getDevicebyIdAndType(DSP_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(pDspGPIOConf);    
    GpioDrv_Ctor(&gpioDsp,pDspGPIOConf);

    //pull reset pin
    DSPDrv1451_SetReset(FALSE);
}

static void DSPDrv1451_Reset2(void *p)
{
    TP_PRINTF("DSPDrv1451_Reset2\r\n");
    DSPDrv1451_SetReset(TRUE);
}

static void DSPDrv1451_Reset3(void *p)
{
    TP_PRINTF("DSPDrv1451_Reset3\r\n");
    DSPDrv1451_SetReset(FALSE);
}

static void DSPDrv1451_InitI2c(void *p)
{
    TP_PRINTF("DSPDrv1451_InitI2c\r\n");

    //Initialize I2C
    cDSPDrv1451* me = (cDSPDrv1451*)p;
    const tDevice * pDevice = NULL;
    pDevice = getDevicebyIdAndType(DSP_DEV_ID, I2C_DEV_TYPE, NULL);
    ASSERT(pDevice);
    I2CDrv_Ctor(me->i2cObj,(tI2CDevice*)pDevice);
    me->i2cObj->registeredUser++;

    TP_PRINTF("DSPDrv1451_InitI2c finish\r\n");
}

static void DSPDrv1451_Writer_Register(cDSPDrv1451* me, uint16 devAddr, uint32 regAddr, uint16 size, const uint8 *pData)
{
    uint8 buffer[2+ADAU1451_REGISTER_LEN];
    ASSERT(size<=ADAU1451_REGISTER_LEN); /* Note: the size should not more than ADAU1451_REGISTER_LEN. */
    ASSERT(me->deviceAddr==devAddr); //I2C address on DSP flow should equal to I2C setting on accachedDevices.c

    buffer[0]=(regAddr>>8);
    buffer[1]=(regAddr&0xff);
    memcpy(&buffer[2], pData, size);
    DSPDrv1451_I2cWrite(me, (2+size), (uint8*)buffer);
}

/**
 * Write data to DSP by I2C.
 *
 * @param      uint8           device I2C address
 *
 * @param      uint8           data byte number
 *
 * @param      const uint8*    pointer to data array
 *
 * @return     void
 */
static void DSPDrv1451_I2cWrite(cDSPDrv1451 *me, uint16 length, const uint8 *data)
{
    /* Make sure DSP flow's address is the same with attachedDevices.c */
    ASSERT(DEVICE_ADDR_IC_1==me->deviceAddr);
    if(!me->i2cEnable) {
        return;
    }

    tI2CMsg i2cMsg =
    {
        .devAddr = me->deviceAddr,
        .regAddr = NULL,
        .length  = length,
        .pMsg    = (uint8*)data
    };
    bool ret= I2CDrv_MasterWrite(me->i2cObj, &i2cMsg);
    ASSERT(ret==TP_SUCCESS);
    
#ifdef DSP_READ_BACK_VERIFY
    int32 lenRb= length-2;
    uint32 reg_addr= (data[0]<<8) | data[1];
    if(length>=0 && reg_addr!=REG_PLL_CTRL1_IC_1_ADDR && reg_addr!=REG_PLL_CLK_SRC_IC_1_ADDR && reg_addr!=REG_MCLK_OUT_IC_1_ADDR)
    {
        uint8 *pDataRb= (uint8*)malloc(lenRb);
        ASSERT(pDataRb);
        memset(pDataRb, 0, lenRb);
        DSPDrv1451_I2cRead(me, reg_addr, lenRb, pDataRb);
        ASSERT( 0==memcmp(data+2, pDataRb, lenRb) );
        free(pDataRb);
    }
#endif
}

static void DSPDrv1451_I2cRead(cDSPDrv1451 *me, uint32 regAddr, uint16 length, const uint8 *data)
{
    /* Make sure DSP flow's address is the same with attachedDevices.c */
    ASSERT(DEVICE_ADDR_IC_1==me->deviceAddr);
    if(!me->i2cEnable) {
        return;
    }
    
    tI2CMsg i2cMsg =
    {
        .devAddr = me->deviceAddr,
        .regAddr = regAddr,
        .length  = length,
        .pMsg    = (uint8*)data
    };
    bool ret= I2CDrv_MasterRead(me->i2cObj, &i2cMsg);
    ASSERT(ret==TP_SUCCESS);
}

