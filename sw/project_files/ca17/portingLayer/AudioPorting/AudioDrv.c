/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Audio Driver
                  -------------------------

                  SW Module Document

@file        AudioDrv.c
@brief       This file implementes the middle layer of the audio service BnO Playbox Specific
@author      Edmond Sung
@date        2015-10-15
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
#include "AseNgSrv.h"
#include "ADCDrv.h"
#include "I2CDrv.h"
#include "GPIODrv.h"
#include "DspDrv1761.h"
#include "AudioDrv.config"
#include "timer.h"



/***********************************************
 * Definition
 ***********************************************/
//#define AUDIO_DRV_TEMP_DEBUG
#define AUDIO_DRV_ENABLE_OVERHEAT_PROTECTION



/***********************************************
 * Type
 ***********************************************/
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



/***********************************************
 * Extern Function
 ***********************************************/
extern void DSPDrv1761_WriteDcValue32_0(cDSPDrv1761 *me, uint16 reg_addr, int32 data);
extern void DSPDrv1761_WriteValue5_23(cDSPDrv1761 *me, uint16 reg_addr, float fValue);
extern float DSPDrv1761_ReadValue5_23(cDSPDrv1761 *me, uint16 reg_addr);
extern void DSPDrv1761_SetAuxinMute(cDSPDrv1761 *me, bool auxin_mute);



/***********************************************
 * Global Variable
 ***********************************************/
static cI2CDrv      dspI2cObj;
static cDSPDrv1761 DspADAU1761Drv;

#ifdef AUDIO_DRV_TEMP_DEBUG
int32 dbgTempIncr= 20;  //when init audio drv, init temp +40C.
int32 dbgDiff= 1;
#endif

static cGpioDrv gpioAudio;

static bool audioOverheat= FALSE;

cADCDrv adcAudio;

static int32 musicDetTimer;
static int32 jackDetTimer;
static int32 tempMonTimer;

static uint16 volFadeTimerId= 0;
static tVolFadeParam volFadeParam= {0};

/* Record requester id for volume fading request */
static uint32 requester_id = 0;

static float auxin_input_db_smooth= -90.0;
static bool hasMusicStream_DebNew= FALSE;
static bool hasMusicStream_DebPre= FALSE;
static uint32 auxin_detect_debounce_msec= 0;



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
    audioOverheat = FALSE;

    //Initialize Audio GPIO pins
    gpioAudio.gpioConfig= (tGPIODevice*)getDevicebyIdAndType(AUDIO_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(gpioAudio.gpioConfig);
    GpioDrv_Ctor(&gpioAudio, gpioAudio.gpioConfig);

    //Initialize Audio ADC pins
    adcAudio.ADCConfig= (tADCDevice*)getDevicebyIdAndType(AUDIO_DEV_ID, ADC_DEV_TYPE, NULL);
    ASSERT(adcAudio.ADCConfig);
    ADCDrv_Ctor(&adcAudio, adcAudio.ADCConfig);

    //Initialize DSP
    dspI2cObj.pConfig = (tI2CDevice*)getDevicebyIdAndType(DSP_DEV_ID, I2C_DEV_TYPE, NULL);
    ASSERT(dspI2cObj.pConfig);
    DSPDrv1761_Ctor(&DspADAU1761Drv, &dspI2cObj);

    //Shutdown all amplifier
    AMP_SHUTDOWN(gpioAudio); //pull low SDZ
}

void AudioDrv_Xtor(cAudioDrv *me)
{
    DSPDrv1761_Xtor(&DspADAU1761Drv);

    //Shutdown all amplifier
    AMP_MUTE(gpioAudio);
    AMP_SHUTDOWN(gpioAudio); //pull low SDZ

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
            //1st step
        }

        //2nd-Nst step
        me->nextDelayTime = DSPDrv1761_Init(&DspADAU1761Drv);
        
        //Last step
        if(me->nextDelayTime == 0)
        {
            //Set DSP to slave mode
            DspDrv1761_SetI2Smode(&DspADAU1761Drv, /*isMaster:*/FALSE);
            AudioDrv_SetChannel(AUDIO_CHNANNEL_ASE);
            
            //Read DSP Version
            float dspVer = 0.0;
            DSPDrv1761_GetDspVer(&DspADAU1761Drv, &dspVer);
            Setting_Set(SETID_DSP_VER, &dspVer);
            
            /* When DSP is unstable, DSP version read might be 0.000xxx 
             * Error handling is on MainApp_SysTempCheckTimeoutHandler()
             */
            bool isDspErr= FALSE;
            if( dspVer<DSP_MIN_VER || dspVer>DSP_MAX_VER )
            {
                isDspErr= TRUE;
                TP_PRINTF("\r\n\r\n\r\nERROR: <<<<< dsp version %02.2f is wrong, might due to DSP unstable. Dsp may be not sound output >>>>>\r\n\r\n\r\n\r\n", dspVer);
            }
            else
            {
                TP_PRINTF("\r\nDSP Version: %02.2f\r\n\r\n", dspVer);
            }            
            Setting_Set(SETID_IS_DSP_ERROR, &isDspErr);

            /* Pull SDZ to high after I2C programming */
            AMP_SHUTDOWN(gpioAudio);
            AMP_MUTE(gpioAudio);

            /* Set default settings */
            jackDetTimer = AUDIO_ACTIVE_AUXIN_JACK_DET_TIME;
            musicDetTimer = AUDIO_ACTIVE_MUSIC_DETECT_TIME;
            bool defaultValue = FALSE;
            Setting_Set(SETID_AUXIN_JACK_DET, &defaultValue);
            Setting_Set(SETID_AUXIN_MUSIC_DET, &defaultValue);
            DSPDrv1761_SetAuxinMute(&DspADAU1761Drv, /*auxin_mute:*/!defaultValue);

            memset(&volFadeParam, 0, sizeof(volFadeParam));
            volFadeParam.isFadingInProg = FALSE;
            uint8 defaultVol = DEFAULT_VOLUME;
            Setting_Set(SETID_VOLUME, &defaultVol);
            DSPDrv1761_SetAudio(&DspADAU1761Drv, DSP_VOLUME_SETT_ID, TRUE);
            
            
            /* ASE v1.0.7601 do not support line-in sensitivity setting, thus we 
             * forcely enable currently. When ASE fix bug, it should over-write this
             * setting after bootup
             */
            uint32 sensitivityLineIn = Proto_Dsp_LineInSensitivity_Sensitivity_MEDIUM; //type: Proto_Dsp_LineInSensitivity_Sensitivity
            Setting_Set(SETID_SENSITIVITY_LINEIN, &sensitivityLineIn);
            
            /* Set to NORMAL temp level, to make sure boot threshold is 76, not 73
             */
            eTempLevel tempLevel= TL_NORMAL;
            Setting_Set(SETID_AMP_TEMP_LEVEL, &tempLevel);
            int16 tempAmp= 0;
            Setting_Set(SETID_AMP_TEMP, &tempAmp);

            double toneTouchVal= DSP_TONE_TOUCH_DEFAULT_Gx, toneTouchValK=DSP_TONE_TOUCH_DEFAULT_Kx;
            bool toneTouchEnabled= FALSE;
            Setting_Set(SETID_TONE_TOUCH_ENABLED, &toneTouchEnabled);
            Setting_Set(SETID_TONE_TOUCH_GX1,     &toneTouchVal);
            Setting_Set(SETID_TONE_TOUCH_GX2,     &toneTouchVal);
            Setting_Set(SETID_TONE_TOUCH_GY1,     &toneTouchVal);
            Setting_Set(SETID_TONE_TOUCH_GY2,     &toneTouchVal);
            Setting_Set(SETID_TONE_TOUCH_GZ,      &toneTouchVal);
            Setting_Set(SETID_TONE_TOUCH_K5,      &toneTouchValK);
            Setting_Set(SETID_TONE_TOUCH_K6,      &toneTouchValK);
                
            //Position
            eSpeakerPosition position = DSP_POSITION_DEFAULT;
            Setting_Set(SETID_SPEAKER_POSITION, &position);
            DSPDrv1761_SetSpeakerPosition(&DspADAU1761Drv, position);
            
            /* Update temp and level, to let MainApp check overheat on boot up
             */
            tempMonTimer= AUDIO_SRV_TIMEOUT_IN_MS; //trigger temp monitor now
            musicDetTimer = AUDIO_SRV_TIMEOUT_IN_MS;
            jackDetTimer = AUDIO_SRV_TIMEOUT_IN_MS;
            AudioDrv_TempMonitor();
            
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
    DSPDrv1761_SetInputChannel(&DspADAU1761Drv, audioChannel);
}

void AudioDrv_SetLineInMultiRoomChannel(eAudioChannel audioChannel)
{
//    DSPDrv1451_SetLineInMultiRoomChannel(&DspADAU1451Drv, audioChannel);
}



void AudioDrv_UpdateStatus(cAudioDrv *audioDrvObj)
{
    AudioDrv_TempMonitor();
    AudioDrv_CheckJackInStatus();
    AudioDrv_CheckMusicStreamStatus();
}


/* When jack in:
 *   1 unmute auxin input, MCU may detect auxin signel, and let ASE trigger source change
 *   2 DSP wakeup to detect auxin signal
 *  
 * When jack not appear:
 *   1 mute auxin input
 *   2 dsp sleep when no audio output
 */
void AudioDrv_CheckJackInStatus(void)
{
    uint8 i = 0;
    if ((jackDetTimer > 0) && ((jackDetTimer -= AUDIO_SRV_TIMEOUT_IN_MS) <= 0))
    {
        bool jackIn = GpioDrv_ReadBit(&gpioAudio, GPIO_IN_AUXIN_JACK_DET);
        bool preJackIn = *(bool*)Setting_Get(SETID_AUXIN_JACK_DET);
            
        if (jackIn != preJackIn)
        {
            TP_PRINTF("Auxin Jack = %d\r\n", jackIn);
            Setting_Set(SETID_AUXIN_JACK_DET, &jackIn);
            DSPDrv1761_SetAuxinMute(&DspADAU1761Drv, /*auxin_mute:*/!jackIn);
        }
        jackDetTimer = AUDIO_ACTIVE_AUXIN_JACK_DET_TIME;
    }
}


/**
 * Function to update the music stream status
 */
void AudioDrv_CheckMusicStreamStatus(void)
{
    if ((musicDetTimer -= AUDIO_SRV_TIMEOUT_IN_MS) <= 0)
    {
        musicDetTimer = AUDIO_ACTIVE_MUSIC_DETECT_TIME;
        bool hasMusicStream= *(bool*)Setting_Get(SETID_AUXIN_PLAYING);

        uint32 sensitivityLineIn = Proto_Dsp_LineInSensitivity_Sensitivity_DISABLED; //type: Proto_Dsp_LineInSensitivity_Sensitivity
        sensitivityLineIn= *(uint32*)Setting_GetEx(SETID_SENSITIVITY_LINEIN, &sensitivityLineIn);
        int8 sensitivityThresholdDb;
        if(sensitivityLineIn==Proto_Dsp_LineInSensitivity_Sensitivity_HIGH)
        {
            sensitivityThresholdDb= AUDIO_AUXIN_DETECT_HIGH_DB;
        }
        else if(sensitivityLineIn==Proto_Dsp_LineInSensitivity_Sensitivity_MEDIUM)
        {
            sensitivityThresholdDb= AUDIO_AUXIN_DETECT_MID_DB;
        }
        else if(sensitivityLineIn==Proto_Dsp_LineInSensitivity_Sensitivity_LOW)
        {
            sensitivityThresholdDb= AUDIO_AUXIN_DETECT_LOW_DB;
        }
        else if(sensitivityLineIn==Proto_Dsp_LineInSensitivity_Sensitivity_DISABLED)
        {
            hasMusicStream_DebNew= FALSE;
            hasMusicStream_DebPre= FALSE;
            auxin_detect_debounce_msec= 0;
        }
        else
        {
            ASSERT(0);
        }

        float auxin_input_db= 0.0;        
        bool preJackIn = *(bool*)Setting_Get(SETID_AUXIN_JACK_DET);
        if( preJackIn )
        {
            DSPDrv1761_GetSigLevel(&DspADAU1761Drv, &auxin_input_db);
        }
        else
        {
            auxin_input_db= -90.0;
        }

        /* Some music do not have constant volume and hard to pass debounce,
         * thus we get smooth volume then pass to debounce.
         */
        auxin_input_db_smooth= (auxin_input_db*AUDIO_AUXIN_DETECT_SMOOTH_RATE) + (auxin_input_db_smooth*(1-AUDIO_AUXIN_DETECT_SMOOTH_RATE));        
        Setting_Set(SETID_AUDIO_AUXIN_IN_DB, &auxin_input_db_smooth);

        //hasMusicStream_DebNew= (auxin_input_db>=sensitivityThresholdDb)?TRUE:FALSE;
        hasMusicStream_DebNew= (auxin_input_db_smooth>=sensitivityThresholdDb)?TRUE:FALSE;

//        TP_PRINTF("aux=%.1fdB, aux-smooth=%.1fdB, new=%d, pre=%d, curr=%d, time=%dms, threshold=%ddB[L%d]\r\n",
//                  auxin_input_db, auxin_input_db_smooth, hasMusicStream_DebNew,
//                  hasMusicStream_DebPre, hasMusicStream, auxin_detect_debounce_msec,
//                  sensitivityThresholdDb, sensitivityLineIn);

        if(sensitivityLineIn==Proto_Dsp_LineInSensitivity_Sensitivity_DISABLED)
        {
        }
        if(hasMusicStream_DebNew==hasMusicStream)
        {
            //Do not need debounce
            auxin_detect_debounce_msec= 0;
        }
        else if(hasMusicStream_DebNew!=hasMusicStream_DebPre)
        {
            //stop debounce
            auxin_detect_debounce_msec= 0;
            //TP_PRINTF("aux-in: debounce fail\r\n");
        }
        else
        {
            //start or continue debounce (new == pre == curr)
            if(auxin_detect_debounce_msec <= AUDIO_AUXIN_DETECT_PEROID_MSEC)
            {
                auxin_detect_debounce_msec+= AUDIO_SRV_TIMEOUT_IN_MS;
                //TP_PRINTF("aux: debouncing\r\n");
            }
            else
            {
                //finish debounce
                hasMusicStream= hasMusicStream_DebNew;
                Setting_Set(SETID_AUXIN_PLAYING, &hasMusicStream);
                AudioMusicDetectStateEvt* pAudioMusicStateEvt;
                pAudioMusicStateEvt = Q_NEW(AudioMusicDetectStateEvt, AUDIO_MUSIC_STREAM_STATE_SIG);
                pAudioMusicStateEvt->hasMusicStream = hasMusicStream;
                Setting_Set(SETID_AUXIN_PLAYING, &hasMusicStream);
                QF_PUBLISH(&pAudioMusicStateEvt->super, me);
                TP_PRINTF("\r\n*** Aux-in music states is %d ***\r\n\r\n", pAudioMusicStateEvt->hasMusicStream);
            }
        }
        hasMusicStream_DebPre= hasMusicStream_DebNew;
    }
}


static void AudioDrv_StopFadeVolume()
{
    if (volFadeParam.isFadingInProg == TRUE)
    {
        TP_PRINTF("Volume fade: Stop volume fading and reply ASE (red_id=%d)\r\n", requester_id);
        AseNgSrv_ReplyVolumeFade(requester_id, TRUE);
        bool ret= Timer_StopTimer(volFadeTimerId);
        ASSERT(ret); //FALSE means Fade volume timer is not running
        volFadeParam.isFadingInProg = FALSE;
        requester_id = 0;
    }
}


static void AudioDrv_SetFadeVolume(cAudioDrv *me, uint32 target_volume, uint32 fade_duration)
{
    /* The start volume should always be equal to current volume. */
    uint8 start_volume = *(uint8*)Setting_Get(SETID_VOLUME);
    uint32 fadeRange = TYM_DIFF(target_volume, start_volume);
    
    /* fade_duration should not be less than ADUIO_MIN_FADE_STEP_TIME */
    if(fade_duration < ADUIO_MIN_FADE_STEP_TIME)
    {
        fade_duration= ADUIO_MIN_FADE_STEP_TIME;
    }

    if(((fadeRange <= (MAX_VOLUME - MIN_VOLUME)) && fadeRange > 0)
        && (target_volume >= MIN_VOLUME && target_volume <= MAX_VOLUME)
        && (start_volume >= MIN_VOLUME && start_volume <= MAX_VOLUME))
    {
        volFadeParam.fadeType     = (target_volume > start_volume)? TRUE : FALSE;
        volFadeParam.fadeStep     = ADUIO_MIN_FADE_STEP;
        volFadeParam.fadeStepTime = (fade_duration * volFadeParam.fadeStep)/fadeRange;
        volFadeParam.targetVol    = target_volume;
        volFadeParam.isFadingInProg = TRUE;
        if(ADUIO_MIN_FADE_STEP_TIME > volFadeParam.fadeStepTime)
        {
            volFadeParam.fadeStepTime = ADUIO_MIN_FADE_STEP_TIME;
            volFadeParam.fadeStep = (volFadeParam.fadeStepTime * fadeRange)/fade_duration;
            ASSERT(volFadeParam.fadeStep>0);
        }
        Timer_StartTimer((uint32)volFadeParam.fadeStepTime, &volFadeTimerId, AudioDrv_VolFadeTimerCallBack, &volFadeParam);
    }
    else
    {
        TP_PRINTF("The volume fade parameters are invalid. \r\n");
        //TODO: reply FAIL to ASE
    }


}

/**
 * Function to change audio settings, such as EQ.
 * @param[in]     pAudioEvt, a pointer of the audio event
 */
void AudioDrv_SetAudio(cAudioDrv *me, eAudioSettId audioSettId, BOOL enabled, uint32 param, uint32 param2)
{
    ASSERT(audioSettId < AUDIO_SETT_ID_MAX);

    /* When bootup, DSP do not initialize, do not access DSP on this moment */
    if(!(me->drvIsReady)) {
        TP_PRINTF("AudioDrv_SetAudio: driver is not ready\r\n");
        return;
    }

    switch (audioSettId)
    {
        case DSP_VOLUME_SETT_ID:
        {
            AudioDrv_StopFadeVolume();                
            uint8 volume= (uint8)param;
            Setting_Set(SETID_VOLUME, &volume);
            AudioDrv_DspVolSetId_Handler();
            break;
        }
        case AMP_SLEEP_MODE_ID:
        {
            if(!audioOverheat)
            {
                //TP_PRINTF("amp shutdown=%d\r\n", enabled);
                if (enabled)
                {
                    AMP_SHUTDOWN(gpioAudio);
                }
                else
                {
                    AMP_WAKEUP(gpioAudio);
                }
            }
            break;
        }
        case AUDIO_OVERHEAT_MODE_ID:
        {
            AudioDrv_SetOverheat(me, enabled);
            break;
        }
#ifdef HAS_DSP_TUNING_MODE
        case DSP_TUNNING_SETT_ID:
        {
            /* In DSP tuning mode, always power on and unmute amplifier */
            TP_PRINTF("DSP-TUNNING = %d\r\n", enabled);
            if (enabled)
            {
                AMP_WAKEUP(gpioAudio);
                AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, FALSE);
                AudioDrv_SetChannel(AUDIO_CHANNEL_I2S_2); // auxin
            }
            /* DSP and AMP I2C bus should be disable when tuning DSP. */
            DSPDrv1761_I2cEnable(&DspADAU1761Drv, !enabled);
            break;
        }
#endif
        case AUDIO_VOL_FADE_SETT_ID:
        {  /* param ( vv:ffffff    8 bits volume : 24 bits fade duration )
              param2 request id */
            uint32 target_volume= param >> 24;
            requester_id = param2;
            uint32 fade_duration= param & 0xFFFFFF;
            AudioDrv_StopFadeVolume();
            AudioDrv_SetFadeVolume(me, target_volume, fade_duration);
            break;
        }
        case AUDIO_LINEIN_SENSITIVITY_SETT_ID:
        {
            uint32 level = param;
            Setting_Set(SETID_SENSITIVITY_LINEIN, &level);
            break;
        }
        case DSP_WRITE_TONE_TOUCH_SETT_ID:
        {
            bool toneTouchEnabled= *(bool*)Setting_Get(SETID_TONE_TOUCH_ENABLED);
            double toneTouchGx1= *(double*)Setting_Get(SETID_TONE_TOUCH_GX1);
            double toneTouchGx2= *(double*)Setting_Get(SETID_TONE_TOUCH_GX2);
            double toneTouchGy1= *(double*)Setting_Get(SETID_TONE_TOUCH_GY1);
            double toneTouchGy2= *(double*)Setting_Get(SETID_TONE_TOUCH_GY2);
            double toneTouchGz=  *(double*)Setting_Get(SETID_TONE_TOUCH_GZ);
            double toneTouchK5=  *(double*)Setting_Get(SETID_TONE_TOUCH_K5);
            double toneTouchK6=  *(double*)Setting_Get(SETID_TONE_TOUCH_K6);
            
            DSPDrv1761_SetToneTouchEnable(&DspADAU1761Drv, toneTouchEnabled);
            DSPDrv1761_WriteValue5_23(&DspADAU1761Drv, (uint16)DSP_TONE_TOUCH_GX1_REGISTER, toneTouchGx1);
            DSPDrv1761_WriteValue5_23(&DspADAU1761Drv, (uint16)DSP_TONE_TOUCH_GX2_REGISTER, toneTouchGx2);
            DSPDrv1761_WriteValue5_23(&DspADAU1761Drv, (uint16)DSP_TONE_TOUCH_GY1_REGISTER, toneTouchGy1);
            DSPDrv1761_WriteValue5_23(&DspADAU1761Drv, (uint16)DSP_TONE_TOUCH_GY2_REGISTER, toneTouchGy2);
            DSPDrv1761_WriteValue5_23(&DspADAU1761Drv, (uint16)DSP_TONE_TOUCH_GZ_REGISTER,  toneTouchGz);
            DSPDrv1761_WriteValue5_23(&DspADAU1761Drv, (uint16)DSP_TONE_TOUCH_K5_REGISTER,  toneTouchK5);
            DSPDrv1761_WriteValue5_23(&DspADAU1761Drv, (uint16)DSP_TONE_TOUCH_K6_REGISTER,  toneTouchK6);
            
            TP_PRINTF("\r\n\r\nDSP ToneTouch: enabled=%d, Gx1=%f, Gx2=%f, Gy1=%f, Gy2=%f, Gz=%f, k5=%f, k6=%f\r\n\r\n\r\n", 
                        toneTouchEnabled, toneTouchGx1, toneTouchGx2, toneTouchGy1, toneTouchGy2,
                        toneTouchGz, toneTouchK5, toneTouchK6);
            break;
        }
        case AUDIO_POS_SOUND_MODE_SETT_ID:
        {
            eSpeakerPosition position = (eSpeakerPosition)param;
            Setting_Set(SETID_SPEAKER_POSITION, &position);
            DSPDrv1761_SetSpeakerPosition(&DspADAU1761Drv, position);
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

    //printf("AudioDrv_VolFadeTimerCallBack: fadeType=%d, step=%d, vol:(curr=%d,tgt=%d) \r\n",
    //           pVolFadeParam->fadeType, pVolFadeParam->fadeStep, volLevel, pVolFadeParam->targetVol);

    ASSERT(pVolFadeParam->isFadingInProg==TRUE);

    /* For issue FS-202, sometimes AudioDrv_VolFadeTimerCallBack() repeated execute and set the same volume.
     * The only possible condition is fadeStep==0, but there is no reason for it.
     * Before find root cause, we add workaround here.
     */
    if(pVolFadeParam->fadeStep==0)
    {
        ASSERT(0);
        TP_PRINTF("\r\n\r\n\r\n*** ERROR: AudioDrv_VolFadeTimerCallBack fadeStep==0 ***\r\n\r\n\r\n\r\n");
        return;
    }

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
        /* Reply Ase-tk when volume fade completed. */
        AudioDrv_StopFadeVolume();
        TP_PRINTF("Volume fade: finish, vol=%d, req_id=%d\r\n", volLevel, requester_id);
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
        {
            TP_PRINTF("amp mute=%d\r\n", muteEnable);
            if (muteEnable)
            {
                AMP_MUTE(gpioAudio);
            }
            else
            {
                AMP_UNMUTE(gpioAudio);
            }
            //TP_PRINTF("amp-mute=%d\n\r", muteEnable);
            break;
        }
        default:
            break;
    }
}

static uint16 AudioDrv_ConvertNtcToTemperature(uint16 ntcValue)
{
    uint8 degreeC;

    for (degreeC = 0; degreeC < TEMPERATURE_POSITIONS_NUM; degreeC++)
    {
        if (ntcValue > audioNtcTempTable[degreeC])
        {
            break;
        }
    }
    return degreeC;
}

static void AudioDrv_SetOverheat(cAudioDrv *me, bool overheat)
{
    /* audioOverheat occurs: Shutdown ampliifer for over-heat
     * recover from audioOverheat: do nothing, but allow MainApp to wakeup amplifier
     */
    TP_PRINTF("AudioDrv_SetOverheat: overheat=%d\r\n", overheat);
    audioOverheat= overheat;
    if(overheat)
    {
        AMP_SHUTDOWN(gpioAudio);
    }
    else
    {
        AMP_WAKEUP(gpioAudio);
    }
}


void AudioDrv_UpdateTempLevel(int16 tempNew, int16 tempOld, eTempLevel *pLevel, const sRange *levels, uint32 numLevel)
{
    eTempLevel tempLevelNew;
    int16 i;
    
    //config check
    ASSERT( numLevel==TL_NUM );

    if( tempOld < tempNew  ) //raise curve
    {
        //Check if new curve cross over "lower bound"
        for (i = 0; i < numLevel; i++)
        {
            if ( tempOld<=levels[i].lower && levels[i].lower<=tempNew  )
            {
                tempLevelNew= (eTempLevel)i;
                break;
            }
        }
        //Do not cross over, keep previous level
        if(i >= numLevel)
        {
            tempLevelNew= *pLevel;
        }
    }
    else //down curve (include equal)
    {
        //Check if new curve cross over "upper bound"
        for (i = numLevel-1; i >=0; i--)
        {
            if ( tempOld>=levels[i].upper && levels[i].upper >= tempNew )
            {
                tempLevelNew= (eTempLevel)i;
                break;
            }
        }
        //Do not cross over, keep previous level
        if(i < 0)
        {
            tempLevelNew= *pLevel;
        }
    }
    
    /* When boot up, temperature leavel may not mapping to temperature
     * For this case, the above for-loop can not assign level, thus we assign here
     */
    if(tempNew<=levels[TL_NORMAL].upper)
    {
        tempLevelNew= TL_NORMAL;
    }
    if(tempNew>=levels[TL_CRITICAL].lower)
    {
        tempLevelNew= TL_CRITICAL;
    }
    ASSERT(tempLevelNew<TL_NUM);
    
    //Store result
    *pLevel= tempLevelNew;
}


/**
 * Function to update amplifier temperature.
 */
static void AudioDrv_TempMonitor()
{
    int16 rawResult = 0;
    //int16 tempWf = 0;
    int16 tempAmpNew = 0;
    //int16 tempDsp = 0;
    
    static float tempAmpToDsp= 0.0;

    if (tempMonTimer > 0)
    {
        if ((tempMonTimer -= AUDIO_SRV_TIMEOUT_IN_MS) <= 0)
        {
            tempMonTimer = AUDIO_AMP_TEMP_CHECK_TIME;
            
//            //print PLL
//            static uint32 timer_pll= 0;
//            timer_pll++;
//            if( timer_pll % (AUDIO_DRV_PRINT_TIMEOUT_IN_MS*10/AUDIO_AMP_TEMP_CHECK_TIME) == 0)
//            {
//                bool pll_lock= DSPDrv1761_IsPllLock(&DspADAU1761Drv);
//            }
            

            /* update Amplifier temperature */
            rawResult = (int16)ADCDrv_GetData(&adcAudio, ADC_AMP_NTC);
        
            /* Because ES and EVT1 have old NTC, enable overheat protect only for 
             * EVT2 and later HW (DVT, MP)
             */
            eHwVer hwVerIndex= (eHwVer)( *(uint32 *)Setting_Get(SETID_HW_VER_INDEX) );
            if( hwVerIndex<=HW_TYPE_EVT2  &&  (ADC_DATA_IS_NOT_READY != rawResult) )
            {
                //level
                eTempLevel tempLevel= 0;
                tempLevel= *(eTempLevel*)Setting_GetEx(SETID_AMP_TEMP_LEVEL, &tempLevel);

                //temperature
                int16 tempAmpOld=0;
                tempAmpOld= *(int16*)Setting_GetEx(SETID_AMP_TEMP, &tempAmpOld);
                tempAmpNew= AudioDrv_ConvertNtcToTemperature(rawResult);
#ifdef AUDIO_DRV_TEMP_DEBUG
                tempAmpNew+= (float)dbgTempIncr;
#endif                    
                Setting_Set(SETID_AMP_TEMP, &tempAmpNew); 
#ifdef AUDIO_DRV_ENABLE_OVERHEAT_PROTECTION
                
                tempAmpToDsp= (tempAmpNew);
                DSPDrv1761_WriteValue5_23(&DspADAU1761Drv, (uint16)DSP_TEMP_NTC_REGISTER, tempAmpToDsp/100.0 );

                //update level and dsp             
                AudioDrv_UpdateTempLevel(tempAmpNew, tempAmpOld, &tempLevel, ampTempLevels, ArraySize(ampTempLevels));
                Setting_Set(SETID_AMP_TEMP_LEVEL, &tempLevel);
#endif                
            }

            /* Scan for new values */
            ADCDrv_StartScanning(&adcAudio);
            
            /* Print log */
            bool hasMusicStream= *(bool*)Setting_Get(SETID_AUXIN_PLAYING);
            static uint32 timer_print= 0;
            timer_print++;
            if( timer_print % (AUDIO_DRV_PRINT_TIMEOUT_IN_MS/AUDIO_AMP_TEMP_CHECK_TIME) == 0)
            {
#ifdef AUDIO_DRV_TEMP_DEBUG
                dbgTempIncr+= dbgDiff;
#endif
                
                /* When DSP is unstable, DSP version read might be 0.000xxx */
                bool isDspErr= FALSE;
                isDspErr= *(bool*)Setting_GetEx(SETID_IS_DSP_ERROR, &isDspErr);
                char *errMsg= (isDspErr)?(" DSP-VER-ERROR!!"):("");

                float tempWf= 100.0 * DSPDrv1761_ReadValue5_23(&DspADAU1761Drv, DSP_READBACK_TEMP_WF_REGISTER);
                float tempTw= 100.0 * DSPDrv1761_ReadValue5_23(&DspADAU1761Drv, DSP_READBACK_TEMP_TW_REGISTER);
                float gainWf= 100.0 * DSPDrv1761_ReadValue5_23(&DspADAU1761Drv, DSP_READBACK_GAIN_WF_REGISTER);
                float gainTw= 100.0 * DSPDrv1761_ReadValue5_23(&DspADAU1761Drv, DSP_READBACK_GAIN_TW_REGISTER);
                float gainHw= 100.0 * DSPDrv1761_ReadValue5_23(&DspADAU1761Drv, DSP_READBACK_GAIN_HW_REGISTER);
                float noiseGate= DSPDrv1761_ReadValue5_23(&DspADAU1761Drv, DSP_READBACK_NOISE_GATE_REGISTER);

                TP_PRINTF("AmpTemp=%dC, DspTemp=(wf:%.1f, tw:%.1f), DspGain=(wf:%.0f%%, tw:%.0f%%, hw:%.0f%%), AuxIn=%.1fdB[Det=%d], NG=%.2f%s\r\n", 
                              tempAmpNew, tempWf, tempTw, gainWf, gainTw, gainHw, auxin_input_db_smooth, hasMusicStream, noiseGate, errMsg);
            }
        }
    }
}


static void AudioDrv_DspVolSetId_Handler(void)
{
    uint8 volumeLevel = MIN_VOLUME;

    volumeLevel =  *(uint8*)Setting_Get(ctrIdEQIdMap[DSP_VOLUME_SETT_ID].settingId);
    if(volumeLevel <= MAX_VOLUME && volumeLevel >= MIN_VOLUME)
    {
          DSPDrv1761_SetAudio(&DspADAU1761Drv, DSP_VOLUME_SETT_ID, TRUE);
    }
    else
    {
        ASSERT(0);
    }
}



void AudioDrv_CaptureAnalogData()
{
}
