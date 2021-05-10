/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Audio Driver
                  -------------------------

                  SW Module Document
 
@file        AudioDrv.c
@brief       This file implementes the middle layer of the audio service 
@author      Bob.Xu 
@date        2015-06-15
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2015-06-15     Bob.Xu 
DESCRIPTION: First Draft. Generated by newclass.py
SCO/ERROR  : 
-------------------------------------------------------------------------------
*/

#include "./AudioDrv_priv.h"
#include "SettingSrv.h"
#include "trace.h"
#include "AudioSrv.h"
#include "I2CDrv.h"
#include "DspDrv1761.h"
#include "AdcDrv.h"
#include "gpioDrv.h"
#include "commonTypes.h"
#include "DspDrv.Conf"

static cI2CDrv      i2cObj;
static cDSPDrv1761  dspDrv;
/*Amplify adc object and config */
static cADCDrv adcDrvForAmp;
static cGpioDrv gpioAmpMuteDrv;
static BOOL ampMuted = FALSE;
static uint16 musicCheckCnt = 0;
static uint16 hasMusicCnt = 0;

static tTempLimiterCtrl tempLimiter[]=
{
    {1118,  4096,  LIMITER_LEVEL_MINUS_20_DB},//0.9v~3.3v   
    {1093,  1115,  LIMITER_LEVEL_MINUS_22_DB},//0.88v~0.9v
    {1068,  1090,  LIMITER_LEVEL_MINUS_24_DB},//0.86v~0.88v
    {1044,  1065,  LIMITER_LEVEL_MINUS_26_DB},//0.84v~0.86v
    {1018,  1040,  LIMITER_LEVEL_MINUS_28_DB},//0.82v~0.84v
    {993,   1015,  LIMITER_LEVEL_MINUS_30_DB},//<0.8v~0.82v
    {0,     990,  LIMITER_LEVEL_MINUS_90_DB},//<0.9v~0.92v  mute the system
};
/* Private functions / variables. Declare and drivers here */

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void AudioDrv_Ctor(cAudioDrv *me)
{
    ASSERT(me);
    
    //temperature and ac detect ctor
    tADCDevice *pADCConfigForAMP;
    tGPIODevice *pAmpMuteConfig;
    pADCConfigForAMP = (tADCDevice*)getDevicebyIdAndType(AUDIO_DEV_ID,ADC_DEV_TYPE,NULL);
    ADCDrv_Ctor(&adcDrvForAmp,pADCConfigForAMP);
    
    //AMP mute obj ctor 
    pAmpMuteConfig = (tGPIODevice*)getDevicebyIdAndType(AUDIO_DEV_ID,GPIO_DEV_TYPE,NULL);
    GpioDrv_Ctor(&gpioAmpMuteDrv,pAmpMuteConfig);
    //dsp driver ctor
    DSPDrv1761_Ctor(&dspDrv,&i2cObj);
    
    BOOL defaultValue = FALSE;
    Setting_Set(SETID_MUSIC_DET,&defaultValue);
}

void AudioDrv_Xtor(cAudioDrv *me)
{
    /* Fill me in! */
}

BOOL AudioDrv_Init(cAudioDrv *me)
{
    if(!(me->drvIsReady))
    {
        me->nextDelayTime = DSPDrv1761_Init(&dspDrv);
        if(me->nextDelayTime == 0)
        {
            me->drvIsReady = TRUE;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    return TRUE;
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
}

/**
 * Function to update audio status including jackin status,  music steam status and so on.
 */
void AudioDrv_UpdateStatus(cAudioDrv *audioDrvObj)
{
    AudioDrv_CheckJackInStatus();
    AudioDrv_CheckMusicStreamStatus();
    AudioDrv_AmpSafetyMonitor();
}

void AudioDrv_CheckJackInStatus()
{
    
}

/**
 * Function to monitor the amp status
 */
void AudioDrv_AmpSafetyMonitor()
{
    AMP_TempMonitor();
}

/**
 * Function to update the music stream status
 */
void AudioDrv_CheckMusicStreamStatus()
{
    BOOL sampleStatus = FALSE;
    BOOL preMusicSteamStatus = *((bool*)Setting_Get(SETID_MUSIC_DET));
    BOOL currentMusicStatus = FALSE;
    sampleStatus = DSPDrv1761_HasMusicStream(&dspDrv);
    musicCheckCnt++;
    if(musicCheckCnt <= MUSIC_STATUS_SAMPLES)
    {
      if(sampleStatus)
      {
          hasMusicCnt++;
      }
    }
    else
    {
        if(hasMusicCnt >= HAS_MUSIC_THRESHOLD)
        {
            currentMusicStatus = TRUE;
        }
        //sampling done, we got the result, now we compare the result with the previous status
        if(preMusicSteamStatus != currentMusicStatus)
        {
            AudioMusicDetectStateEvt* pAudioMusicStateEvt;
            pAudioMusicStateEvt = Q_NEW(AudioMusicDetectStateEvt, AUDIO_MUSIC_STREAM_STATE_SIG);
            pAudioMusicStateEvt->hasMusicStream = currentMusicStatus;
            Setting_Set(SETID_MUSIC_DET, &currentMusicStatus);
            QF_PUBLISH(&pAudioMusicStateEvt->super, dummy);
        }
        musicCheckCnt = 0;
        hasMusicCnt = 0;
    }
}

/**
 * Function to change audio settings, such as EQ.
 * @param[in]     pAudioEvt, a pointer of the audio event
 */
void AudioDrv_SetAudio(cAudioDrv *me, eAudioSettId audioSettId, BOOL enabled, uint32 param, uint32 param2)
{
    ASSERT(audioSettId < AUDIO_SETT_ID_MAX);
    DSPDrv1761_SetAudio(&dspDrv,audioSettId,enabled);
}

/**
 * Function to mute the system.
 * @param[in]   muteType    This parameter defines the way of how to mute the system,
 * you could mute the input source, or mute by dsp or mute the output
 * @param[in]   muteEnable  mute or unmute
 */
void AudioDrv_Mute(eAudioMuteType muteType, bool muteEnable)
{
    if(muteEnable)
    {
        DSPDrv1761_Mute(&dspDrv,AUDIO_DSP_DACOUT_MUTE,muteEnable);
        AMP_MUTE;
    }
    else
    {
        DSPDrv1761_Mute(&dspDrv,AUDIO_DSP_DACOUT_MUTE,muteEnable);
        AMP_UNMUTE;
    }
}


static void MainApp_AmpMute(BOOL muteEnable)
{
    if(muteEnable)
    {
        GpioDrv_SetBit(&gpioAmpMuteDrv,AMP_MUTE_IO);
    }
    else
    {
        GpioDrv_ClearBit(&gpioAmpMuteDrv,AMP_MUTE_IO);
    }
}

/**** Amplify temperature ****/
static void AMP_TempMonitor()
{
    uint8 i;
    int32 tempAdc;
    tempAdc = ADCDrv_GetData(&adcDrvForAmp,TEMPERATURE_ADC_PIN);
    for(i = 0; i < ArraySize(tempLimiter); i++)
    {
        if(tempAdc >= tempLimiter[i].adcValueMin && tempAdc <= tempLimiter[i].adcValueMax)
        {
            if(i == ArraySize(tempLimiter) - 1)// For very serious problem, we mute the system
            {
                AMP_MUTE;
                ampMuted = TRUE;
            }
            else
            {
                if(ampMuted)
                {
                    ampMuted = FALSE;
                    AMP_UNMUTE;
                }
                DspDrv1761_SetLimiter(&dspDrv, tempLimiter[i].limiterLevel);
            }
            break;
        }
    }
}
/**** End of Amplify temperature ****/