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
#ifdef HAS_SYSTEM_CONTROL
#include "SystemDrv.h"
#endif
#ifdef HAS_ADAU1761_DSP
#include "Adau1761_Drv.h"
#endif

#ifdef HAS_AUDIO_IO_EXPANDER
#include "IoExpanderDrv.h"
#include "IoExpanderLedDrv.h"
#endif

#ifdef HAS_TAS5760_AMP
#include "AmpDrvTas5760.h"

static cI2CDrv      ampTas5760I2cObj;
static cAudioAmpDrv audioAmpTas5760Drv;
#endif


/*GPIO object*/
static cGpioDrv audioGpioDrv;

/***************************************************************************
 * GPIO Operation
 ***************************************************************************/

#define DSP_PWR_ENABLE(x)         GpioDrv_SetBit(&(x),GPIO_OUT_DSP_3V3)
#define DSP_PWR_DISABLE(x)        GpioDrv_ClearBit(&(x),GPIO_OUT_DSP_3V3)

#define AMP_PWR_ENABLE(x)       GpioDrv_SetBit(&(x),GPIO_OUT_AMP_ON)
#define AMP_PWR_DISABLE(x)      GpioDrv_ClearBit(&(x),GPIO_OUT_AMP_ON)

#define AMP_PVDD_ENABLE(x)      GpioDrv_SetBit(&(x),GPIO_OUT_PVDD_EN)
#define AMP_PVDD_DISABLE(x)     GpioDrv_ClearBit(&(x),GPIO_OUT_PVDD_EN)

#define AMP_TW_MUTE(x)          GpioDrv_SetBit(&(x),GPIO_OUT_TW_MUTE)
#define AMP_WF_MUTE(x)          GpioDrv_SetBit(&(x),GPIO_OUT_WF_MUTE)

#define AMP_TW_UNMUTE(x)        GpioDrv_SetBit(&(x),GPIO_OUT_TW_MUTE)
#define AMP_WF_UNMUTE(x)        GpioDrv_ClearBit(&(x),GPIO_OUT_WF_MUTE)

#define AUDIO_DRV_INIT_DSP_POWER_DELAY  (30) //30ms

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
// To Do: rework the juge condition  this detection may move to main App? only left the detect function here.
    if (!(Setting_IsReady(SETID_SYSTEM_SLEEP) && *(bool*)Setting_Get(SETID_SYSTEM_SLEEP)))
    {
        if( Adau1761Drv_MusicDetected() )
            auto_standby_cnt = 0;
        else
        {
            auto_standby_cnt ++;
//            if( auto_standby_cnt > 500 )    /* 5 seconds for debug */
            if( auto_standby_cnt > AUTO_STANDBY_TIMEOUT_CNT )
            {
            	// To Do: set system to auto standby 
                auto_standby_cnt = 0;
            }
        }
    }
    else
        auto_standby_cnt = 0;
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
    AMP_WF_MUTE(audioGpioDrv);
    me->initState = AUDIO_DRV_INIT_DSP_POWER;
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
                AudioAmpDrv_setSoftMute(&audioAmpTas5760Drv, TRUE);
                me->nextDelayTime = 0; //will just delay AUDIO_SRV_TIMEOUT_IN_MS by default.
                me->initState = AUDIO_DRV_INIT_END;
                break;
            }
            case AUDIO_DRV_INIT_END:
            {
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
            Adau1761Drv_SetVolume(param);
            break;
        case SYSTEM_GAIN_SETT_ID:
            break;
        case DSP_BASS_SETT_ID:
            Adau1761Drv_SetBass(param);
            break;
        case DSP_TREBLE_SETT_ID:
            Adau1761Drv_SetTreble(param);
            break;
        case DSP_PASSTHROUGH_SETT_ID:
            Adau1761Drv_BypassEnable(enabled);
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
        case AUDIO_AMP_MUTE:
            break;

        default:
            //Support only soft mute
            ASSERT(0);
            break;
    }
}


