/**
* @file RT9114BDrv.c
* @brief The devices attached to the product.
* @author Zorro Lu
* @date 14-Aug-2019
* @copyright Tymphany Ltd.
*/
#include "product.config"
#include "attachedDevices.h"
#include "trace.h"
#include "GPIODrv.h"
#include "I2CDrv.h"
#include "AmpDrvRT9120S.h"
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

/***********  Bit definition for SW RESET register  ****/
//#define RT9114B_SW_RESET_ADDR   0x80
#define RT9120S_SW_RESET_ADDR   0x40

/***********  Bit definition for Max Vol register  ****/
//#define RT9114B_MAX_VOL_ADDR   0x07
#define RT9120S_MAX_VOL_ADDR   0x20

/***********  Bit definition for Class D gain setting = 4.5 gain register  ****/
#define RT9120S_CLASS_D_GAIN_SETTING_ADDR   0x07
#define RT9120S_INIT_ANALOG_GAIN                   ((uint8)0x35) //15dB

/***********  Bit definition for Amp turn on register  ****/
#define RT9120S_AMP_TURN_ON_ADDR   0x05
#define RT9120S_AMP_TURN_ON        ((uint8)0x80)

/***********  Bit definition for Amp mute register  ****/
#define RT9120S_AMP_MUTE_ADDR   0x0A

/***********  Bit definition for Amp err mask  ****/
#define RT9120S_AMP_ERRMASK_ADDR   0x11

/****************************************************
 * Function Prototype
 ****************************************************/
static void AudioAmpDrv_Init(cAudioAmpDrv * me);
static void AudioAmpDrv_I2cWrite(cAudioAmpDrv * me, uint8 bytes, const uint8 *pData);
static void AudioAmpDrv_I2cRead(cAudioAmpDrv * me, uint32 regAddr, uint16 bytes, uint8 *pData);
static void AudioAmpDrv_SwReset(cAudioAmpDrv * me);
static void AudioAmpDrv_MaxVol(cAudioAmpDrv * me);
//static void AudioAmpDrv_setPostIDFgain(cAudioAmpDrv * me);
static void AudioAmpDrv_setClassDgain(cAudioAmpDrv * me, uint8 value);
static void AudioAmpDrv_AmpTurnOn(cAudioAmpDrv * me, uint8 value);
static void AudioAmpDrv_AmpDelay(cAudioAmpDrv * me);
static void AudioAmpDrv_AmpUnmute(cAudioAmpDrv * me);
static void AudioAmpDrv_AmpMute(cAudioAmpDrv * me);
static void AudioAmpDrv_SwReset(cAudioAmpDrv * me);
static void AudioAmpDrv_AmpShutDown(cAudioAmpDrv * me);
static void AudioAmpDrv_AmpInitSettings_0x01(cAudioAmpDrv * me);
static void AudioAmpDrv_AmpInitSettings_0x02(cAudioAmpDrv * me);
static void AudioAmpDrv_AmpInitSettings_0x15(cAudioAmpDrv * me);
static void AudioAmpDrv_AmpInitSettings_0x64(cAudioAmpDrv * me);
static void AudioAmpDrv_AmpInitSettings_0x63(cAudioAmpDrv * me);
static void AudioAmpDrv_AmpInitSettings_0x65(cAudioAmpDrv * me);


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
    AudioAmpDrv_AmpShutDown(me);  
    I2CDrv_Xtor(me->pAudioAmpI2cObj);
    me->pAudioAmpI2cObj = NULL;
    me->isCreated  = FALSE;
}

void AudioAmpDrv_setSoftMute(cAudioAmpDrv * me, bool enable)
{
     if(enable)
     {
        AudioAmpDrv_AmpMute(me);
     }
     else
     {
        AudioAmpDrv_AmpUnmute(me);
     }
}

static void AudioAmpDrv_Init(cAudioAmpDrv * me)
{
    AudioAmpDrv_SwReset(me); //0x40, 0x80
    BSP_BlockingDelayMs(10);//delay 10ms   
    
    //AudioAmpDrv_AmpInitSettings_0x63(me); //0x63, 0xDE
    AudioAmpDrv_setClassDgain(me, RT9120S_INIT_ANALOG_GAIN); //0x07, 0x35 (15dB)
    //AudioAmpDrv_AmpInitSettings_0x65(me); //0x65, 0x66
    
    //AudioAmpDrv_AmpInitSettings_0x01(me); //0x01, 0x50
    //AudioAmpDrv_AmpInitSettings_0x02(me); //0x02, 0x00
    
    AudioAmpDrv_MaxVol(me); //set the volume from mute to 0dB //0x20, 0x01 0x80
    //AudioAmpDrv_AmpInitSettings_0x15(me); //0x15, 0x40
    
    //AudioAmpDrv_AmpInitSettings_0x64(me); //0x64, 0x39
    
    AudioAmpDrv_AmpTurnOn(me, RT9120S_AMP_TURN_ON); //0x05, 0x80
}

static void AudioAmpDrv_AmpInitSettings_0x01(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {0x01, 0x50}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_AmpInitSettings_0x02(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {0x02, 0x00}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_AmpInitSettings_0x15(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {0x15, 0x40}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_AmpInitSettings_0x64(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {0x64, 0x39}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_AmpInitSettings_0x63(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {0x63, 0xDE}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_AmpInitSettings_0x65(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {0x65, 0x66}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_MaxVol(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);
    /*Master volume = 0dB*/
    uint8 buf[3] = {RT9120S_MAX_VOL_ADDR, 0x01,0x80}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 3, (const uint8*)buf);
}
#if 0
static void AudioAmpDrv_setPostIDFgain(cAudioAmpDrv * me) //Nick: not used in RT2120S
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);
    /*Post IDF gain setting = 2dB*/
    uint8 buf[5] = {0x62, 0x00, 0x00,0x00,0xA0}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 5, (const uint8*)buf);
}
#endif
static void AudioAmpDrv_setClassDgain(cAudioAmpDrv * me, uint8 value)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {RT9120S_CLASS_D_GAIN_SETTING_ADDR, value}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_AmpShutDown(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {RT9120S_AMP_TURN_ON_ADDR, 0xC0}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_AmpTurnOn(cAudioAmpDrv * me, uint8 value)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {RT9120S_AMP_TURN_ON_ADDR, value}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_AmpDelay(cAudioAmpDrv * me) //Nick: not used
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {0x0A, 0x00}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_AmpUnmute(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);

    uint8 buf[2] = {RT9120S_AMP_MUTE_ADDR, 0x00}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_AmpMute(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);

    uint8 buf[2] = {RT9120S_AMP_MUTE_ADDR, 0x30}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

void AudioAmpDrv_setSoftMuteLeftChannel(cAudioAmpDrv * me, bool enable) //Nick++
{
    uint8 buf[2] = 0;
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);
    
    if(enable)
    {
        buf[0] = RT9120S_AMP_MUTE_ADDR;
        buf[1] = 0x20;
    }
    else
    {
        buf[0] = RT9120S_AMP_MUTE_ADDR;
        buf[1] = 0x00;
    }
 
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

void AudioAmpDrv_setSoftMuteRightChannel(cAudioAmpDrv * me, bool enable) //Nick++
{
    uint8 buf[2] = 0;
    ASSERT(me && me->isCreated);
    AMPDRV_DEBUG_MSG("AudioAmpDrv_setDigitalControl: value=0x%02x\r\n", value);
    
    if(enable)
    {
        buf[0] = RT9120S_AMP_MUTE_ADDR;
        buf[1] = 0x10;
    }
    else
    {
        buf[0] = RT9120S_AMP_MUTE_ADDR;
        buf[1] = 0x00;
    }
 
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

static void AudioAmpDrv_SwReset(cAudioAmpDrv * me)
{
    ASSERT(me && me->isCreated);

    uint8 buf[2] = {RT9120S_SW_RESET_ADDR, 0x80}; //first byte: reg addr   second byte: data
    AudioAmpDrv_I2cWrite(me, 2, (const uint8*)buf);
}

void AudioAmpDrv_ErrorStatus(cAudioAmpDrv * me)
{
    uint8_t reg=RT9120S_AMP_ERRMASK_ADDR, data=0;
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


