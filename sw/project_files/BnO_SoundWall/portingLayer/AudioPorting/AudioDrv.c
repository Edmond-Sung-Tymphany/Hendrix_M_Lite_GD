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

#include "product.config"
#include "stm32f0xx.h"
#include "SettingSrv.h"
#include "trace.h"
#include "bsp.h"
#include "AudioSrv.h"
#include "I2CDrv.h"
#include "GpioDrv.h"
#ifdef HAS_SYSTEM_CONTROL
#include "SystemDrv.h"
#endif
#include "deviceTypes_v2.h"
#ifdef HAS_SOUNDMODE_UPDATE
#include "core.pb.h"
#endif
#include "Adau1452_Drv.h"
#include "Codec_cs42528.h"
#ifdef HAS_IOE_LED
#include "IoeLedDrv.h"
#endif
#include "./AudioDrv_priv.h"

/*GPIO object*/
static cGpioDrv audioGpioDrv;

/* for 100ms audio tast */
static uint8_t  audio_server_cnt=0;
/* for 1000ms audio tast */
static uint8_t  audio_server_1s_cnt=0;
#ifdef DSP_TUNING_ON_THE_FLY
static uint8_t adi_usbi_inserted=0;
#endif

#ifdef MUSIC_DETECT_BY_GPIO
static BOOL has_spdif_signal=FALSE;
static uint16_t spdif_detect_cnt=0;
static uint16_t spdif_pulse_cnt=0;
SourceAudioInfo_t opt_audio_info =
{
    .audio_input_on = 0,
    .audio_linked_cnt = 0,
    .audio_lost_cnt = 0,
};
SourceAudioInfo_t aux_audio_info =
{
    .audio_input_on = 0,
    .audio_linked_cnt = 0,
    .audio_lost_cnt = 0,
};

#define SPDIF_DETECT_MAX_CNT        50
#define SPDIF_PULSE_THRESHOLD_CNT   9
// please refer to attached device.c: PC2 used as spdif detect
#define SPDIF_SIGNAL_CURRENT_LEVEL     (GPIOC->IDR & GPIO_Pin_2)
//#define SPDIF_SIGNAL_CURRENT_LEVEL     GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2)

static void AudioDrv_CheckSpdifSignal(void)
{
    spdif_detect_cnt ++;
    if( spdif_detect_cnt > SPDIF_DETECT_MAX_CNT )
    {
        if( spdif_pulse_cnt > SPDIF_PULSE_THRESHOLD_CNT )
            has_spdif_signal = TRUE;
        else
            has_spdif_signal = FALSE;
//        printf("\n%d", spdif_pulse_cnt);
        // reset the cnt
        spdif_detect_cnt = 0;
        spdif_pulse_cnt = 0;
    }
    else
    {
        uint16_t spdif_level1, spdif_level2;
        spdif_level1 = SPDIF_SIGNAL_CURRENT_LEVEL;
        spdif_level2 = SPDIF_SIGNAL_CURRENT_LEVEL;
        spdif_pulse_cnt += (spdif_level1 != spdif_level2);
    }    
}
#endif

#ifdef HAS_AUTO_STANDBY
#define AUTO_STANDBY_TIMEOUT_CNT    (15*60*10)  // 15minute * 60second / 0.1 second(100ms)
static uint32_t auto_standby_cnt=0;

static void AudioDrv_CheckAutoStandby()
{
    uint32_t is_music_detected;
#ifdef MUSIC_DETECT_BY_GPIO
    uint32_t audio_source;
    audio_source = *(uint32_t *)Setting_Get(SETID_AUDIO_SOURCE);
    if( audio_source == AUDIO_CHANNEL_AUXIN )
    {
        is_music_detected = aux_audio_info.audio_input_on;
    }
    else if( audio_source == AUDIO_CHANNEL_OPT )
    {
        is_music_detected = opt_audio_info.audio_input_on;
    }
    else if( audio_source == AUDIO_CHANNEL_I2S_2 )
    {   // A2B 
        is_music_detected = 1;
    }
#else
    // currently, check the codec input channel only, fix me later.
    if( Adau1452Drv_CodecSignalDetect() )
        is_music_detected = 0;
    else
        is_music_detected = 1;
#endif

    if( is_music_detected )
    {
        auto_standby_cnt = 0;
    }
    else
    {
        auto_standby_cnt ++;
//        if( auto_standby_cnt > 200 )    /* 20 seconds for debug */
//          if( auto_standby_cnt > 1200 )    /* 2 minutes for SQA verify */
        if( auto_standby_cnt > AUTO_STANDBY_TIMEOUT_CNT ) 
        {
            SystemDrv_SetSystemStatus(SYSTEM_STATUS_AUTO_STANDBY);
            auto_standby_cnt = 0;
        }
    }
}
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
#ifdef DSP_TUNING_ON_THE_FLY
/* ignore auto switch when dsp tuning */
    if( adi_usbi_inserted )
        return ;
#endif
#ifdef MUSIC_DETECT_BY_GPIO
{
    uint32_t pl_sensed, music_detected;
    // POWER LINK music check, 1:no music, 0:music detected.
    if( GpioDrv_ReadBit(&audioGpioDrv, GPIO_IN_DET_AUX) )    
        music_detected = 0;
    else
        music_detected = 1;
    if( GpioDrv_ReadBit(&audioGpioDrv, GPIO_IN_PL_INSERT) )    
        pl_sensed = 1;
    else
        pl_sensed = 0;
    if( pl_sensed || music_detected )    
    {   
        aux_audio_info.audio_input_on = 1;
        aux_audio_info.audio_linked_cnt ++;
        aux_audio_info.audio_lost_cnt = 0;
    }
    else
    {
        aux_audio_info.audio_input_on = 0;
        aux_audio_info.audio_linked_cnt = 0;
        aux_audio_info.audio_lost_cnt ++;
    }

    if( has_spdif_signal )    
    {
        opt_audio_info.audio_input_on = 1;
        opt_audio_info.audio_linked_cnt ++;
        opt_audio_info.audio_lost_cnt = 0;
    }
    else
    {
        opt_audio_info.audio_input_on = 0;
        opt_audio_info.audio_linked_cnt = 0;
        opt_audio_info.audio_lost_cnt ++;
    }
//    printf("\n\rAUX:%d, SPDIF:%d.", aux_audio_info.audio_input_on, opt_audio_info.audio_input_on);
}
#endif  // MUSIC_DETECT_BY_GPIO
}

void AudioDrv_SetDspVol(uint32 vol)
{
    Adau1452Drv_SetVolume(vol);
}

static void AudioDrv_BypassEnable(bool enable)
{
    Adau1452Drv_BypassEnable(enable);
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

    // dsp setup
    Adau1452Drv_Ctor();
    // codec setup
    CS42528Drv_Ctor();

    audio_server_cnt = 0;
    audio_server_1s_cnt = 0;
    auto_standby_cnt = 0;
}

void AudioDrv_Xtor(cAudioDrv *me)
{
    /* Fill me in! */
    Adau1452Drv_Xtor();
    CS42528Drv_Xtor();

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
        me->nextDelayTime = Adau1452Drv_Init(); // fix me later
        if(me->nextDelayTime == 0)
        {
            uint32_t dsp_ver;
            dsp_ver = Adau1452Drv_DspVersion();
            // initial the codec here
            CS42528Drv_Init();  // mute codec first
#ifdef HAS_SYSTEM_CONTROL
            SystemDrv_SetDspVersion(dsp_ver);
            // tell everybody the audio driver is ready
            SystemDrv_NextPowerStage();
#endif
            me->drvIsReady = TRUE;
        }
    }
    
    audio_server_cnt = 0;
    audio_server_1s_cnt = 0;

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
    if (AUDIO_CHANNEL_AUXIN == audioChannel)
    {
        CS42528Drv_SAIOutputSelect(SAI_SOURCE_ADC);
        Adau1452Drv_AsrcInSource(DSP_Asrc_Codec);
    }
    else if (AUDIO_CHANNEL_OPT == audioChannel)
    {
#ifdef SPDIF_DIRECT_TO_DSP
        CS42528Drv_SAIOutputSelect(SAI_SOURCE_RX2);
        Adau1452Drv_AsrcInSource(DSP_Asrc_Spdif);
//        Adau1452Drv_AsrcInSource(DSP_Asrc_Codec);
/*
        Adau1452Drv_AsrcInSource(DSP_Asrc_Spdif);
        CS42528Drv_SAIOutputSelect(SAI_SOURCE_ADC);
*/
#else
        CS42528Drv_SAIOutputSelect(SAI_SOURCE_RX0);
        Adau1452Drv_AsrcInSource(DSP_Asrc_Codec);
#endif
    }
    else if(AUDIO_CHANNEL_I2S_2 == audioChannel)
    {
        CS42528Drv_SAIOutputSelect(SAI_SOURCE_A2B);
        Adau1452Drv_AsrcInSource(DSP_Asrc_A2B);
    }
    else
    {
        ASSERT(0);  // source channel error
    }

    Setting_Set(SETID_AUDIO_SOURCE, (void *)&audioChannel);
}

/**
 * Function to update audio status including jackin status,  music steam status and so on.
 */
void AudioDrv_UpdateStatus(cAudioDrv *audioDrvObj)
{
    (void)(audioDrvObj);

#ifdef MUSIC_DETECT_BY_GPIO
    AudioDrv_CheckSpdifSignal();
#endif

    audio_server_cnt ++;
    if( audio_server_cnt < 10 )
        return ;
    
    audio_server_cnt = 0;   // reset count

    // the following status updated each 100ms
    AudioDrv_CheckMusicStreamStatus();

// the following status only updated on "WORKING" status
#ifdef HAS_SYSTEM_CONTROL
    if( SYSTEM_STATUS_WORKING != SystemDrv_GetSystemStatus() )
        return ;
#endif

    AudioDrv_AmpSafetyMonitor();
    AudioDrv_CheckJackInStatus();
    AudioDrv_CheckAutoStandby();

#ifdef DSP_TUNING_ON_THE_FLY
    {
        bool enabled;
        if( GpioDrv_ReadBit(&audioGpioDrv, GPIO_IN_DSP_TUNING) )    // ADI USBi tool inserted
        {
            if( ! adi_usbi_inserted )
            {
                adi_usbi_inserted = 1;
                I2C2_GPIO_Deinit();
            }
            enabled = FALSE;
        }
        else
        {
            if( adi_usbi_inserted )
            {
                adi_usbi_inserted = 0;
                I2C2_GPIO_ReInit();
            }
            enabled = TRUE;
        }
        CS42528Drv_I2cEnable(enabled);
        Adau1452Drv_I2cEnable(enabled);
#if defined(HAS_IOE_LED) && defined(IOE_LED_VIA_HW_I2C)
        IoeLed_I2cEnable(enabled);
#endif
    }
#endif

    // 1000ms task start here
    audio_server_1s_cnt ++;
    if( audio_server_1s_cnt < 10 )
        return ;
    audio_server_1s_cnt = 0;
#ifdef HAS_TEMPERATURE_MONITOR
    Adau1452_UpdateNtcValue();
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
        case DSP_PASSTHROUGH_SETT_ID:
            AudioDrv_BypassEnable(enabled);
            break;
        case AUDIO_CHANNEL_MUTE_ID:
            Adau1452Drv_ChannelMute(param);
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
            break;
        case AUDIO_DSP_DACOUT_MUTE:
            // in fact we use codec 42528 mute
            CS42528Drv_DacMute(muteEnable);
            break;
        case AUDIO_AMP_MUTE:
            // amp shutdown/reset control
            AudioDrv_ShutDown(muteEnable);
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

#ifdef HAS_AUDIO_MUTE_CHANNEL 
// for soundwall test tone control
// [NOTE] mute all the channel except the channel_mask
// channel_mask = 0 : umute all channel for normal operation
// channel_mask = 1 : test tone on channel 1
// channel_mask = 2 : test tone on channel 2
// channel_mask = 3 : test tone on channel 3
// channel_mask = 4 : test tone on channel 4
void AudioDrv_MuteChannel(uint32 channel_mask)
{
    if( channel_mask > 4 )
    {   // exceed the channel number
        ALWAYS_printf("\n\rUnknown mute channel.");
    }
    else
    {
        if( channel_mask == 0 )
        {
            Adau1452Drv_ChannelMute(0xf0);
            Adau1452Drv_EnableTestTone(FALSE);
        }
        else
        {
            Adau1452Drv_EnableTestTone(TRUE);
            // mute all channel first
            Adau1452Drv_ChannelMute(0xf1);
            // unmute the specify channel
            if( 1 == channel_mask )
            {
                Adau1452Drv_ChannelMute(0x00);
                Adau1452Drv_ChannelMute(0x10);
            }
            else if( 2 == channel_mask )
            {
                Adau1452Drv_ChannelMute(0x20);
                Adau1452Drv_ChannelMute(0x30);
            }
            else if( 3 == channel_mask )
            {
                Adau1452Drv_ChannelMute(0x40);
                Adau1452Drv_ChannelMute(0x50);
            }
            else if( 4 == channel_mask )
            {
                Adau1452Drv_ChannelMute(0x60);
                Adau1452Drv_ChannelMute(0x70);
            }
            else
                ;
        }
    }
}
#endif

