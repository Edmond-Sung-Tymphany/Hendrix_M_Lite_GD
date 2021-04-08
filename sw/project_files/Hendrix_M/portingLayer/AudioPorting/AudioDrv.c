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

#ifdef HAS_ADAU1761_DSP
#include "Adau1761_Drv.h"
#endif

#ifdef HAS_AUDIO_IO_EXPANDER
#include "IoExpanderDrv.h"
#include "IoExpanderLedDrv.h"
#include "VIoExpanderGpioDrv.h"
#endif


#ifdef HAS_TAS5760_AMP
#include "AmpDrvTas5760.h"

static cI2CDrv      ampTas5760I2cObj;
static cAudioAmpDrv audioAmpTas5760Drv;
#endif

//#define     IS_AUX_IN_PLUGGED(x)     ((!(GpioDrv_ReadBit(&(x),GPIO_IN_AUDIO_JACK_DET1)))  // need to rework, Aux_IN detected by DSP

/* the time (ms) per timeout signal */
#define AUDIO_SRV_TIMEOUT_IN_MS  10
#define AUDIO_DRV_INIT_DSP_POWER_DELAY      (30) //30ms
#define AUDIO_DRV_INIT_AMP_UNMUTE_DELAY     (100) //100ms



/*GPIO object*/
static cGpioDrv audioGpioDrv;

#ifdef HAS_AUDIO_IO_EXPANDERx
static cVIoExpanderGpioDrv AudioIoeDrv;
#define TW_MUTE    (VIoExpanderGpioDrv_SetGpio(&AudioIoeDrv, IOE_OUT_TW_MUTE))    //IOE_OUT_TW_MUTE
#define WF_MUTE    (VIoExpanderGpioDrv_SetGpio(&AudioIoeDrv, IOE_OUT_WF_MUTE))    //IOE_OUT_WF_MUTE

#define TW_UNMUTE  (VIoExpanderGpioDrv_ClearGpio(&AudioIoeDrv, IOE_OUT_TW_MUTE))
#define WF_UNMUTE  (VIoExpanderGpioDrv_ClearGpio(&AudioIoeDrv, IOE_OUT_WF_MUTE))

#define TW_READ_FAULT   (VIoExpanderGpioDrv_ReadGpio(&AudioIoeDrv, IOE_IN_TW_FAULT))
#define WF_READ_FAULT   (VIoExpanderGpioDrv_ReadGpio(&AudioIoeDrv, IOE_IN_WF_FAULTZ))
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
    uint8 tw_fault = TW_READ_FAULT();
    uint8 wf_fault = WF_READ_FAULT();
    bool isAmpFault;
    (tw_fault && wf_fault)?(isAmpFault = FALSE):(isAmpFault= TRUE);
    if(isAmpFault != (* (bool *)Setting_Get(SETID_IS_AMP_FAULT)))
    {
        Setting_Set(SETID_IS_AMP_FAULT, &isAmpFault);
        TYMQP_LOG(NULL,"AMP FAULT :%d",isAmpFault);
    }

}


void AudioDrv_CheckJackInStatus(void)
{

}

/**
 * Function to update the music stream status
 */
void AudioDrv_CheckMusicStreamStatus(void)
{
#define AUDIO_DETECTION_TIMEOUT_IN_MS  600

    static uint32 audio_detection_timeout;
    uint8 status = 0;

    bool isauxStreaming =  Adau1761Drv_CH1_MusicDetected();
    if(isauxStreaming)
        status |= (1<<AUXIN_JACK);

#ifdef DSP_BT_CHANNEL_DETECTION
    bool isbtStreaming =  Adau1761Drv_CH2_MusicDetected();
    if(isbtStreaming)
        status |= (1<<BLUETOOTH_JACK);
#endif
    uint8 prev_status;
    if(Setting_IsReady(SETID_MUSIC_STATUS))
        prev_status = *(uint8*)Setting_Get(SETID_MUSIC_STATUS);
    else
        prev_status = INVALID_VALUE;
    if(status != prev_status)
    {
        if(0 == audio_detection_timeout)
        {
            audio_detection_timeout = getSysTime();
        }
        if ((getSysTime() - audio_detection_timeout) >= AUDIO_DETECTION_TIMEOUT_IN_MS)
        {

            AudioMusicDetectStateEvt* pAudioMusicStateEvt = Q_NEW(AudioMusicDetectStateEvt, AUDIO_MUSIC_STREAM_STATE_SIG);
            pAudioMusicStateEvt->hasMusicStream = (status!=0)?TRUE:FALSE;
            pAudioMusicStateEvt->jackId = status;
            QF_PUBLISH(&pAudioMusicStateEvt->super, NULL);

            Setting_Set(SETID_MUSIC_STATUS, &(status));

            audio_detection_timeout = 0;
        }
    }
    else
    {
        audio_detection_timeout = 0;
    }
}

void AudioDrv_SetDspVol(uint32 vol)
{
#ifdef HAS_ADAU1761_DSP
    Adau1761Drv_SetVolume(vol);
#endif
}

#ifdef HAS_SYSTEM_GAIN_CONTROL
static void AudioDrv_SetSystemGain(uint32 gain)
{
#ifdef HAS_ADAU1761_DSP
    Adau1761Drv_SetSystemGain(gain);
#endif
}
#endif

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
    me->initState = AUDIO_DRV_INIT_DSP_CODE;


#ifdef HAS_AUDIO_IO_EXPANDERx
    tVIoeGpioDevice *pAudioIoeGpioDevice = (tVIoeGpioDevice*)getDevicebyIdAndType(AUDIO_DEV_ID, IO_EXPANDER_DEV_TYPE, NULL);
    VIoExpanderGpioDrv_Ctor(&AudioIoeDrv, pAudioIoeGpioDevice);
#endif

#ifdef HAS_ADAU1761_DSP_VERSION
    AudioDrv_ReadDspVersion();
#endif
}

void AudioDrv_Xtor(cAudioDrv *me)
{
    /* Fill me in! */
    AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, TRUE);
#ifdef HAS_ADAU1761_DSP
    Adau1761Drv_Xtor();
#endif
#ifdef HAS_TAS5760_AMP
    AudioAmpDrv_Xtor(&audioAmpTas5760Drv);
#endif

#ifdef HAS_AUDIO_IO_EXPANDERx
    VIoExpanderGpioDrv_Xtor(&AudioIoeDrv);
#endif
    me->drvIsReady = FALSE;
}

BOOL AudioDrv_Init(cAudioDrv *me)
{
    if( ! me->drvIsReady )
    {
        switch(me->initState)
        {
            case AUDIO_DRV_INIT_DSP_CODE:
            {
                me->nextDelayTime = Adau1761Drv_Init();
                if(me->nextDelayTime == 0)
                {
                    me->initState = AUDIO_DRV_INIT_TW_AMP;
                }
                break;
            }
            case AUDIO_DRV_INIT_TW_AMP:
            {
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
                Setting_Set(SETID_IS_AMP_FAULT, &defaultVal);
                Setting_Set(SETID_MUSIC_STATUS, &defaultVal);
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
#ifdef HAS_ADAU1761_DSP
    if (AUDIO_CHANNEL_AUXIN == audioChannel)
    {
        Adau1761Drv_AnalogInSource(DSP_I2S_ADC);
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
#define AUDIO_DRV_UPDATE_TICK_COUNT 20

    static uint8 audio_drv_tick_count;

    if(audio_drv_tick_count > AUDIO_DRV_UPDATE_TICK_COUNT)
    {
        audio_drv_tick_count = 0;

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
    else
    {
        audio_drv_tick_count++;

    }
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
#ifdef HAS_SYSTEM_GAIN_CONTROL
            AudioDrv_SetSystemGain(param);
#endif
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
            TYMQP_LOG(NULL,"AUDIO_AMP_SOFT_MUTE %d",muteEnable);
#ifdef HAS_TAS5760_AMP
            AudioAmpDrv_setSoftMute(&audioAmpTas5760Drv, muteEnable);
#endif
#ifdef HAS_TPA3116_AMP
            if( muteEnable )
            {
                PowerDrv_WFMute();
            }
            else
            {
                PowerDrv_WFUnMute();
            }
#endif
            break;
        case AUDIO_AMP_MUTE:
#ifdef HAS_AUDIO_IO_EXPANDER
            if(muteEnable)
            {
#ifdef HAS_TAS5760_AMP
                PowerDrv_TWMute();
#endif
#ifdef HAS_TPA3116_AMP
                PowerDrv_WFMute();
#endif
            }
            else
            {
#ifdef HAS_TAS5760_AMP
                PowerDrv_TWUnMute();
#endif
#ifdef HAS_TPA3116_AMP
                PowerDrv_WFUnMute();
#endif

            }
#endif
            break;
        case AUDIO_AMP_SOFT_MUTE_TW1:
#ifdef HAS_TAS5760_AMP
            AudioAmpDrv_setSoftMuteLeftChannel(&audioAmpTas5760Drv, muteEnable);
#endif
            break;
        case AUDIO_AMP_SOFT_MUTE_TW2:
#ifdef HAS_TAS5760_AMP
            AudioAmpDrv_setSoftMuteRightChannel(&audioAmpTas5760Drv, muteEnable);
#endif
            break;
        case AUDIO_AMP_SOFT_MUTE_WF:
            if(muteEnable)
            {
#ifdef HAS_TPA3116_AMP
                PowerDrv_WFMute();
#endif
            }
            else
            {
#ifdef HAS_TPA3116_AMP
                PowerDrv_WFUnMute();
#endif
            }
            break;
        case AUDIO_SOURCE_MUTE:
            Adau1761Drv_MainMute(muteEnable);
            break;
        case AUDIO_DSP_OUT_CH1_MUTE:
            Adau1761Drv_CH_Mute(muteEnable,AUDIO_DSP_OUT_CH1_MUTE-AUDIO_DSP_OUT_CH_MIN);
#ifdef HAS_TWO_CH_WF
            Adau1761Drv_CH_Mute(muteEnable,AUDIO_DSP_OUT_CH2_MUTE-AUDIO_DSP_OUT_CH_MIN);
#endif
            break;
        case AUDIO_DSP_OUT_CH2_MUTE:
#ifndef HAS_TWO_CH_WF
            Adau1761Drv_CH_Mute(muteEnable,AUDIO_DSP_OUT_CH2_MUTE-AUDIO_DSP_OUT_CH_MIN);
#endif
            break;
        case AUDIO_DSP_OUT_CH3_MUTE:
            Adau1761Drv_CH_Mute(muteEnable,AUDIO_DSP_OUT_CH3_MUTE-AUDIO_DSP_OUT_CH_MIN);
            break;
        case AUDIO_DSP_OUT_CH4_MUTE:
            Adau1761Drv_CH_Mute(muteEnable,AUDIO_DSP_OUT_CH4_MUTE-AUDIO_DSP_OUT_CH_MIN);
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
    Setting_Set(SETID_DSP_VER, msg);
}
#endif
