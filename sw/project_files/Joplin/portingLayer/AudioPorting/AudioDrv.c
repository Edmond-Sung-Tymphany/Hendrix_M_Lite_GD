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
#ifdef HAS_POWER_CONTROL
#include "PowerDrv_v2.h"

#endif
#include "Adau1761_Drv.h"
#include "AmpDrvTas5760.h"

#define AUDIODRV_DEBUG_ENABLEx
#ifndef AUDIODRV_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif

/***************************************************************************
 * GPIO Operation
 ***************************************************************************/

#define DSP_PWR_ENABLE(x)         GpioDrv_SetBit(&(x),GPIO_OUT_DSP_3V3)
#define DSP_PWR_DISABLE(x)        GpioDrv_ClearBit(&(x),GPIO_OUT_DSP_3V3)

#define AMP_PWR_ENABLE(x)       GpioDrv_SetBit(&(x),GPIO_OUT_AMP_ON)
#define AMP_PWR_DISABLE(x)      GpioDrv_ClearBit(&(x),GPIO_OUT_AMP_ON)

#define AMP_PVDD_ENABLE(x)      GpioDrv_SetBit(&(x),GPIO_OUT_PVDD_EN)
#define AMP_PVDD_DISABLE(x)     GpioDrv_ClearBit(&(x),GPIO_OUT_PVDD_EN)

#define AMP_WF_MUTE(x)          GpioDrv_SetBit(&(x),GPIO_OUT_WF_MUTE)
#define AMP_WF_UNMUTE(x)        GpioDrv_ClearBit(&(x),GPIO_OUT_WF_MUTE)

#define TW_READ_FAULT(x)        GpioDrv_ReadBit(&(x),GPIO_IN_TW_FAULT)
#define WF_READ_FAULT(x)        GpioDrv_ReadBit(&(x),GPIO_IN_WF_FAULT)

/* the time (ms) per timeout signal */
#define AUDIO_SRV_TIMEOUT_IN_MS  10
#define AUDIO_DRV_INIT_DSP_POWER_DELAY      (30) //30ms
#define AUDIO_DRV_INIT_AMP_UNMUTE_DELAY     (100) //100ms
#define AUDIO_ACTIVE_MUSIC_DETECT_TIME      (4000)

#ifdef DSP_TUNING_ON_THE_FLY
static uint32_t adi_usbi_inserted=0;
#endif

static cI2CDrv      ampTas5760I2cObj;
static cAudioAmpDrv audioAmpTas5760Drv;
/*GPIO object*/
static cGpioDrv audioGpioDrv;

static int32 musicDetTimer = AUDIO_ACTIVE_MUSIC_DETECT_TIME;

/* Private functions / variables. Declare and drivers here */
/**
 * Function to monitor the amp status
 */
static void AudioDrv_AmpSafetyMonitor()
{
    uint8 tw_fault = (!TW_READ_FAULT(audioGpioDrv)); //low active
    uint8 wf_fault = (!WF_READ_FAULT(audioGpioDrv)); //low active
    bool isAmpFault = (tw_fault || wf_fault);
    if(isAmpFault != *(bool*)Setting_Get(SETID_IS_AMP_FAULT))
    {
        Setting_Set(SETID_IS_AMP_FAULT, &isAmpFault);
        if (tw_fault)
        {
            AudioAmpDrv_ErrorStatus(&audioAmpTas5760Drv);
        }
        if (wf_fault)
        {
            TP_PRINTF("Note: WF amp error !!! \r\n");
        }
    }
}

/**
 * Function to update the music stream status
 */
void AudioDrv_CheckMusicStreamStatus(void)
{
    if ((musicDetTimer > 0) && ((musicDetTimer -= AUDIO_SRV_TIMEOUT_IN_MS) <= 0))
    {
        bool isMusicOn = Adau1761Drv_MusicDetected();
        bool preStatus = *(bool*)Setting_Get(SETID_MUSIC_DET);
        if (isMusicOn != preStatus)
        {
            AudioMusicDetectStateEvt* pAudioMusicStateEvt;
            pAudioMusicStateEvt = Q_NEW(AudioMusicDetectStateEvt, AUDIO_MUSIC_STREAM_STATE_SIG);
            pAudioMusicStateEvt->hasMusicStream = isMusicOn;
            QF_PUBLISH(&pAudioMusicStateEvt->super, NULL);

            Setting_Set(SETID_MUSIC_DET, &isMusicOn);
            TP_PRINTF("Music status %d\r\n", isMusicOn);
        }
        musicDetTimer = AUDIO_ACTIVE_MUSIC_DETECT_TIME;
    }
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
    AMP_WF_MUTE(audioGpioDrv);

    me->initState = AUDIO_DRV_INIT_DSP_POWER;
#ifdef HAS_ADAU1761_DSP_VERSION
    AudioDrv_ReadDspVersion();
#endif
}

void AudioDrv_Xtor(cAudioDrv *me)
{
    AMP_WF_MUTE(audioGpioDrv);
    AudioAmpDrv_setSoftMute(&audioAmpTas5760Drv, TRUE);
#ifdef HAS_ADAU1761_DSP
    DSP_PWR_DISABLE(audioGpioDrv);
    Adau1761Drv_Xtor();
#endif
#ifdef HAS_TAS5760_AMP
    AudioAmpDrv_Xtor(&audioAmpTas5760Drv);
#endif

    AMP_PWR_DISABLE(audioGpioDrv);
    AMP_PVDD_DISABLE(audioGpioDrv);

    me->drvIsReady = FALSE;
}

BOOL AudioDrv_Init(cAudioDrv *me)
{
    if( ! me->drvIsReady )
    {
        switch(me->initState)
        {
            case AUDIO_DRV_INIT_DSP_POWER:
            {
                DSP_PWR_ENABLE(audioGpioDrv);
                me->nextDelayTime = AUDIO_DRV_INIT_DSP_POWER_DELAY;
                me->initState = AUDIO_DRV_INIT_DSP_CODE;
                break;
            }
            case AUDIO_DRV_INIT_DSP_CODE:
            {
                me->nextDelayTime = Adau1761Drv_Init();
                if(me->nextDelayTime == 0)
                {
                    Adau1761Drv_SysMute(TRUE);
                    me->initState = AUDIO_DRV_INIT_AMP_PVDD;
                }
                break;
            }
            case AUDIO_DRV_INIT_AMP_PVDD:
            {
                AMP_PVDD_ENABLE(audioGpioDrv);
                me->nextDelayTime = 0; //will just delay AUDIO_SRV_TIMEOUT_IN_MS by default.
                me->initState = AUDIO_DRV_INIT_TW_AMP;
                break;
            }
            case AUDIO_DRV_INIT_TW_AMP:
            {
                AMP_PWR_ENABLE(audioGpioDrv);
                ampTas5760I2cObj.pConfig = (tI2CDevice*)getDevicebyIdAndType(AMP_DEV_ID, I2C_DEV_TYPE, NULL);
                ASSERT(ampTas5760I2cObj.pConfig);
                AudioAmpDrv_Ctor(&audioAmpTas5760Drv, &ampTas5760I2cObj);
                AudioAmpDrv_setPbtlMode(&audioAmpTas5760Drv, FALSE);
                AudioAmpDrv_setPwmRate(&audioAmpTas5760Drv, ADC_PWM_RATE_16LRCK);
                AudioAmpDrv_setAnalogGain(&audioAmpTas5760Drv, ANALOG_GAIN_22_6_DBV);
                AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, FALSE);
                me->nextDelayTime = AUDIO_DRV_INIT_AMP_UNMUTE_DELAY;
                me->initState = AUDIO_DRV_INIT_END;
                break;
            }
            case AUDIO_DRV_INIT_END:
            {
                bool defaultVal = FALSE;
                Setting_Set(SETID_MUSIC_DET, &defaultVal);
                Setting_Set(SETID_IS_AMP_FAULT, &defaultVal);
                AudioDrv_SetChannel(AUDIO_CHANNEL_BT);
                me->drvIsReady = TRUE;
                break;
            }
            default:
                ASSERT(0);
                break;
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
    if (AUDIO_CHANNEL_AUXIN == audioChannel)
    {
        Adau1761Drv_AnalogInSource(DSP_ANALOG_LINE_IN);
    }
    else if(AUDIO_CHANNEL_BT == audioChannel)
    {
        Adau1761Drv_I2SInSource(DSP_I2S);
    }
    else if (AUDIO_CHANNEL_RCA == audioChannel)
    {
        Adau1761Drv_AnalogInSource(DSP_ANALOG_AUX_IN);
    }
    else if (AUDIO_CHANNEL_ANALOG_MIXED == audioChannel)
    {
        Adau1761Drv_AnalogInSourceMix();
    }
    else
    {
        ASSERT(0);  // source channel error
    }
}

/**
 * Function to update audio status including jackin status,  music steam status and so on.
 */
void AudioDrv_UpdateStatus(cAudioDrv *audioDrvObj)
{
    (void)(audioDrvObj);
    AudioDrv_AmpSafetyMonitor();
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
        AudioAmpDrv_I2cEnable(&audioAmpTas5760Drv, enabled);
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
            Setting_Set(SETID_VOLUME, &param);
            Adau1761Drv_SetVolume(param);
            break;
        case SYSTEM_GAIN_SETT_ID:
            break;
        case DSP_BASS_SETT_ID:
            Setting_Set(SETID_BASS, &param);
            Adau1761Drv_SetBass(param);
            break;
        case DSP_TREBLE_SETT_ID:
            Setting_Set(SETID_TREBLE, &param);
            Adau1761Drv_SetTreble(param);
            break;
        case DSP_PASSTHROUGH_SETT_ID:
            Adau1761Drv_BypassEnable(enabled);
            break;
        case AMP_SLEEP_MODE_ID:
            if (enabled)
            {
                AMP_PWR_DISABLE(audioGpioDrv);
                AMP_PVDD_DISABLE(audioGpioDrv);
            }
            else
            {
                AMP_PVDD_ENABLE(audioGpioDrv);
                AMP_PWR_ENABLE(audioGpioDrv);
            }
            break;
        case DSP_EQ_CTRL_TUNING:
            ASSERT(param < EQ_BAND_MAX);
            Setting_Set((SETID_EQ_BAND1 + param), &param2);
            Adau1761Drv_SetEqBands(param, param2);
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
            AudioAmpDrv_setSoftMute(&audioAmpTas5760Drv, muteEnable);
#endif
#ifdef HAS_TPA3116_AMP
            if( muteEnable )
            {
                AMP_WF_MUTE(audioGpioDrv);
            }
            else
            {
                AMP_WF_UNMUTE(audioGpioDrv);
            }
#endif
            break;
        case AUDIO_SOURCE_MUTE:
             Adau1761Drv_SysMute(muteEnable);
            break;
        case AUDIO_DSP_OUT_CH1_MUTE:
            Adau1761Drv_WFMute(muteEnable);

            break;
        case AUDIO_DSP_OUT_CH3_MUTE:
            Adau1761Drv_TW1Mute(muteEnable);
            break;
        case AUDIO_DSP_OUT_CH4_MUTE:
            Adau1761Drv_TW2Mute(muteEnable);
            break;

        default:
            //Support only soft mute
            ASSERT(0);
            break;
    }
}

#ifdef HAS_ADAU1761_DSP_VERSION

static void AudioDrv_ReadDspVersion()
{
    char msg[DSP_VERSION_LENGTH];
    sprintf(msg,"%.4g",Adau1761Drv_ReadVersion());
    Setting_Set(SETID_DSP_VER_STR, msg);
}
#endif

