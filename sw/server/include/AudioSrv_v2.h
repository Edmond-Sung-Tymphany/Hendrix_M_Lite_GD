/**
 * @file        AudioSrv.h
 * @brief       Audio Control Server
 * @author      Wesley Lee
 * @date        2014-02-17
 * @copyright   Tymphany Ltd.
 */


#ifndef AUDIOSRV_V2_H
#define AUDIOSRV_V2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "attachedDevices.h"
  
#define EQ_DATA_SIZE     (SIZE_OF_LARGE_EVENTS - 12)
#ifdef HAS_GRAPHICAL_EQ
#define GRAPHICAL_EQ_BANDS_NUM  (5)
#endif //HAS_GRAPHICAL_EQ


/********************************New Events******************************/
REQ_EVT(AudioSettEvt)
    eAudioSettId  aduioSettId;
    BOOL          enable;
    uint32        param;
    uint32        param2;
END_REQ_EVT(AudioSettEvt)

REQ_EVT(AudioMuteReqEvt)
    bool            mute;
    eAudioMuteType  audioMuteType;
END_REQ_EVT(AudioMuteReqEvt)

#ifdef HAS_AUDIO_MUTE_CHANNEL
REQ_EVT(AudioMuteChannelReqEvt)
    uint32          channel_mask;
END_REQ_EVT(AudioMuteChannelReqEvt)
#endif

/**********************************************************************/
IND_EVT(AudioJackinStateEvt)
    eAudioJackId jackId;
    bool IsJackIn;
END_IND_EVT(AudioJackinStateEvt)

/**
 * Note: There are two scenarios. 1. Check a specific line to see if there is music
 * stream on this line.
 * 2. Check the system if there is music streaming without knowing where does this
 * music stream coming from.
 * For the first one, The event AudioMusicDetectStateEvt will be perfetctly match
 * the requirment. However, for the second scenario, the element jackId will make
 * no sense, if your project requirement match the second scenario, you can simply ignore
 * the field jackId or assign it with a value of JACK_IN_INVALID.
 */
IND_EVT(AudioMusicStreamStateEvt)
    eAudioJackId jackId;
    bool hasMusicStream;
END_IND_EVT(AudioMusicDetectStateEvt)

REQ_EVT(AudioChannelSwitchReqEvt)
    eAudioChannel channel;
END_REQ_EVT(AudioChannelSwitchReqEvt)

RESP_EVT(AudioChannelSwitchRespEvt)
    eAudioChannel channel;
END_RESP_EVT(AudioChannelSwitchRespEvt)

SUBCLASS(cAudioSrv, cServer)
    /* private data */
    QTimeEvt scanJackEvt;
    int32 JackScanTimer;
    int32 musicDetectTimer;
    int32 changeChannelTimer;
    eAudioChannel channel;
METHODS
/* public functions */
/* Implement these so the controller can launch the server */
void AudioSrv_StartUp(cPersistantObj *me);
void AudioSrv_ShutDown(cPersistantObj *me);

void AudioSrv_SetChannel(QActive * sender, eAudioChannel channel);
void AudioSrv_SendMuteReq(QActive* sender, eAudioMuteType muteType, const bool mute);

void AudioSrv_SetEq(eAudioSettId audioSettId, BOOL enable);
void AudioSrv_SetVolume(int8 volLevel);
void AudioSrv_SetDspCalGain(eAudioSettId dspCalGainId, float fGainDb);
void AudioSrv_SystemTuning(eAudioSettId sysTuneId, BOOL enable);
void AudioSrv_SetAudio(eAudioSettId audioSettId, BOOL enable, uint32 param, uint32 param2);

END_CLASS
#ifdef __cplusplus
}
#endif

#endif  /* AUDIOSRV_V2_H */

