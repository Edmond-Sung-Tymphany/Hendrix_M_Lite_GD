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
#include "bsp.h"
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
#include "ssm3582_drv.config"

static Ssm3582Drv_t ssm3582_full_range = 
{
    .p_regMap = ssm_reg_full_range,
    .deviceID = AMP_DEV_ID,
    .deviceType = I2C_DEV_TYPE,
};
#endif

#ifdef HAS_TAS5760_AMP
#include "AmpDrvTas5760.h"

static cI2CDrv      ampWooferI2cObj;
static cAudioAmpDrv audioAmpWooferDrv;
#endif

/*GPIO object*/
static cGpioDrv audioGpioDrv;

/* for 100ms audio tast */
static uint8_t  audio_server_cnt=0;
#ifdef HAS_SHAPE_EQ_SWITCH
static uint8_t is_shape_switch_on = 0;
static uint32_t is_shape_eq_on = 0;
#endif
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
//            if( auto_standby_cnt > 200 )    /* 20 seconds for debug */
//            if( auto_standby_cnt > 1200 )    /* 2 minutes for SQA verify */
            if( auto_standby_cnt > AUTO_STANDBY_TIMEOUT_CNT ) /* 20 minutes for release */
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
    Ssm3582Drv_Ctor( &ssm3582_full_range );
#endif

    audio_server_cnt = 0;
#ifdef HAS_SHAPE_EQ_SWITCH
    is_shape_switch_on = 0;
    is_shape_eq_on = 0;
    Setting_Set(SETID_SHAPE_EQ, &is_shape_eq_on);
#endif
}

void AudioDrv_Xtor(cAudioDrv *me)
{
    /* Fill me in! */
#ifdef HAS_ADAU1761_DSP
    Adau1761Drv_Xtor();
#endif
#ifdef HAS_SSM3582_AMP_HWI2C
    Ssm3582Drv_Xtor( &ssm3582_full_range );
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
            Ssm3582Drv_Init( &ssm3582_full_range );
#endif
#ifdef HAS_TAS5760_AMP
            ampWooferI2cObj.pConfig = (tI2CDevice*)getDevicebyIdAndType(AMP_WOOFER_1_DEV_ID, I2C_DEV_TYPE, NULL);
            ASSERT(ampWooferI2cObj.pConfig);
            AudioAmpDrv_Ctor(&audioAmpWooferDrv, &ampWooferI2cObj);
            AudioAmpDrv_setPbtlMode(&audioAmpWooferDrv, FALSE);
//            AudioAmpDrv_setPwmRate(&audioAmpWooferDrv, ADC_PWM_RATE_16LRCK);
            AudioAmpDrv_setPwmRate(&audioAmpWooferDrv, ADC_PWM_RATE_8LRCK);
#ifdef WOOFER_TWEETER_SWAP_ENABLE
            AudioAmpDrv_setAnalogGain(&audioAmpWooferDrv, ANALOG_GAIN_19_2_DBV);    // ANALOG_GAIN_19_2_DBV, ANALOG_GAIN_22_6_DBV,
#else
            AudioAmpDrv_setAnalogGain(&audioAmpWooferDrv, ANALOG_GAIN_22_6_DBV);    // ANALOG_GAIN_19_2_DBV, ANALOG_GAIN_22_6_DBV,
#endif
#endif
            AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, TRUE);
            AudioDrv_ShutDown(FALSE);
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
        Adau1761Drv_AnalogInSource(DSP_ANALOG_AUX_IN);
    }
    else if (AUDIO_CHANNEL_RCA == audioChannel)
    {
        Adau1761Drv_I2SInSource(DSP_I2S_ADC);
        Adau1761Drv_AnalogInSource(DSP_ANALOG_LINE_IN);
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

#ifdef HAS_SYSTEM_CONTROL
    // only work on "WORKING" status
    if( SYSTEM_STATUS_WORKING != SystemDrv_GetSystemStatus() )
        return ;
#endif

    audio_server_cnt ++;
    if( audio_server_cnt < 10 )
        return ;
    
    audio_server_cnt = 0;   // reset count
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
        Ssm3582Drv_I2cEnable(&ssm3582_full_range, enabled);
#endif
    }
#endif

#ifdef HAS_SHAPE_EQ_SWITCH
    if( GpioDrv_ReadBit(&audioGpioDrv, GPIO_IN_SHAPE_EQ) )
        is_shape_switch_on = 1;
    else
        is_shape_switch_on = 0;
    if( is_shape_switch_on != is_shape_eq_on )
    {
        is_shape_eq_on = is_shape_switch_on;
        Adau1761Drv_EnableShapeEQ(is_shape_eq_on);
        Setting_Set(SETID_SHAPE_EQ, &is_shape_eq_on);
		BRINGUP_printf("\n\r shape on : %d \n\r", is_shape_eq_on);
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
#ifdef HAS_AUTO_STANDBY
        case AUDIO_RESET_LINEIN_JACK_AND_MUSIC_STA_SETT_ID:
            auto_standby_cnt = 0;   // reset the count;
            break;
#endif
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

#ifdef HAS_I2C_BUS_DETECT
    if( SystemDrv_GetI2cBusStatus() != I2C_ERROR_NONE )
    {
        // i2c bus error, mute the amplifier always to protect the speaker.
        muteEnable = TRUE;
    }
#endif

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
        case AUDIO_AMP_MUTE:
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
            break;
        default:
            //Support only soft mute
            ASSERT(0);
            break;
    }
}

void AudioDrv_ShutDown(bool enable)
{
#ifdef HAS_SYSTEM_CONTROL
    SystemDrv_ShutDownAmp(enable);
#endif
}


