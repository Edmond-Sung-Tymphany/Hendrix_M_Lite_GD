/*
-------------------------------------------------------------------------------
TYMPHANY LTD

                  Audio Driver
                  -------------------------

                  SW Module Document
 
@file        AudioDrv.c
@brief       This file implementes the middle layer of the audio service ATMOS spec
@author      Viking Wang
@date        2016-05-30
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "./AudioDrv_priv.h"
#include "SettingSrv.h"
#include "trace.h"
#include "AudioSrv.h"
#include "ADCDrv.h"
#include "I2CDrv.h"
#include "GPIODrv.h"
#include "AudioDrv.config"
#include "timer.h"
#ifdef HAS_PCM9211_CODEC
#include "pcm9211_drv.h"
#endif
#ifdef HAS_EP9xAx_HDMI
#include "EpHDMI_drv.h"
#endif

typedef struct tCtrIdEQIdMap
{
    eAudioSettId dspSettid;
    eSettingId settingId;
}tCtrIdEQIdMap;

static tCtrIdEQIdMap ctrIdEQIdMap[] =
{
    /* DSP setting ID  index of setting db*/
    {DSP_VOLUME_SETT_ID,  SETID_VOLUME},

};

static cGpioDrv gpioAudio;

#ifdef HAS_SSM3582_AMP
#include "swi2c_drv.h"
#include "ssm3582_drv.h"
#include "./ssm3582_tbl.h"

static stSsm3582Drv_t ssm_W_L_Drv = {
    .isReady = FALSE,
    .p_regMap = ssm_reg_woofer_left,
    .deviceID = SSM3582_W_L_DEV_ID,
    .deviceType = SWI2C_DEV_TYPE,
};

static stSsm3582Drv_t ssm_T_L_LU_Drv = {
    .isReady = FALSE,
    .p_regMap = ssm_reg_tweeter_left_leftUP,
    .deviceID = SSM3582_T_L_LU_DEV_ID,
    .deviceType = SWI2C_DEV_TYPE,
};

static stSsm3582Drv_t ssm_T_C_Drv = {
    .isReady = FALSE,
    .p_regMap = ssm_reg_tweeter_center,
    .deviceID = SSM3582_T_C_DEV_ID,
    .deviceType = SWI2C_DEV_TYPE,
};

static stSsm3582Drv_t ssm_W_C_Drv = {
    .isReady = FALSE,
    .p_regMap = ssm_reg_woofer_center,
    .deviceID = SSM3582_W_C_DEV_ID,
    .deviceType = SWI2C_DEV_TYPE,
};

static stSsm3582Drv_t ssm_T_R_RU_Drv = {
    .isReady = FALSE,
    .p_regMap = ssm_reg_tweeter_right_rightUP,
    .deviceID = SSM3582_T_R_RU_DEV_ID,
    .deviceType = SWI2C_DEV_TYPE,
};

static stSsm3582Drv_t ssm_W_R_Drv = {
    .isReady = FALSE,
    .p_regMap = ssm_reg_woofer_right,
    .deviceID = SSM3582_W_R_DEV_ID,
    .deviceType = SWI2C_DEV_TYPE,
};
#endif  // HAS_SSM3582_AMP

static uint16 volFadeTimerId;
static tVolFadeParam* pVolFadeParam;

/* Private functions / variables. Declare and drivers here */

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void AudioDrv_Ctor(cAudioDrv *me)
{
    /* When bootup, AudioSrv call AudioDrv_Ctor() very soon, wait few seconds (after ASE-TK MCLK is ready),
     * then execute AudioDrv_Init() to initialize DSP.
     */
    ASSERT(me);

    //Initialize Audio GPIO pins
    gpioAudio.gpioConfig= (tGPIODevice*)getDevicebyIdAndType(AUDIO_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(gpioAudio.gpioConfig);
    GpioDrv_Ctor(&gpioAudio, gpioAudio.gpioConfig);

    //Initialize DSP

    //Shutdown all amplifier
//    AMP_SHUTDOWN(gpioAudio); //pull low SDZ
            
    //Enable power for DSP/AMP/NTC
    SYSPWR_ENABLE(gpioAudio);
    BRINGUP_printf("\n\rsystem power on...\n\r");
    BSP_BlockingDelayMs(20);    // delay for a while to wait the chip ready
}

void AudioDrv_Xtor(cAudioDrv *me)
{
    // dsp shutdown
    
    //Shutdown all amplifier
    AMP_SHUTDOWN(gpioAudio); //pull low SDZ
    //Disable power for DSP/AMP/NTC
    SYSPWR_DISABLE(gpioAudio);

    me->drvIsReady = FALSE;
}


/**
 * @Function to Init the low layer Audio related driver
 * @return This function return TRUE when audioDrv initialization is finished.
 */
BOOL AudioDrv_Init(cAudioDrv *me)
{
    if(!(me->drvIsReady))
    {        
        if(me->nextDelayTime == 0)
        {
            //Enable power for ADC/DSP/AMP/NTC
            SYSPWR_ENABLE(gpioAudio);
        }

#ifdef HAS_PCM9211_CODEC
        Pcm9211Drv_Ctor();
#ifdef BRINGUP_DEBUG
//        Pcm9211_MainOutputSetting(MAIN_OUTPUT_SOURCE_AUXIN0);   // test hdmi port
//        Pcm9211_MainOutputSetting(MAIN_OUTPUT_SOURCE_DIR);   // test hdmi port
//        Pcm9211_DirSourceSelect(0);
#endif
#endif

#ifdef HAS_SSM3582_AMP
        Ssm3582Drv_Ctor(&ssm_W_L_Drv);
        Ssm3582Drv_Ctor(&ssm_T_L_LU_Drv);
        Ssm3582Drv_Ctor(&ssm_T_C_Drv);
        Ssm3582Drv_Ctor(&ssm_W_C_Drv);
        Ssm3582Drv_Ctor(&ssm_T_R_RU_Drv);
        Ssm3582Drv_Ctor(&ssm_W_R_Drv);
#ifdef BRINGUP_DEBUG
    /*
        Ssm3582Drv_PowerControl(&ssm_W_R_Drv, 0);
        Ssm3582Drv_RegDump(&ssm_W_L_Drv);
        Ssm3582Drv_RegDump(&ssm_T_L_LU_Drv);
        Ssm3582Drv_RegDump(&ssm_T_C_Drv);
        Ssm3582Drv_RegDump(&ssm_W_C_Drv);
        Ssm3582Drv_RegDump(&ssm_T_R_RU_Drv);
        Ssm3582Drv_RegDump(&ssm_W_R_Drv);
        {
            int ii;
            for(ii=4; ii<0x1c; ii++)
            {
                BRINGUP_printf("\n\r reg[%d] = 0x%x\n\r", ii, ssm_W_R_Drv.p_regMap[ii]);
            }
        }
    */    
#endif
#endif

#ifdef HAS_EP9xAx_HDMI
        EpHDMIDrv_Ctor();
#endif
        me->nextDelayTime = 0;
        if(me->nextDelayTime == 0) 
        {
            //Read DSP Version
            char dspVersion[DSP_VERSION_LENGTH]= {"0.1"};
            Setting_Set(SETID_DSP_VER, dspVersion);

             /* It is important to note that control port register changes should only occur when the device is placed into
             *  shutdown. This can be accomplished either by pulling the SPK_SD pin "LOW" or clearing the SPK_SD bit in the
             *  control port.
             */
            
            /* Pull SDZ to high after I2C programming */
            AMP_WAKEUP(gpioAudio);
            
            //AudioAmpDrv_printError(&audioAmpWoofer1Drv);
//            AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, /*muteEnable:*/TRUE);

            /* Set default settings */
            bool defaultValue = FALSE;
            Setting_Set(SETID_IS_AUXIN_PLUG_IN, &defaultValue);
            Setting_Set(SETID_IS_SPDIF_IN_PLUG_IN, &defaultValue);
            Setting_Set(SETID_AUXIN_MUSIC_DET, &defaultValue);
            Setting_Set(SETID_SPDIF_IN_MUSIC_DET, &defaultValue);
            Setting_Set(SETID_ASETK_MUSIC_DET, &defaultValue);
            tVolFadeParam volFadeParam = {0};
            Setting_Set(SETID_VOLUME_FADE_PARAM, &volFadeParam);
            uint8 defaultVol = DEFAULT_VOLUME;
            Setting_Set(SETID_VOLUME, &defaultVol);

            //Finish
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
    AudioDrv_AmpSafetyMonitor();
    AudioDrv_CheckJackInStatus();
    AudioDrv_CheckMusicStreamStatus();
#ifdef BRINGUP_DEBUG
{
/*    uint8_t sample_rate;
    sample_rate = Pcm9211_GetSampleRate();
//    BRINGUP_printf("\n\rsample rate = 0x%x\n\r", sample_rate);
    sample_rate = sample_rate + 1;
    sample_rate --;
*/
}
#endif
}

void AudioDrv_CheckJackInStatus(void)
{
}

/**
 * Function to update the music stream status
 */
void AudioDrv_CheckMusicStreamStatus(void)
{
}

/**
 * Function to change audio settings, such as EQ.
 * @param[in]     pAudioEvt, a pointer of the audio event
 */
void AudioDrv_SetAudio(cAudioDrv *me, eAudioSettId audioSettId, BOOL enabled, uint32 param)
{
    ASSERT(audioSettId < AUDIO_SETT_ID_MAX);

    /* When bootup, DSP do not initialize, do not access DSP on this moment */
    if(!(me->drvIsReady)) {
        TP_PRINTF("AudioDrv_SetAudio: driver is not ready");
        return;
    }

    switch (audioSettId)
    {
        case DSP_VOLUME_SETT_ID:
            TP_PRINTF("AudioDrv_SetAudio: id=(%d)DSP_VOLUME_SETT_ID\r\n", audioSettId);
            pVolFadeParam = (tVolFadeParam*)Setting_Get(SETID_VOLUME_FADE_PARAM);
            if(!pVolFadeParam->isFadingInProg)
            {
                uint8 volume= (uint8)param;
                Setting_Set(SETID_VOLUME, &volume);
                AudioDrv_DspVolSetId_Handler();
            }
            break;

        case DSP_PASSTHROUGH_SETT_ID:
            break;
        case SYSTEM_GAIN_SETT_ID:
            // CHANGE THE INPUT GAIN FOR ADC OR DSP
            break;
        case AMP_SLEEP_MODE_ID:
            break;
        case DSP_TUNNING_SETT_ID:
        {
            break;
        }
        case AUDIO_VOL_FADE_SETT_ID:
        {
            pVolFadeParam = (tVolFadeParam*)Setting_Get(SETID_VOLUME_FADE_PARAM);
            pVolFadeParam->isFadingInProg = TRUE;
            Timer_StartTimer((uint32)pVolFadeParam->fadeStepTime, &volFadeTimerId, AudioDrv_VolFadeTimerCallBack, pVolFadeParam);
            break;
        }
        default:
             break;
    }
}

static void AudioDrv_VolFadeTimerCallBack(void *pCbPara)
{
    
    tVolFadeParam* pVolFadeParam = (tVolFadeParam*)pCbPara;
    /* get current volume value */
    uint8 volLevel = *(uint8*)Setting_Get(SETID_VOLUME);
    /* change volume*/
    if(pVolFadeParam->fadeType)
    {
        /* volume fade up*/
        if(pVolFadeParam->targetVol < (volLevel + pVolFadeParam->fadeStep))
        {
            volLevel = pVolFadeParam->targetVol;
        }
        else if(MAX_VOLUME < (volLevel + pVolFadeParam->fadeStep))
        {
            volLevel = MAX_VOLUME;
        }
        else
        {
            volLevel += pVolFadeParam->fadeStep;
        }
    }
    else
    {
        /* volume fade down*/
        if(pVolFadeParam->targetVol > (volLevel - pVolFadeParam->fadeStep))
        {
            volLevel = pVolFadeParam->targetVol;
        }
        else if(pVolFadeParam->fadeStep > (volLevel - MIN_VOLUME))
        {
            volLevel = MIN_VOLUME;
        }
        else
        {
            volLevel -= pVolFadeParam->fadeStep;
        }
    }
    Setting_Set(SETID_VOLUME, &volLevel);
    AudioDrv_DspVolSetId_Handler();

    if((volLevel != pVolFadeParam->targetVol)
        && (volLevel < MAX_VOLUME) && (volLevel > MIN_VOLUME))
    {
        Timer_StartTimer(pVolFadeParam->fadeStepTime, &volFadeTimerId, AudioDrv_VolFadeTimerCallBack, pVolFadeParam);
    }
    else
    {
        pVolFadeParam->isFadingInProg = FALSE;
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
            TP_PRINTF("AudioDrv_Mute: muteType=AUDIO_AMP_SOFT_MUTE, muteEnable=%d\n\r", muteEnable);
            break;
        default:
            //Support only soft mute
            ASSERT(0);
            break;
    }
}

/**
 * Function to monitor the amp status
 */
void AudioDrv_AmpSafetyMonitor()
{
    AMP_TempMonitor();
}

/**
 * Function to update amplifier temperature.
 */
static void AMP_TempMonitor()
{
}

static void AudioDrv_DspVolSetId_Handler(void)
{
    uint8 volumeLevel = MIN_VOLUME;

    volumeLevel =  *(uint8*)Setting_Get(ctrIdEQIdMap[DSP_VOLUME_SETT_ID].settingId);
    if(volumeLevel <= MAX_VOLUME && volumeLevel >= MIN_VOLUME)
    {
        // ATMOS: set dsp volume here.
    }
    else
    {
        ASSERT(0);
    }    
}

