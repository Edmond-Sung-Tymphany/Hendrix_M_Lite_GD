/**
* @file tas5760Drv.c
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 16-Dec-2015
* @copyright Tymphany Ltd.
*/
#include "product.config"
#include "attachedDevices.h"
#include "trace.h"
#include "GPIODrv.h"
#include "I2CDrv.h"
#include "AmpDrvTas5760.h"
#include "tas5760InitTab.h"
#include "bsp.h"
#include "GpioDrv.h"
#if defined(HAS_SYSTEM_CONTROL) && defined(HAS_I2C_BUS_DETECT)
#include "SystemDrv.h"
#endif

/****************************************************
 * Definition
 ****************************************************/
#ifdef AMDRV_DEBUG
    #define AMPDRV_DEBUG_MSG TP_PRINTF
#else
    #define AMPDRV_DEBUG_MSG(...)
#endif

#define TAS5760_REGISTER_ADDR_LEN   (1)
#define TAS5760_REGISTER_VALUE_LEN  (1)
/* include register + data, except for slave address */
#define TAS5760_REGISTER_LEN (TAS5760_REGISTER_ADDR_LEN + TAS5760_REGISTER_VALUE_LEN)

#define TAS5760_POWER_CTRL_ADDR         0x01
/*******************  Bit definition for Power Control register  *******************/
#define TAS5760_DIG_CLIP_LEV_BITS     ((uint8)0xFC)
#define TAS5760_SLEEP_MODE_BITS       ((uint8)0x02)
#define TAS5760_SPEAKER_SHUTDOWN_BITS ((uint8)0x01)

#define TAS5760_VOL_CTRL_CONF_ADDR          ((uint8)0x03)
#define TAS5760_FAULT_CONF_ERROR_STAT_ADDR  ((uint8)0x08)
#define TAS5760_ERROR_STAT_MASK             ((uint8)0x07)

/*******************  Bit definition for Volume Control Configuration register  ****/
#define TAS5760_VOL_FADE_ENABLE_BITS        ((uint8)0x80)
#define TAS5760_VOL_CTRL_CONF_RESERVED_BITS ((uint8)0x7C)
#define TAS5760_SOFT_MUTE_R_CH_BITS         ((uint8)0x02)
#define TAS5760_SOFT_MUTE_L_CH_BITS         ((uint8)0x01)

#define TAS5760_DIGITAL_CONTROL_CTRL_ADDR 0x02
/***********  Bit definition for digital control register  ****/
#define TAS5760_HPF_NOT_BYPASS_BITS      ((uint8)0x00)  //high pass filter is not bypass
#define TAS5760_HPF_BYPASS_BITS          ((uint8)0x80)  //high pass filter is bypass
#define TAS5760_DIGITAL_BOOST_0DB_BITS   ((uint8)0x00)  
#define TAS5760_DIGITAL_BOOST_6DB_BITS   ((uint8)0x10)  
#define TAS5760_DIGITAL_BOOST_12DB_BITS  ((uint8)0x20)  
#define TAS5760_DIGITAL_BOOST_18DB_BITS  ((uint8)0x30)  
#define TAS5760_SERIAL_PORT_SINGLE_FS_BITS  ((uint8)0x00)  //Serial Audio Port is single speed sample rates (32kHz, 44.1kHz, 48kHz)
#define TAS5760_SERIAL_PORT_DOUBLE_FS_BITS  ((uint8)0x08)  //Serial Audio Port is double speed sample rates (64kHz, 88.2kHz, 96kHz)
#define TAS5760_SERIAL_PORT_24BIT_RIGHT_JUSTIFIED_BITS    ((uint8)0x00)
#define TAS5760_SERIAL_PORT_20BIT_RIGHT_JUSTIFIED_BITS    ((uint8)0x01)
#define TAS5760_SERIAL_PORT_18BIT_RIGHT_JUSTIFIED_BITS    ((uint8)0x02)
#define TAS5760_SERIAL_PORT_16BIT_RIGHT_JUSTIFIED_BITS    ((uint8)0x03)
#define TAS5760_SERIAL_PORT_I2S_BITS                      ((uint8)0x04)
#define TAS5760_SERIAL_PORT_16_24BIT_LEFT_JUSTIFIED_BITS  ((uint8)0x05)

/* 
* Please refer to below info from TAS5760 spec to set volume.
* Left/Right Channel Volume Control [7:0] (R/W) 11001111
* Channel Volume is +24 dB 11111111
* Channel Volume is +23.5 dB 11111110
* Channel Volume is +23.0 dB 11111101
* ... ...
* Channel Volume is 0 dB 11001111
* ... ...
* Channel Volume is -100 dB 00000111
* Any setting less than 00000111 places the channel in Mute < 00000111
*/
#define TAS5760_LEFT_CH_VOL_CTRL_ADDR     0x04
#define TAS5760_RIGHT_CH_VOL_CTRL_ADDR    0x05
/***********  Bit definition for volume register  ****/
#define TAS5760_MAX_VOL ((uint8)0xFF)  //+24dB
#define TAS5760_0DB_VOL ((uint8)0xCF)  //+0dB
#define TAS5760_MIN_VOL ((uint8)0x07)  //-100dB


#define TAS5760_ANALOG_CONTROL_CTRL_ADDR  0x06
/***********  Bit definition for Analog Control register  ****/
#define TAS5760_PBTL_MODE_BITS                   ((uint8)0x80)
#define TAS5760_PWM_RATE_BITS                    ((uint8)0x70)
#define TAS5760_ANALOG_GAIN_BITS                 ((uint8)0x0C)
#define TAS5760_PBTL_CH_SLE_BITS                 ((uint8)0x02)


#define TAS5760_FAULT_CONF_AND_ERR_STATUS_ADDR   0x08
/***********  Bit definition for Fault Configuration and Error Status register  ****/
#define TAS5760_FAULT_CONF_AND_ERR_STATUS_RESERVED_BITS ((uint8)0xC0)
#define TAS5760_OCE_THRESHOLD_BITS                      ((uint8)0x30)
#define TAS5760_CLK_ERROR_STATUS_BITS                   ((uint8)0x08)
#define TAS5760_OVER_CURRENT_ERROR_STATUS_BITS          ((uint8)0x04)
#define TAS5760_OUTPUT_DC_ERROR_STATUS_BITS             ((uint8)0x02)
#define TAS5760_OVER_TEMP_ERROR_STATUS_BITS             ((uint8)0x01)


/****************************************************
 * Function Prototype
 ****************************************************/
static void AudioAmpDrv_Init(cAudioAmpDrv * me);
static void AudioAmpDrv_I2cWrite(cAudioAmpDrv * me, uint8 bytes, const uint8 *pData);
static void AudioAmpDrv_I2cRead(cAudioAmpDrv * me, uint32 regAddr, uint16 bytes, uint8 *pData);



/****************************************************
 * Function Implemenation
 ****************************************************/
void AudioAmpDrv_Ctor(cAudioAmpDrv * me, cI2CDrv *pI2cObj)
{
    me->pAudioAmpI2cObj = pI2cObj;
    me->deviceI2cAddr   = pI2cObj->pConfig->devAddress;
    me->isCreated       = TRUE;
    me->i2cEnable       = TRUE;

    ASSERT(pI2cObj->pConfig->regAddrLen==REG_LEN_8BITS);
    I2CDrv_Ctor(pI2cObj, me->pAudioAmpI2cObj->pConfig);
    ASSERT(me->pAudioAmpI2cObj);
    
    AudioAmpDrv_Init(me);
}

void AudioAmpDrv_Xtor(cAudioAmpDrv * me)
{
    AudioAmpDrv_setShutdown(me, TRUE);
    I2CDrv_Xtor(me->pAudioAmpI2cObj);
    me->pAudioAmpI2cObj = NULL;
    me->isCreated  = FALSE;
}

static void AudioAmpDrv_setRegBits(cAudioAmpDrv * me, uint8 regAddr, uint8 bitsMask, uint8 value)
{
    uint8 reg[TAS5760_REGISTER_LEN] = {0, 0}; //first byte: reg addr   second byte: data
    uint8 i = 0;
    ASSERT(me && me->isCreated);
    reg[0] = regAddr;
    //step1: readback register value.
    AudioAmpDrv_I2cRead(me, reg[0], sizeof(reg[1]), &reg[1]);
    //step2: clear register bits(old value) accordingly.
    TYM_CLR_BIT(reg[1], bitsMask);
    //step3: set register bits(new value) accordingly.
    for(i=0; i < TYM_SIZE_OF_IN_BIT_NUM(i); i++ )
    {
        if(bitsMask & (1<<i))
        {
            TYM_SET_BIT(reg[1], value<<i);
            break;
        }
    }
    AudioAmpDrv_I2cWrite(me, TAS5760_REGISTER_LEN, (const uint8*)&reg);
}

void AudioAmpDrv_setShutdown(cAudioAmpDrv * me, bool enable)
{
    uint8 powerContol[TAS5760_REGISTER_LEN] = {TAS5760_POWER_CTRL_ADDR, 0}; //first byte: reg addr   second byte: data
    ASSERT(me && me->isCreated);
    //readback power control register value.
    AudioAmpDrv_I2cRead(me, powerContol[0], sizeof(powerContol[1]), &powerContol[1]);
    //set sleep mode bit accordingly.
    if(enable)
    {
        TYM_CLR_BIT(powerContol[1], TAS5760_SPEAKER_SHUTDOWN_BITS);
    }
    else
    {
        TYM_SET_BIT(powerContol[1], TAS5760_SPEAKER_SHUTDOWN_BITS);
    }
    AudioAmpDrv_I2cWrite(me, TAS5760_REGISTER_LEN, (const uint8*)powerContol);
}

void AudioAmpDrv_setSleepMode(cAudioAmpDrv * me, bool enable)
{
    uint8 powerContol[TAS5760_REGISTER_LEN] = {TAS5760_POWER_CTRL_ADDR, 0}; //first byte: reg addr   second byte: data
    ASSERT(me && me->isCreated);
    //readback power control register value.
    AudioAmpDrv_I2cRead(me, powerContol[0], sizeof(powerContol[1]), &powerContol[1]);
    //set sleep mode bit accordingly.
    if(enable)
    {
        TYM_SET_BIT(powerContol[1], TAS5760_SLEEP_MODE_BITS);
    }
    else
    {
        TYM_CLR_BIT(powerContol[1], TAS5760_SLEEP_MODE_BITS);
    }
    AudioAmpDrv_I2cWrite(me, TAS5760_REGISTER_LEN, (const uint8*)powerContol);
}

void AudioAmpDrv_setSoftMute(cAudioAmpDrv * me, bool enable)
{
    //AMPDRV_DEBUG_MSG("AudioAmpDrv_setSoftMute: mute=%d\r\n", enable);
    AudioAmpDrv_setRegBits(me, TAS5760_VOL_CTRL_CONF_ADDR, TAS5760_SOFT_MUTE_L_CH_BITS, enable);
    AudioAmpDrv_setRegBits(me, TAS5760_VOL_CTRL_CONF_ADDR, TAS5760_SOFT_MUTE_R_CH_BITS, enable);
}

void AudioAmpDrv_setSoftMuteLeftChannel(cAudioAmpDrv * me, bool enable)
{
    AudioAmpDrv_setRegBits(me, TAS5760_VOL_CTRL_CONF_ADDR, TAS5760_SOFT_MUTE_L_CH_BITS, enable);
}

void AudioAmpDrv_setSoftMuteRightChannel(cAudioAmpDrv * me, bool enable)
{
    AudioAmpDrv_setRegBits(me, TAS5760_VOL_CTRL_CONF_ADDR, TAS5760_SOFT_MUTE_R_CH_BITS, enable);
}

void AudioAmpDrv_setPbtlMode(cAudioAmpDrv * me, bool enable)
{
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setPbtlMode: enable=0x%02x\r\n", enable);
    AudioAmpDrv_setRegBits(me, TAS5760_ANALOG_CONTROL_CTRL_ADDR, TAS5760_PBTL_MODE_BITS, enable);
}

void AudioAmpDrv_setPwmRate(cAudioAmpDrv * me, eAdcPwmRate value)
{
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setPwmRate: value=0x%02x\r\n", value);
    AudioAmpDrv_setRegBits(me, TAS5760_ANALOG_CONTROL_CTRL_ADDR, TAS5760_PWM_RATE_BITS, value);
}

void AudioAmpDrv_setAnalogGain(cAudioAmpDrv * me, eAnalogGain value)
{
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setAnalogGain: value=0x%02x\r\n", value);
    AudioAmpDrv_setRegBits(me, TAS5760_ANALOG_CONTROL_CTRL_ADDR, TAS5760_ANALOG_GAIN_BITS, value);
}

void AudioAmpDrv_setVol(cAudioAmpDrv * me, uint8 vol)
{
    //AMPDRV_DEBUG_MSG("AudioAmpDrv_setVol: vol=0x%02x\n\r", vol);
    
    uint8 volContol[TAS5760_REGISTER_LEN] = {TAS5760_LEFT_CH_VOL_CTRL_ADDR, 0}; //first byte: reg addr   second byte: data
    ASSERT(me->isCreated && vol >= TAS5760_MIN_VOL && vol <= TAS5760_MAX_VOL);
    volContol[1] = vol;
    
    //set left channel vol
    AudioAmpDrv_I2cWrite(me, TAS5760_REGISTER_LEN, (const uint8*)volContol);
    
    //set right channel vol
    volContol[0] = TAS5760_RIGHT_CH_VOL_CTRL_ADDR;
    AudioAmpDrv_I2cWrite(me, TAS5760_REGISTER_LEN, (const uint8*)volContol);
}

void AudioAmpDrv_SetVolGainByDB(cAudioAmpDrv * me, int8_t gain_dB)
{
    uint8 volContol[TAS5760_REGISTER_LEN] = {TAS5760_LEFT_CH_VOL_CTRL_ADDR, 0}; //first byte: reg addr   second byte: data
    int32_t vol_reg;
    ASSERT( (-100<=gain_dB) && (gain_dB<=24) );
    vol_reg = TAS5760_0DB_VOL;
    vol_reg += gain_dB * 2;
    volContol[1] = (uint8_t)vol_reg;
    
    //set left channel vol
    AudioAmpDrv_I2cWrite(me, TAS5760_REGISTER_LEN, (const uint8*)volContol);
    
    //set right channel vol
    volContol[0] = TAS5760_RIGHT_CH_VOL_CTRL_ADDR;
    AudioAmpDrv_I2cWrite(me, TAS5760_REGISTER_LEN, (const uint8*)volContol);
}

void AudioAmpDrv_setClip(cAudioAmpDrv * me, uint32 clipLevel)
{
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setClip: clipLevel=0x%02x\r\n", clipLevel);
}

//TODO: split this function to setPbtlMode(), setAnalogGain(), setPwmRate()
//TODO: when select left channel for PBTL(ex.0x91), amplifer still use right channle, is it a bug?
void AudioAmpDrv_setAnalogControl(cAudioAmpDrv * me, uint8 value)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setAnalogControl: value=0x%02x\r\n", value);
    
    uint8 buf[TAS5760_REGISTER_LEN] = {TAS5760_ANALOG_CONTROL_CTRL_ADDR, value}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, TAS5760_REGISTER_LEN, (const uint8*)buf);
}

void AudioAmpDrv_setDigitalControl(cAudioAmpDrv * me, uint8 value)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);
     
    uint8 buf[TAS5760_REGISTER_LEN] = {TAS5760_DIGITAL_CONTROL_CTRL_ADDR, value}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, TAS5760_REGISTER_LEN, (const uint8*)buf);
}

void AudioAmpDrv_getErrorStatus(cAudioAmpDrv * me, bool* isError)
{
    uint8 FaultConfErrorStat[TAS5760_REGISTER_LEN] = {TAS5760_FAULT_CONF_ERROR_STAT_ADDR, 0}; //first byte: reg addr   second byte: data
    ASSERT(me && me->isCreated);

    AudioAmpDrv_I2cRead(me, FaultConfErrorStat[0], sizeof(FaultConfErrorStat[1]), &FaultConfErrorStat[1]);

    *isError = (bool)((FaultConfErrorStat[1] & TAS5760_ERROR_STAT_MASK));
}

static void AudioAmpDrv_Init(cAudioAmpDrv * me)
{
    int i = 0;
    ASSERT(me && me->isCreated);
    const cfg_reg *r = registers;
    int n = ArraySize(registers);
    while (i < n) {
        switch (r[i].command) {
        case CFG_META_SWITCH:
            // Used in legacy applications.  Ignored here.
            break;
        case CFG_META_DELAY:
            BSP_BlockingDelayMs(r[i].param);
            break;
        case CFG_META_BURST:
            AudioAmpDrv_I2cWrite(me, r[i].param, (unsigned char *)&r[i+1]);
            i += r[i].param/2 +1;
            break;
        default:
            AudioAmpDrv_I2cWrite(me, TAS5760_REGISTER_LEN, (unsigned char *)&r[i]);
            break;
        }
        i++;
    }
    
    AudioAmpDrv_setSoftMute(me, /*muteEnable:*/TRUE);
    AudioAmpDrv_setVol(me, TAS5760_0DB_VOL);
    
    uint8 digital_control= TAS5760_DIGITAL_BOOST_0DB_BITS | \
                           TAS5760_HPF_NOT_BYPASS_BITS | \
                           TAS5760_SERIAL_PORT_SINGLE_FS_BITS | \
                           TAS5760_SERIAL_PORT_I2S_BITS;
    AudioAmpDrv_setDigitalControl(me, digital_control);
}


void AudioAmpDrv_dumpReg(cAudioAmpDrv * me, char *name)
{
    ASSERT(me && me->isCreated);
    
    uint8 reg, data;
    AMPDRV_DEBUG_MSG("%s:\r\n", name);
    for(reg=0 ; reg<=17 ; reg++)
    {
        data= 0;
        AudioAmpDrv_I2cRead(me, reg, sizeof(data), &data);
        if(reg==8) {
          AMPDRV_DEBUG_MSG("AMP REG[0x%02x]: 0x%02x (ClockError=%d)\r\n", reg, data, (data&0x8)>>3);
        }
        else {
          AMPDRV_DEBUG_MSG("AMP REG[0x%02x]: 0x%02x\r\n", reg, data);
        }
    }
}

void AudioAmpDrv_ErrorStatus(cAudioAmpDrv * me)
{
    uint8_t reg=8, data=0;
    AudioAmpDrv_I2cRead(me, reg, sizeof(data), &data);
    if( data != 0x00 )
    {
        TP_PRINTF("\n\rTAS5760 error status : 0x%x .", data);
    }
}

/**
 * Enable/Disalbe Amp I2C access
 */
void AudioAmpDrv_I2cEnable(cAudioAmpDrv *me, bool i2cEnable)
{
    me->i2cEnable= i2cEnable;
}

static void AudioAmpDrv_I2cWrite(cAudioAmpDrv * me, uint8 bytes, const uint8 *pData)
{
    if(!me->i2cEnable) {
        return;
    }
    
    tI2CMsg i2cMsg=
    {
        .devAddr = me->deviceI2cAddr,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)pData
    };
    bool ret= I2CDrv_MasterWrite(me->pAudioAmpI2cObj, &i2cMsg);
#ifdef HAS_I2C_BUS_DETECT
    if( ret != TP_SUCCESS )
    {
        SystemDrv_SetI2cBusStatus(I2C_ERROR_AMP);
    }
#else
    ASSERT(ret==TP_SUCCESS);
#endif
}

static void AudioAmpDrv_I2cRead(cAudioAmpDrv * me, uint32 regAddr, uint16 bytes, uint8 *pData)
{
    if(!me->i2cEnable) {
        return;
    }
    
    tI2CMsg i2cMsg =
    {
        .devAddr = me->deviceI2cAddr,
        .regAddr = regAddr,
        .length  = bytes,
        .pMsg    = pData
    };
    I2CDrv_MasterRead(me->pAudioAmpI2cObj, &i2cMsg);
}


