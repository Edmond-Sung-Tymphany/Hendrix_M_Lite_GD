/*
-------------------------------------------------------------------------------
TYMPHANY LTD

                  Audio Driver
                  -------------------------
                  SW Module Document
 
@file        AudioDrv.c
@brief       This file implementes the middle layer of the audio service 
@author      Viking WANG
@date        2016-06-15
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "./AudioDrv_priv.h"
#include "SettingSrv.h"
#include "trace.h"
#include "AudioSrv.h"
#include "I2CDrv.h"
#include "GpioDrv.h"
#ifdef HAS_SYSTEM_CONTROL
#include "SystemDrv.h"
#endif
#ifdef HAS_ADAU1761_DSP
#include "Adau1761_Drv.h"
#endif
#ifdef HAS_SSM3582_AMP_HWI2C
#include "Ssm3582_HW_Drv.h"
#include "./Ssm3582_tbl.h"  // register value

static Ssm3582Drv_t ssm3582_full_range = 
{
    .p_regMap_init = ssm_reg_full_range_init,
    .p_regMap = ssm_reg_full_range,
    .deviceID = AMP_DEV_ID,
    .deviceType = I2C_DEV_TYPE,
};

static Ssm3582Drv_t AmpDrvObj;
#endif

#ifdef HAS_TAS5760_AMP
#include "AmpDrvTas5760.h"

static cI2CDrv      ampWooferI2cObj;
static cAudioAmpDrv audioAmpWooferDrv;
#endif

#define     IS_AUX_IN_PLUGGED(x)     (!(GpioDrv_ReadBit(&(x),GPIO_IN_AUDIO_JACK_DET1)))

/*GPIO object*/
static cGpioDrv audioGpioDrv;

#ifdef HAS_AUTO_STANDBY
static uint32_t auto_standby_cnt=0;
#endif
#ifdef DSP_TUNING_ON_THE_FLY
static uint32_t adi_usbi_inserted=0;
#endif

/* Private functions / variables. Declare and drivers here */
/**
 * Function to monitor the amp status
 */
static void AudioDrv_AmpSafetyMonitor()
{
}

void AudioDrv_CheckJackInStatus(void)
{

    uint32_t isAuxInPlugIn = IS_AUX_IN_PLUGGED(audioGpioDrv);
    /* test */
   
    Setting_Set(SETID_IS_AUXIN_PLUG_IN, &isAuxInPlugIn); 
}

void AudioDrv_ResetStandbyCounter(void)
{
    auto_standby_cnt = 0;
}

/**
 * Function to update the music stream status
 */
void AudioDrv_CheckMusicStreamStatus(void)
{
#ifdef HAS_AUTO_STANDBY
    if( SYSTEM_STATUS_WORKING == SystemDrv_GetSystemStatus() )
    {
        if( Adau1761Drv_MusicDetected() )
            auto_standby_cnt = 0;
        else
        {
            auto_standby_cnt ++;
//            if( auto_standby_cnt > 500 )    /* 5 seconds for debug */
            if( auto_standby_cnt > AUTO_STANDBY_TIMEOUT_CNT )
            {
                SystemDrv_SetSystemStatus(SYSTEM_STATUS_AUTO_STANDBY);
                auto_standby_cnt = 0;
            }
        }
    }
    else
        auto_standby_cnt = 0;
#endif
}

void AudioDrv_SetDspVol(uint32 vol)
{
#ifdef HAS_ADAU1761_DSP
    Adau1761Drv_SetVolume(vol);
#endif
}

static void AudioDrv_SetDspBass(uint32_t level)
{
#ifdef HAS_ADAU1761_DSP
    Adau1761Drv_SetBass(level);
#endif
}

static void AudioDrv_SetDspTreble(uint32_t level)
{
#ifdef HAS_ADAU1761_DSP
    Adau1761Drv_SetTreble(level);
#endif
}

static void AudioDrv_BypassEnable(bool enable)
{
#ifdef HAS_ADAU1761_DSP
    Adau1761Drv_BypassEnable(enable);
#endif
}

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void AudioDrv_Ctor(cAudioDrv *me)
{
    ASSERT(me);

    //Initialize Audio GPIO pins
    audioGpioDrv.gpioConfig= (tGPIODevice*)getDevicebyIdAndType(AUDIO_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(audioGpioDrv.gpioConfig);
    GpioDrv_Ctor(&audioGpioDrv, audioGpioDrv.gpioConfig);

#ifdef HAS_ADAU1761_DSP
    Adau1761Drv_Ctor();
#endif

#ifdef HAS_SSM3582_AMP_HWI2C
    AmpDrvObj = ssm3582_full_range;
    Ssm3582Drv_Ctor( &AmpDrvObj );
#endif
}

void AudioDrv_Xtor(cAudioDrv *me)
{
    /* Fill me in! */
#ifdef HAS_ADAU1761_DSP
    Adau1761Drv_Xtor();
#endif
#ifdef HAS_SSM3582_AMP_HWI2C
    Ssm3582Drv_Xtor( &AmpDrvObj );
#endif
#ifdef HAS_TAS5760_AMP
    AudioAmpDrv_Xtor(&audioAmpWooferDrv);
#endif

    me->drvIsReady = FALSE;
}

BOOL AudioDrv_Init(cAudioDrv *me)
{
    if( ! me->drvIsReady )
    {
#ifdef HAS_SYSTEM_CONTROL
        if( POWER_STAGE_POWER_OFF == SystemDrv_GetPowerStage() )
        {   // power is not ready
            me->nextDelayTime = 10;
            return FALSE;
        }
#endif
#ifdef HAS_ADAU1761_DSP
        me->nextDelayTime = Adau1761Drv_Init();
#else
        me->nextDelayTime = 0;
#endif
        if(me->nextDelayTime == 0)
        {
#ifdef HAS_SSM3582_AMP_HWI2C
            Ssm3582Drv_Init( &AmpDrvObj);
#endif
#ifdef HAS_TAS5760_AMP
            ampWooferI2cObj.pConfig = (tI2CDevice*)getDevicebyIdAndType(AMP_WOOFER_1_DEV_ID, I2C_DEV_TYPE, NULL);
            ASSERT(ampWooferI2cObj.pConfig);
            AudioAmpDrv_Ctor(&audioAmpWooferDrv, &ampWooferI2cObj);
            AudioAmpDrv_setPbtlMode(&audioAmpWooferDrv, FALSE);
            AudioAmpDrv_setPwmRate(&audioAmpWooferDrv, ADC_PWM_RATE_16LRCK);
            AudioAmpDrv_setAnalogGain(&audioAmpWooferDrv, ANALOG_GAIN_22_6_DBV);
#endif
            me->drvIsReady = TRUE;
#ifdef HAS_SYSTEM_CONTROL
            // tell everybody the audio driver is ready
            SystemDrv_NextPowerStage();
#endif
        }
    }
    
    return me->drvIsReady;
}

void AudioDrv_Reset(cAudioDrv *me)
{
    me->drvIsReady = FALSE;
}
/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/
/**
 * Function to change the channel. To call this function, normally you have to
 * call AudioDrv_Mute to mute the system in order to avoid pop noise, then set 
 * the channel and last,call AudioDrv_Mute to umute the system
 * @param[in]     theChannelToSet, the channel which will replace the current channel
 */
void AudioDrv_SetChannel(eAudioChannel audioChannel)
{
#ifdef HAS_ADAU1761_DSP
    if (AUDIO_CHANNEL_AUXIN == audioChannel)
    {
        Adau1761Drv_I2SInSource(DSP_I2S_ADC);
    }
    else if(AUDIO_CHANNEL_BT == audioChannel)
    {
        Adau1761Drv_I2SInSource(DSP_I2S_I2S);
    }
    else
    {
        ASSERT(0);  // source channel error
    }
#else
    if (AUDIO_CHANNEL_AUXIN == audioChannel)
    {
    }
    else if (AUDIO_CHANNEL_RCA == audioChannel)
    {
    }
    else if(AUDIO_CHANNEL_BT == audioChannel)
    {
    }
    else
    {
        ASSERT(0);  // source channel error
    }
#endif
}

/**
 * Function to update audio status including jackin status,  music steam status and so on.
 */
void AudioDrv_UpdateStatus(cAudioDrv *audioDrvObj)
{
    (void)(audioDrvObj);
    AudioDrv_AmpSafetyMonitor();
    AudioDrv_CheckJackInStatus();
    AudioDrv_CheckMusicStreamStatus();
#ifdef DSP_TUNING_ON_THE_FLY
    {
        bool enabled;
        if( GpioDrv_ReadBit(&audioGpioDrv, GPIO_IN_DSP_TUNING) )    // ADI USBi tool inserted
        {
            if( ! adi_usbi_inserted )
            {
                adi_usbi_inserted = 1;
            	I2C1_GPIO_Deinit();
            }
        	enabled = FALSE;
        }
        else
        {
            if( adi_usbi_inserted )
            {
                adi_usbi_inserted = 0;
            	I2C1_GPIO_ReInit();
            }
            enabled = TRUE;
        }
#ifdef HAS_ADAU1761_DSP
        Adau1761Drv_I2cEnable(enabled);
#endif
#ifdef HAS_TAS5760_AMP
        AudioAmpDrv_I2cEnable(&audioAmpWooferDrv, enabled);
#endif
#ifdef HAS_SSM3582_AMP_HWI2C
        Ssm3582Drv_I2cEnable(&AmpDrvObj, enabled);
#endif
    }
#endif
}

/**
 * Function to change audio settings, such as EQ.
 * @param[in]     pAudioEvt, a pointer of the audio event
 */
void AudioDrv_SetAudio(cAudioDrv *me, eAudioSettId audioSettId, BOOL enabled, uint32 param, uint32 param2)
{
    uEqDataPackage * deviceData = (uEqDataPackage*)&audioSettId;

    ASSERT(deviceData->eqDataPackageId < AUDIO_SETT_ID_MAX);

    switch (deviceData->eqDataPackageId)
    {
        case DSP_VOLUME_SETT_ID:
             AudioDrv_SetDspVol(param);
             break;
        case SYSTEM_GAIN_SETT_ID:
            break;
        case DSP_BASS_SETT_ID:
            AudioDrv_SetDspBass(param);
            break;
        case DSP_TREBLE_SETT_ID:
            AudioDrv_SetDspTreble(param);
            break;
        case DSP_PASSTHROUGH_SETT_ID:
            AudioDrv_BypassEnable(enabled);
            break;
        default:
            ASSERT(0);  // error setting id
            break;
    }
}

/**
 * Function to mute the system.
 * @param[in]   muteType    This parameter defines the way of how to mute the system,
 * you could mute the input source, or mute by dsp or mute the output
 * @param[in]   muteEnable  mute or unmute
 */
void AudioDrv_Mute(eAudioMuteType muteType, bool muteEnable)
{
    ASSERT(muteType < AUDIO_MUTE_TYPE_MAX);
    switch (muteType)
    {
        case AUDIO_AMP_SOFT_MUTE:
#ifdef HAS_TAS5760_AMP
            AudioAmpDrv_setSoftMute(&audioAmpWooferDrv, muteEnable);
#endif
#ifdef HAS_TPA3118_AMP
            if( muteEnable )
            {
                AMP_MUTE_ENABLE(audioGpioDrv);
            }
            else
            {
                AMP_MUTE_DISABLE(audioGpioDrv);
            }
#endif
#ifdef HAS_SSM3582_AMP_HWI2C
            Ssm3582Drv_MuteControl(&AmpDrvObj, muteEnable);
#endif
            break;
        case AUDIO_AMP_MUTE:
#ifdef HAS_SSM3582_AMP_HWI2C
            Ssm3582Drv_PowerControl(&AmpDrvObj, !muteEnable);     /* shutdown */
#endif
            break;

        default:
            //Support only soft mute
            ASSERT(0);
            break;
    }
}

#ifdef HAS_SSM3582_AMP_OUTPUT
/**
 * Function to set Amp output
 * @param[in]   level
 * @param[in]   
 */
void AudioDrv_SetAmpOutput(uint8 level)
{
    Ssm3582Drv_OutputPower(&AmpDrvObj, level);
}

/**
 * Function to get Amp temperature
 * @param[in]   level
 * @param[in]   
 */
uint16 AudioDrv_GetAmpTemperature(void)
{
    Ssm3582Drv_RegGetTemperature(&AmpDrvObj);
}
#endif


