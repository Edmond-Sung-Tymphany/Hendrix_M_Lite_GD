/**
 * @file        AudioSrv.h
 * @brief       Audio Control Server
 * @author      Wesley Lee
 * @date        2014-02-17
 * @copyright   Tymphany Ltd.
 */


#ifndef AUDIOSRV_V1_H
#define AUDIOSRV_V1_H

#ifdef __cplusplus
extern "C" {
#endif
#include "DspDrv.h"
#include "server.h"
#ifdef HAS_GPIO_JACK_IN_DETECT
#include "setting_id.h"
#endif
#ifdef HAS_BT_TONE
#include "BluetoothSrv.h"   //for eBtCmd btTone;
#endif

#define EQ_DATA_SIZE     (SIZE_OF_LARGE_EVENTS - 12)
#ifdef HAS_GRAPHICAL_EQ
#define GRAPHICAL_EQ_BANDS_NUM  (5)
#endif //HAS_GRAPHICAL_EQ

#ifdef HAS_BT_TONE
/* config the channel for BT and aux in, will be moved to audio config later*/
#define BT_CHANNEL      AUDIO_CHANNEL_2
#define AUXIN_CHANNEL   AUDIO_CHANNEL_1
typedef enum
{
    CHANGE_CHANNEL_STEP,
    SEND_TONE_CMD_STEP,
    CHANGE_VOL_STEP,
    PLAY_TONE_STEP,
    MUTE_VOL_STEP,
    RESTORE_CHANNEL_STEP,
    RESTORE_VOL_STEP,
    MAX_STEP,
}eBtToneStep;
typedef struct
{
    eBtCmd toneId;  // the tone that requested
    eBtToneStep step; // step to play the tone
    int16 calTimer;
    bool isExcuting; //flag that if the tone is excuting
}tReqTone;
#endif

#ifdef HAS_GPIO_JACK_IN_DETECT
typedef enum 
{
    AUXIN_JACK,
    SPDIF_IN_JACK,
    SPDIF1_IN_JACK,
    RJ45_IN_JACK,
    RCA_IN_JACK,
    JACK_IN_INVALID=0xff
}eAudioJackId;

typedef struct
{
    eAudioJackId    audioJackInSourceId;
    bool            (*isJackInFunc)(void);
}tIsJackIn;

typedef struct
{
    eSettingId  settingId;
    bool        (*isMusicDetectedFunc)(void);
}tMusicDetected;


#endif

typedef enum
{
    AUDIO_CHANNEL_INVALID,
    AUDIO_CHANNEL_0,
    AUDIO_CHANNEL_1,
    AUDIO_CHANNEL_2,
    AUDIO_CHANNEL_SPDIF_0,
    AUDIO_CHANNEL_SPDIF_1,
    AUDIO_COMMON_MAX_CHANNEL,
#ifdef HAS_BT_TONE
/* switch to this channel to play BT tone */
    BT_TONE_CHANNEL,
#endif
    AUDIO_CHANNEL_RCA,
    AUDIO_CHANNEL_I2S_2,
    AUDIO_CHANNEL_I2S_3,
#ifdef HAS_MULTIROOM_FEATURE
    AUDIO_CHANNEL_AUXIN_DIRECT,
    AUDIO_CHANNEL_AUXIN_MULTIROOM,
    AUDIO_CHANNEL_RCA_IN_DIRECT,
    AUDIO_CHANNEL_RCA_IN_MULTIROOM,
    AUDIO_CHANNEL_OPT_IN_DIRECT,
    AUDIO_CHANNEL_OPT_IN_MULTIROOM,
#endif
    END_AUDIO_CHANNEL
}eAudioChannel;

#ifdef AUDIO_LIMITER_FOR_LOW_POWER
typedef enum
{
    AUDIO_ADAPTER_DRC_LEVEL,
    AUDIO_NORMAL_BATTERY_DRC_LEVEL,
    AUDIO_LOW_POWER_DRC_LEVEL,
    AUDIO_MAX_DRC_LEVEL,
}eAudioPowerDrcLevel;
#endif

#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
typedef enum
{
    AUDIO_ADAPTER_GAIN_LEVEL,
    AUDIO_FULL_BATTERY_GAIN_LEVEL,
    AUDIO_HIGH_BATTERY_GAIN_LEVEL,
    AUDIO_LOW_BATTERY_GAIN_LEVEL,
    AUDIO_INVALID_BATTERY_GAIN_LEVEL,
}eAudioPowerGainLevel;
#endif

/********************************New Events******************************/
REQ_EVT(AudioSettEvt)
    eDspSettId    dspSettId;
    BOOL          enable;
END_REQ_EVT(AudioSettEvt)

REQ_EVT(AudioMuteReqEvt)
    bool mute;
END_REQ_EVT(AudioMuteReqEvt)

#ifdef AUDIO_LIMITER_FOR_LOW_POWER
//use with AUDIO_SET_POWER_LEVEL_DRC_SIG
REQ_EVT(AudioSetPowerLevelDrcEvt)
    eAudioPowerDrcLevel powerLevelDrc;
END_REQ_EVT(AudioSetPowerLevelDrcEvt)
#endif

#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
//use with AUDIO_SET_POWER_LEVEL_DRC_SIG
REQ_EVT(AudioSetPowerLevelGainEvt)
    eAudioPowerGainLevel powerLevelGain;
END_REQ_EVT(AudioSetPowerLevelGainEvt)
#endif

/**********************************************************************/
IND_EVT(AudioStateEvt)
#ifdef HAS_GPIO_JACK_IN_DETECT
    eAudioJackId jackId;
#endif
    bool IsJackIn;
END_IND_EVT(AudioStateEvt)

IND_EVT(AudioMusicDetectStateEvt)
#ifdef HAS_GPIO_JACK_IN_DETECT
    eAudioJackId jackId;
#endif
    bool hasMusicStream;
    bool IsPCM;
END_IND_EVT(AudioMusicDetectStateEvt)

#ifdef AUDIO_MULTI_SOURCE
REQ_EVT(AudioChannelSwitchReqEvt)
    eAudioChannel channel;
#ifdef HAS_BT_TONE
/* it's only needed when switched to BT tone channel, to get the tone delay*/
    eBtCmd btTone;
#endif
END_REQ_EVT(AudioChannelSwitchReqEvt)

RESP_EVT(AudioChannelSwitchRespEvt)
    eAudioChannel channel;
#ifdef HAS_BT_TONE
    eBtCmd btTone;
#endif
END_RESP_EVT(AudioChannelSwitchRespEvt)
#endif //AUDIO_MULTI_SOURCE

SUBCLASS(cAudioSrv, cServer)
    /* private data */
    QTimeEvt scanJackEvt;
    int32 JackScanTimer;
    int32 musicDetectTimer;
    int32 changeChannelTimer;
    eAudioChannel channel;
#ifdef HAS_BT_TONE
    tReqTone reqTone;
#endif
#ifdef HAS_GPIO_JACK_IN_DETECT
    tIsJackIn* audioJackIn;
#endif

#ifdef MUTIPLE_SOURCE_MUSIC_DETECTION
    tMusicDetected* audioMusicDet;
#endif

METHODS
    /* public functions */
END_CLASS



/* declare types */
typedef enum
{
    AUDIOCTRL_INPUTTYPE_UNINITED,
    AUDIOCTRL_INPUTTYPE_LINEIN,
    AUDIOCTRL_INPUTTYPE_BLUETOOTH,
    AUDIOCTRL_INPUTTYPE_APPLEDOCK,
    AUDIOCTRL_INPUTTYPE_ALLPLAY
}AudioCtrl_inputType_t;

typedef enum
{
    AUDIOCTRL_MUTESETTINGS_MUTE,
    AUDIOCTRL_MUTESETTINGS_UNMUTE
}AudioCtrl_muteSetting_t;

typedef enum
{
    AUDIOCTRL_AUDIOCUE_POWERUP,
    AUDIOCTRL_AUDIOCUE_POWERDOWN,
    AUDIOCTRL_AUDIOCUE_CONNECT,
    AUDIOCTRL_AUDIOCUE_DISCONNECT
}AudioCtrl_audioCue_t;

typedef enum
{
    AUDIOCTRL_VOLUME_CONTINOUS_NO_CHANGE,
    AUDIOCTRL_VOLUME_CONTINOUS_UP,
    AUDIOCTRL_VOLUME_CONTINOUS_DOWN
}AudioCtrl_VolContinousChangeSetting_t;

typedef enum
{
    AUDIOCTRL_EVENT_VOLUME_REACHED_MAX,
    AUDIOCTRL_EVENT_VOLUME_REACHED_MIN
}AudioCtrl_event_t;

typedef enum
{
    AUDIOCTRL_DRC_INVALID,
    AUDIOCTRL_DRC_AC,
    AUDIOCTRL_DRC_BATTERY_HIGH,
    AUDIOCTRL_DRC_BATTERY_MIDDLE,
    AUDIOCTRL_DRC_BATTERY_LOW
}AudioCtrl_DRC_Setting_t;

typedef enum
{
    AUDIOCTRL_MUTECHANNEL_COMMON,       // mute needed by boot flow, system logic
    AUDIOCTRL_MUTECHANNEL_BT,           // mute needed from BT, like when PBB3 is pairing
    AUDIOCTRL_MUTECHANNEL_USER,         // mute issued by user, eg. from IR remote ctrl
    AUDIOCTRL_MUTECHANNEL_AUDIOCUE,     // mute channel used before and after audio cue, also, before and after audio input change
    AUDIOCTRL_MUTECHANNEL_BTSTATUS,     // mute channel to mute BT when BT_STATUS is L, unmute when H
    AUDIOCTRL_MUTECHANNEL_AUDIOMUTE,    //mute channel to mute the audio when the audio output vol. is 0
    AUDIOCTRL_MUTECHANNEL_NOINPUT,      //mute the speaker when there is no audio input(auxin/BT) at all
    AUDIOCTRL_MUTECHANNEL_MAX
}AudioCtrl_MuteChannel_t;


typedef struct{
    AudioCtrl_inputType_t                   currentInput:8;
    int8                                   currentVolume:8;

    struct
    {
        AudioCtrl_VolContinousChangeSetting_t   continousChangeSetting:8;
        uint16                                  timeInterval;
        uint32                                  startTime;
        uint32                                  expireTime;
    }VolContinuousChange;

    struct
    {
        AudioCtrl_DRC_Setting_t                 currentDRC;
        uint32                                  lastDRC_setTime;
        bool                                    bDRC_lock;
        uint32                                  timeFirstAboveUpperThres;
        bool                                    bBeingAboveUpperThres;
    }DRC;

}tAudioCtrl;


/* Implement these so the controller can launch the server */
void AudioSrv_StartUp(cPersistantObj *me);
void AudioSrv_ShutDown(cPersistantObj *me);

void AudioSrv_SendSwitchChannelReq(QActive * sender, eAudioChannel channel);
void AudioSrv_SendMuteReq(QActive* sender, const bool mute);

void AudioSrv_Set(eDspSettId dspSettId, BOOL enable);
void AudioSrv_SetVolume(uint8 volLevel);
#ifdef HAS_BASS
void AudioSrv_SetBass(int8 bassLevel);
#endif
#ifdef HAS_TREBLE
void AudioSrv_SetTreble(int8 trebleLevel);
#endif
#ifdef HAS_BT_TONE
void AudioSrv_SendAudioSrvToneCmd(QActive* sender, eBtCmd cmd);
#endif

#ifdef AUDIO_LIMITER_FOR_LOW_POWER
void AudioSrc_SetDrcRange(QActive* sender, eAudioPowerDrcLevel drcLevel);
#endif

bool AudioSrv_IsDigitalPCM();
void AudioSrv_muteDAC(bool mute);


#ifdef __cplusplus
}
#endif

#endif  /* AUDIOSRV_V1_H */

