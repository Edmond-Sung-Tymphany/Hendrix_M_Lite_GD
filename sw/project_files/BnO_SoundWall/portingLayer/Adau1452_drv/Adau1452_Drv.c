/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  dsp adau1452 driver
                  -------------------------
@file        Adau1452_Drv.c
@brief       This file implements the drivers for adau1452 
@author      Viking WANG
@date        2016-10-20
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/
#include "product.config"

#include "commontypes.h"
#include "stm32f0xx.h"
#include "trace.h"
#include "bsp.h"
#include "I2CDrv.h"
#include "SystemDrv.h"

#ifdef HAS_SOUNDMODE_UPDATE
#include "core.pb.h"
#endif

#include "Adau1452_Drv.h"
#include "./Adau1452_Drv_priv.h"
#include "./Adau1452_Volume.h"

#include "./BnO_SoundWall_IC_1_REG.h"
#include "./BnO_SoundWall_IC_1.h"
#include "./BnO_SoundWall_IC_1_PARAM.h"

#include "./Adau1452_Init.config"

/* BnO used > 100 version number, TYM used < 100 version number */
#define IS_DSP_FROM_BnO     (MOD_VERSION_DCINPALG145X1VALUE_FIXPT > 100)

static uint8_t adau1452_dev_addr;
static uint8_t adau1452_inti_stage=0;

static Adau1452Drv_t adau1452_drv;
static Adau1452Drv_t *p_adau1452_drv=NULL;

#ifdef HAS_SOUNDMODE_UPDATE
static SoundModeGainAndDelay_t sound_mode_gain_and_delay;
#endif

/**
 * Convert standard float number to DSP float raw data 8.24
 *
 * @param      float_val: float value
 *
 * @return     starnard float number
 * 
 */
static uint32_t Adau1452Drv_FloatTo8_24Data(float float_val)
{
    float float_val2 = float_val;
    float f = 0.5;
    
    //Check invalid 8.24 float
    /*
    if(float_val>=128.0 || float_val<-128.0) {
        ASSERT(0);
        return 0;
    }
    */
    
    //number part
    int8 data_num= ((int32)float_val2) & 0xFF;
    //TP_PRINTF("data_num= 0x%02x\r\n", data_num);

    //frac part  
    uint32_t data_frac= 0;
    float_val2= float_val2 - (float)((int32)float_val); //give up number part
    if(float_val2<0.0)
    {
        float_val2 *= -1.0;
    }
    for(int32 i=23 ; i>=0 ; i--)
    {
        if( float_val2 >= f )
        {
            data_frac = data_frac | (1<<i);
            float_val2 -= f;
        }        
        f /= 2;
    }
//    ASSERT(data_frac<=0xFFFFFF);
//    ASSERT(float_val2>=0.0);
    
    //merge two part
    uint32_t data = (data_num<<24) | (data_frac);
    //TP_PRINTF("data= 0x%08x\r\n", data);
    return data;    
}

/* for sigma studio compiler */
static void SIGMA_WRITE_REGISTER_BLOCK(uint8_t device_add, uint16_t reg_add, uint16_t bytes, const uint8_t* data)
{
    eTpRet accessOK;

    tI2CMsg i2cMsg;

    ASSERT(p_adau1452_drv->isReady);

    if( ! p_adau1452_drv->i2cEnable )
        return ;
    
    i2cMsg.devAddr = device_add;
    i2cMsg.regAddr = reg_add;
    i2cMsg.length = bytes;
    i2cMsg.pMsg = (uint8_t *)data;

    accessOK = I2CDrv_MasterWriteWith2ByteRegAddress(&p_adau1452_drv->i2cDrv, &i2cMsg);

    if( accessOK != TP_SUCCESS )
    {
#ifdef HAS_I2C_BUS_DETECT
        SystemDrv_SetI2cBusStatus(I2C_ERROR_DSP);
#endif
        p_adau1452_drv->i2cEnable = FALSE;
    }
}

static void Adau1452Drv_I2CWrite(uint16_t reg_add, uint16_t bytes, const uint8_t* data)
{
    eTpRet accessOK;

    tI2CMsg i2cMsg;

    ASSERT(p_adau1452_drv->isReady);

    if( ! p_adau1452_drv->i2cEnable )
        return ;
    
    i2cMsg.devAddr = adau1452_dev_addr;
    i2cMsg.regAddr = reg_add;
    i2cMsg.length = bytes;
    i2cMsg.pMsg = (uint8_t *)data;

    accessOK = I2CDrv_MasterWriteWith2ByteRegAddress(&p_adau1452_drv->i2cDrv, &i2cMsg);

    if( accessOK != TP_SUCCESS )
    {
#ifdef HAS_I2C_BUS_DETECT
        SystemDrv_SetI2cBusStatus(I2C_ERROR_DSP);
#endif
        p_adau1452_drv->i2cEnable = FALSE;
    }
}

static void Adau1452Drv_I2CRead(uint16_t reg_add, uint16_t bytes, uint8_t* data)
{
    ASSERT(bytes < 0x100);
    ASSERT(p_adau1452_drv->isReady);

    if( ! p_adau1452_drv->i2cEnable )
        return ;
    
    tI2CMsg i2cMsg =
    {
        .devAddr = adau1452_dev_addr,
        .regAddr = reg_add,
        .length  = bytes,
        .pMsg    = data
    };
    I2CDrv_MasterRead(&p_adau1452_drv->i2cDrv, &i2cMsg);
}

/**
 * Safeload process
 *
 * @param      num   : data length in word(4 bytes)
 * @param      array : data array
 * @param      addr  : target address
 * @return     none.
 */
/* 
The following steps are necessary for executing a software safeload:
1. Confirm that no safeload operation has been executed in the span of the last audio sample.
2. Write the desired data to the data_SafeLoad, Bit x parameters, starting at data_SafeLoad, Bit 0, and incrementing, as needed, up to a maximum of five parameters.
3. Write the desired starting target address to the address_SafeLoad parameter.
4. Write the number of words to be transferred to the num_SafeLoad parameter. The minimum write length is one word, and the maximum write length is five words.
5. Wait one audio frame for the safeload operation to complete.
*/
static void Adau1452Drv_SafeLoadData(uint8_t num, const uint8_t *array, uint16_t addr)
{
    uint8_t  reg_data[4] = {0};
    ASSERT((num <= SAFE_LOAD_DATA_SIZE_MAX) && (num > 0));

    /* step1: busy check? skip here */
    
    /* step2: write safeload data */
    Adau1452Drv_I2CWrite(SAFE_LOAD_DATA_START_ADD, (num<<2), array);
    
    /* step3: write target addr */
    reg_data[0] = 0;
    reg_data[1] = 0;
    reg_data[2] = (uint8_t)((addr&0xff00) >> 8);
    reg_data[3] = (uint8_t)(addr&0x00ff);
    Adau1452Drv_I2CWrite(SAFE_LOAD_ADD_FOR_TARGET_ADD, 4, reg_data);
    
    /* step4: trigger safeload */
    reg_data[0] = 0;
    reg_data[1] = 0;
    reg_data[2] = 0;
    reg_data[3] = num;
    Adau1452Drv_I2CWrite(SAFE_LOAD_SET_DATA_SIZE_ADD, 4, reg_data);

    /* step 5: wait 1 frame */
    for(num=0; num<50; num++)
    {
        asm("nop");
        asm("nop");
    }
}

/**
 * Construct the DSP driver instance.
 * @return : none
 */
void Adau1452Drv_Ctor(void)
{
    tI2CDevice *p_i2c_device;

    p_adau1452_drv = &adau1452_drv;

    ASSERT( ! p_adau1452_drv->isReady );

    p_i2c_device = (tI2CDevice *) getDevicebyIdAndType(DSP_DEV_ID, I2C_DEV_TYPE, NULL);
    ASSERT(p_i2c_device);

    adau1452_dev_addr = p_i2c_device->devAddress;
    I2CDrv_Ctor((&p_adau1452_drv->i2cDrv), p_i2c_device);
    p_adau1452_drv->isReady = TRUE;
    p_adau1452_drv->i2cEnable = TRUE;
    adau1452_inti_stage = 0;

#ifdef TUNING_ON_ST_EVK_BOARD
    p_adau1452_drv->i2cEnable = FALSE;  // for debug on ST EVK board
#endif
}

/**
 * Exit & clean up the driver.
 */
void Adau1452Drv_Xtor(void)
{
    ASSERT(p_adau1452_drv && p_adau1452_drv->isReady);

    I2CDrv_Xtor( &p_adau1452_drv->i2cDrv );
    p_adau1452_drv->isReady = FALSE;
    p_adau1452_drv->i2cEnable = FALSE;
    adau1452_inti_stage = 0;
}

/**
 * Enable/Disalbe the I2C bus access
 */
void Adau1452Drv_I2cEnable(bool enable)
{
    p_adau1452_drv->i2cEnable = enable;
}

/**
 * Write the dsp program/parameter data.
 */
uint16_t Adau1452Drv_Init(void)
{
    uint8_t total_stages;
    uint16_t delay_time;
    
    ASSERT(p_adau1452_drv->isReady);

    total_stages = ArraySize(dsp_init_stages);
    ASSERT(adau1452_inti_stage < total_stages);

    dsp_init_stages[adau1452_inti_stage].initSectionFunc();
    delay_time = dsp_init_stages[adau1452_inti_stage].delay_time;
    
    adau1452_inti_stage ++;
    if( adau1452_inti_stage == total_stages )
        delay_time = 0;

    return delay_time;
}

/*
 * select the ASRC in source
 */
void Adau1452Drv_AsrcInSource(DspAsrcChannel_t asrc_source)
{
    uint16_t reg_addr;
    uint8_t *p_reg_data;
    uint8_t reg_data_codec[12] = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    uint8_t reg_data_a2b[12] = {0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    uint8_t reg_data_spdif[12] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00};

    ASSERT(asrc_source < DSP_Asrc_MAX);

    reg_addr = MOD_SOURCESWITCH_HWSLEW_NX2_ALG0_STEREOMUXSIGMA3002VOL00_ADDR;

    switch( asrc_source )
    {
    case DSP_Asrc_Codec:
        p_reg_data = reg_data_codec;
        break;
    case DSP_Asrc_A2B:
        p_reg_data = reg_data_a2b;
        break;
    case DSP_Asrc_Spdif:
        p_reg_data = reg_data_spdif;
        break;
    default :
        ASSERT(0);
    }

    Adau1452Drv_SafeLoadData(3, p_reg_data, reg_addr);
}


/* 
 * setup the volume, range from 0 ~ (MAX_VOLUME_STEPS-1)
 * NOTE : the volume control module must be "shared slider" mode.
 */
void Adau1452Drv_SetVolume(uint32_t vol_value)
{
    uint16_t reg_addr;
    uint8_t reg_data[4];
    uint32_t reg_value;
    
    ASSERT(vol_value < MAX_VOLUME_STEPS);

    reg_addr = MOD_VOLUMECONTROL_MASTERVOLUME_ALG0_TARGET_ADDR;
#if 1
    reg_value = vol_value;
    if( reg_value > (DSP_dB_ARRAY_SIZE - 2) )    // exceed the array DSP_Gain_Signed_Format_8_24_Table[]
        reg_value = DSP_dB_ARRAY_SIZE - 2;    
#else
    reg_value = product_volume_curve[vol_value];    // get index dB first
    if( reg_value >= DSP_dB_ARRAY_SIZE )    // exceed the array DSP_Gain_Signed_Format_8_24_Table[]
        reg_value = DSP_dB_ARRAY_SIZE - 1;    
#endif
    reg_value = DSP_Gain_Signed_Format_8_24_Table[reg_value];
    reg_data[0] = (uint8_t)((reg_value >> 24) & 0x00ff);
    reg_data[1] = (uint8_t)((reg_value >> 16) & 0x00ff);
    reg_data[2] = (uint8_t)((reg_value >> 8) & 0x00ff);
    reg_data[3] = (uint8_t)(reg_value & 0x00ff);

//    Adau1452Drv_I2CWrite(reg_addr, 4, reg_data);  // sigma studio use "SafeLoad" command
    Adau1452Drv_SafeLoadData(1, reg_data, reg_addr);
}

/* 
 * bypass enable/disable
 * NOTE : dsp flow related
 */
void Adau1452Drv_BypassEnable(bool enable)
{
    uint16_t reg_addr0;
    uint8_t reg_data_0[4]={0x00, 0x00, 0x00, 0x00};
    uint8_t reg_data_1[4]={0x00, 0x00, 0x00, 0x08};

    reg_addr0 = MOD_BYPASSSWITCH_BP_SWITCH_STEREOMUXSIGMA300NS81INDEX_ADDR;

    if( enable )
    {
        Adau1452Drv_I2CWrite(reg_addr0, 4, reg_data_1);
    }
    else
    {
        Adau1452Drv_I2CWrite(reg_addr0, 4, reg_data_0);
    }
}

/* 
 * channel mute control for production test
 * NOTE : dsp flow related
 */
void Adau1452Drv_ChannelMute(uint32_t param)
{
    uint8_t reg_data_0[4]={0x00, 0x00, 0x00, 0x00};     // mute
    uint8_t reg_data_1[4]={0x00, 0x00, 0x00, 0x01};     // unmute
    uint8_t *p_data;
    uint32_t muted, ch_no;

    ch_no = (param & 0xf0) >> 4;
    muted = param & 0x0f;

    if( muted )
    {
        p_data = reg_data_0;
    }
    else
    {
        p_data = reg_data_1;
    }
    
    switch( ch_no )
    {
    case 0:
        Adau1452Drv_I2CWrite(MOD_MUTE1_W_MUTENOSLEWADAU145XALG1MUTE_ADDR, 4, p_data);
        break;
    case 1:
        Adau1452Drv_I2CWrite(MOD_MUTE1_T_MUTENOSLEWADAU145XALG2MUTE_ADDR, 4, p_data);
        break;
    case 2:
        Adau1452Drv_I2CWrite(MOD_MUTE2_W_MUTENOSLEWADAU145XALG3MUTE_ADDR, 4, p_data);
        break;
    case 3:
        Adau1452Drv_I2CWrite(MOD_MUTE2_T_MUTENOSLEWADAU145XALG4MUTE_ADDR, 4, p_data);
        break;
    case 4:
        Adau1452Drv_I2CWrite(MOD_MUTE3_W_MUTENOSLEWADAU145XALG5MUTE_ADDR, 4, p_data);
        break;
    case 5:
        Adau1452Drv_I2CWrite(MOD_MUTE3_T_MUTENOSLEWADAU145XALG6MUTE_ADDR, 4, p_data);
        break;
    case 6:
        Adau1452Drv_I2CWrite(MOD_MUTE4_W_MUTENOSLEWADAU145XALG7MUTE_ADDR, 4, p_data);
        break;
    case 7:
        Adau1452Drv_I2CWrite(MOD_MUTE4_T_MUTENOSLEWADAU145XALG8MUTE_ADDR, 4, p_data);
        break;
    default :   // all channel
        Adau1452Drv_I2CWrite(MOD_MUTE1_W_MUTENOSLEWADAU145XALG1MUTE_ADDR, 4, p_data);
        Adau1452Drv_I2CWrite(MOD_MUTE1_T_MUTENOSLEWADAU145XALG2MUTE_ADDR, 4, p_data);
        Adau1452Drv_I2CWrite(MOD_MUTE2_W_MUTENOSLEWADAU145XALG3MUTE_ADDR, 4, p_data);
        Adau1452Drv_I2CWrite(MOD_MUTE2_T_MUTENOSLEWADAU145XALG4MUTE_ADDR, 4, p_data);
        Adau1452Drv_I2CWrite(MOD_MUTE3_W_MUTENOSLEWADAU145XALG5MUTE_ADDR, 4, p_data);
        Adau1452Drv_I2CWrite(MOD_MUTE3_T_MUTENOSLEWADAU145XALG6MUTE_ADDR, 4, p_data);
        Adau1452Drv_I2CWrite(MOD_MUTE4_W_MUTENOSLEWADAU145XALG7MUTE_ADDR, 4, p_data);
        Adau1452Drv_I2CWrite(MOD_MUTE4_T_MUTENOSLEWADAU145XALG8MUTE_ADDR, 4, p_data);
        break;
    }
}

#if 0
// implement the delay setting later
/* 
 * Delay1 Module
 * NOTE : dsp flow related
 */
void Adau1452Drv_SetupDelay1(uint16_t delay)
{
    uint16_t reg_addr;
    uint8_t reg_data[4]={0x00, 0x00, 0x00, 0x00};

    ASSERT( delay <= DSP_MAX_DELAY_SAMPLES );

    reg_addr = MOD_SYSTEMTUNING_DELAY1_DELAYAMT_ADDR;

    reg_data[2] = (uint8_t)((delay >> 8) & 0xff);
    reg_data[3] = (uint8_t)(delay & 0xff);

    Adau1452Drv_I2CWrite(reg_addr, 4, reg_data);
}

/* 
 * Delay2 Module
 * NOTE : dsp flow related
 */
void Adau1452Drv_SetupDelay2(uint16_t delay)
{
    uint16_t reg_addr;
    uint8_t reg_data[4]={0x00, 0x00, 0x00, 0x00};

    ASSERT( delay <= DSP_MAX_DELAY_SAMPLES );

    reg_addr = MOD_SYSTEMTUNING_DELAY2_DELAYAMT_ADDR;

    reg_data[2] = (uint8_t)((delay >> 8) & 0xff);
    reg_data[3] = (uint8_t)(delay & 0xff);

    Adau1452Drv_I2CWrite(reg_addr, 4, reg_data);
}

/* 
 * Delay3 Module
 * NOTE : dsp flow related
 */
void Adau1452Drv_SetupDelay3(uint16_t delay)
{
    uint16_t reg_addr;
    uint8_t reg_data[4]={0x00, 0x00, 0x00, 0x00};

    ASSERT( delay <= DSP_MAX_DELAY_SAMPLES );

    reg_addr = MOD_SYSTEMTUNING_DELAY3_DELAYAMT_ADDR;

    reg_data[2] = (uint8_t)((delay >> 8) & 0xff);
    reg_data[3] = (uint8_t)(delay & 0xff);

    Adau1452Drv_I2CWrite(reg_addr, 4, reg_data);
}

/* 
 * Delay4 Module
 * NOTE : dsp flow related
 */
void Adau1452Drv_SetupDelay4(uint16_t delay)
{
    uint16_t reg_addr;
    uint8_t reg_data[4]={0x00, 0x00, 0x00, 0x00};

    ASSERT( delay <= DSP_MAX_DELAY_SAMPLES );

    reg_addr = MOD_SYSTEMTUNING_DELAY4_DELAYAMT_ADDR;

    reg_data[2] = (uint8_t)((delay >> 8) & 0xff);
    reg_data[3] = (uint8_t)(delay & 0xff);

    Adau1452Drv_I2CWrite(reg_addr, 4, reg_data);
}
#endif

/* 
 * Get Dsp version from dsp flow
 * return value : 32.0 format
 * e.g 123 = Ver1.2.3, 5 = Ver0.0.5
 */
uint32_t Adau1452Drv_DspVersion(void)
{
    uint32_t version;
#if 1
    version = MOD_VERSION_DCINPALG145X1VALUE_FIXPT;
#else
    uint16_t reg_addr;
    uint8_t reg_data[4];

    reg_addr = MOD_VERSION_DCINPALG145X1VALUE_ADDR;

    Adau1452Drv_I2CRead(reg_addr, 4, reg_data);

//    version = (reg_data[0]<<24) | (reg_data[1]<<16) | (reg_data[2]<<8) | reg_data[3];
    version = (reg_data[2]<<8) | reg_data[3];   // only LSB 16bits.
#endif
    return version;
}

/*
uint32_t Adau1452Drv_CodecSignalDetect(void)
{
    uint16_t reg_addr;
    uint8_t reg_data[4];

    reg_addr = MOD_SIGNALDETECT_READBACK_CODEC_READBACKALGSIGMA3002_ADDR;

    Adau1452Drv_I2CRead(reg_addr, 4, reg_data);

    return (*(uint32_t *)reg_data);
}

uint32_t Adau1452Drv_A2BSignalDetect(void)
{
    uint16_t reg_addr;
    uint8_t reg_data[4];

    reg_addr = MOD_SIGNALDETECT_READBACK_A2B_READBACKALGSIGMA3003_ADDR;

    Adau1452Drv_I2CRead(reg_addr, 4, reg_data);

    return (*(uint32_t *)reg_data);
}

uint32_t Adau1452Drv_SpdifSignalDetect(void)
{
    uint16_t reg_addr;
    uint8_t reg_data[4];

    reg_addr = MOD_SIGNALDETECT_READBACK_SPDIF_READBACKALGSIGMA3004_ADDR;

    Adau1452Drv_I2CRead(reg_addr, 4, reg_data);

    return (*(uint32_t *)reg_data);
}
*/

#ifdef HAS_AUDIO_MUTE_CHANNEL
void Adau1452Drv_EnableTestTone(bool enable)
{
    uint8_t tone_on[4]={0x01, 0x00, 0x00, 0x00};     // test tone on
    uint8_t tone_off[4]={0x00, 0x00, 0x00, 0x00};     // test tone off
    uint8_t reg_data_0[4]={0x00, 0x00, 0x00, 0x00};     // audio in channel
    uint8_t reg_data_1[4]={0x00, 0x00, 0x00, 0x02};     // test tone channel

    if( enable )
    {
        Adau1452Drv_I2CWrite(MOD_TESTSIGNAL_W_NOISE_ENABLENOISE_ADDR, 4, tone_on);
        Adau1452Drv_I2CWrite(MOD_TESTSIGNAL_TESTSIGNAL_SWITCH_STEREOMUXSIGMA300NS1INDEX_ADDR, 4, reg_data_1);
    }
    else
    {
        Adau1452Drv_I2CWrite(MOD_TESTSIGNAL_W_NOISE_ENABLENOISE_ADDR, 4, tone_off);
        Adau1452Drv_I2CWrite(MOD_TESTSIGNAL_TESTSIGNAL_SWITCH_STEREOMUXSIGMA300NS1INDEX_ADDR, 4, reg_data_0);
    }
}
#endif

#ifdef HAS_TEMPERATURE_MONITOR
void Adau1452_UpdateNtcValue()
{
#if IS_DSP_FROM_BnO
    NTCInfo_t *p_ntc_value;
    uint16_t ntc_addr[NTC_INDEX_MAX];
    NTCIndex_t index;
    float ff;
    uint32_t ntc_8_24;
    uint8_t reg_value[4];

    // fill the register address
    ntc_addr[NTC_INDEX_DSP] = MOD_TEMPPROTECTION_TEMPDSP_DCINPALG145X23VALUE_ADDR;
    ntc_addr[NTC_INDEX_AMP] = MOD_TEMPPROTECTION_TEMPAMPLIFIERPCB_DCINPALG145X22VALUE_ADDR;
    ntc_addr[NTC_INDEX_PSU] = MOD_TEMPPROTECTION_TEMPPOWERSUPPLYPCB_DCINPALG145X21VALUE_ADDR;

    p_ntc_value = SystemDrv_GetNtcValue();

    for(index=NTC_INDEX_START; index<NTC_INDEX_MAX; index++)
    {
        ff = (float)p_ntc_value->degree / 100;
        ntc_8_24 = Adau1452Drv_FloatTo8_24Data(ff);
        reg_value[0] = (uint8_t)((ntc_8_24>>24) & 0xff);
        reg_value[1] = (uint8_t)((ntc_8_24>>16) & 0xff);
        reg_value[2] = (uint8_t)((ntc_8_24>>8) & 0xff);
        reg_value[3] = (uint8_t)(ntc_8_24 & 0xff);
//        printf("\n\rdeg=%d, 8_24=0x%x.", p_ntc_value->degree, ntc_8_24);
        p_ntc_value ++;
        Adau1452Drv_I2CWrite(ntc_addr[index], 4, reg_value);
    }
#endif    
}
#endif

#ifdef HAS_SOUNDMODE_UPDATE
static void Adau1452_WriteLittleEndian(uint16_t addr, uint32_t value)
{
    uint8_t dsp_data[4];

    dsp_data[0] = (uint8_t)((value>>24) & 0xff);
    dsp_data[1] = (uint8_t)((value>>16) & 0xff);
    dsp_data[2] = (uint8_t)((value>>8) & 0xff);
    dsp_data[3] = (uint8_t)((value) & 0xff);

    Adau1452Drv_I2CWrite(addr, 4, dsp_data);
}

bool Adau1452Drv_UpdateGainAndDelay(Proto_SoundWall_GainAndDelay * p_gain_and_delay)
{
#if IS_DSP_FROM_BnO
    uint32_t *p_u32;
    
    // save the parameter
    p_u32 = (uint32_t *)p_gain_and_delay;
    p_u32 ++;
    memcpy(&sound_mode_gain_and_delay, p_u32, sizeof(SoundModeGainAndDelay_t));
    // update gain
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_GAIN_L_1_DCINPALG145X12VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_gain_L_1);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_GAIN_L_2_DCINPALG145X13VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_gain_L_2);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_GAIN_L_3_DCINPALG145X14VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_gain_L_3);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_GAIN_L_4_DCINPALG145X15VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_gain_L_4);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_GAIN_R_1_DCINPALG145X17VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_gain_R_1);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_GAIN_R_2_DCINPALG145X18VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_gain_R_2);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_GAIN_R_3_DCINPALG145X19VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_gain_R_3);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_GAIN_R_4_DCINPALG145X20VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_gain_R_4);
    // update delay
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_DELAY_L_1_DCINPALG145X4VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_delay_L_1);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_DELAY_L_2_DCINPALG145X5VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_delay_L_2);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_DELAY_L_3_DCINPALG145X6VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_delay_L_3);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_DELAY_L_4_DCINPALG145X7VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_delay_L_4);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_DELAY_R_1_DCINPALG145X8VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_delay_R_1);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_DELAY_R_2_DCINPALG145X9VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_delay_R_2);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_DELAY_R_3_DCINPALG145X10VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_delay_R_3);
    Adau1452_WriteLittleEndian(MOD_IN2XO_MT_TW_DELAY_R_4_DCINPALG145X11VALUE_ADDR, sound_mode_gain_and_delay.MT_TW_delay_R_4);
    // Mute_L_R
    Adau1452_WriteLittleEndian(MOD_GAINGLOBALFILTSCOMPRESSOR_MUTE_L_R_ALG0_MUTE_ADDR, sound_mode_gain_and_delay.MUTE_L_R);
#endif    

    return TRUE;
}
#endif

