/**
 * @file        AudioSrv_priv.h
 * @brief       Audio Control Server
 * @author      Wesley Lee
 * @date        2014-02-17
 * @copyright   Tymphany Ltd.
 */

#ifndef AUDIOSRV_PRIV_H
#define AUDIOSRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../server_priv.h"
#include "AudioSrv.h"

/* private state functions */
QState AudioSrv_Initial(cAudioSrv * const me);
QState AudioSrv_Active(cAudioSrv * const me, QEvt const * const e);
QState AudioSrv_AudioMainHandler(cAudioSrv * const me, QEvt const * const e);
QState AudioSrv_Mute(cAudioSrv * const me, QEvt const * const e);
QState AudioSrv_DeActive(cAudioSrv * const me, QEvt const * const e);

#ifdef AUDIO_DSP_AUTO_UPDATE
QState AudioSrv_DSPInit(cAudioSrv * const me, QEvt const * const e);
#endif

#ifdef AUDIO_MULTI_SOURCE
QState AudioSrv_SwitchChannel(cAudioSrv * const me, QEvt const * const e);
#endif
#ifdef HAS_BT_TONE
QState AudioSrv_PlayToneState(cAudioSrv * const me, QEvt const * const e);
#endif

#if (defined MUSIC_DETECTION_WHEN_STANDBY) || (defined JACK_DETECTION_WHEN_STANDBY)
static void AudioSrv_DeactivePeriodicTask(cAudioSrv * const me, uint16 TickTimeInterval);
#endif
#ifdef AUDIO_MULTI_SOURCE
static void AudioSrv_ScanChannelState(cAudioSrv * const me, uint16 TickTimeInterval);
#endif
static void AudioSrv_CheckJackInStatus(cAudioSrv * const me, cDSPDrv* DSPDrv);

/*****************************************************************
 * Private function declaration
 *****************************************************************/
 #ifdef AUDIO_MULTI_SOURCE
static void AudioSrv_CheckAuxinStatus(cAudioSrv * const me, cDSPDrv* DSPDrv);
static void AudioSrv_CheckMusicStream(cAudioSrv * const me, cDSPDrv* DSPDrv);
static void AudioSrv_SetChannel(eAudioChannel audioChannel);
static void AudioSrv_RespSwitchChannelReq(eAudioChannel channel, eEvtReturn result);
#endif
static void AudioSrv_PeriodicTask(cAudioSrv * const me, uint16 TickTimeInterval);

#ifdef HAS_BT_TONE
static void AudioSrv_InitialTone(cAudioSrv * const me, AudioChannelSwitchReqEvt* req);
static bool AudioSrv_IsTimeToPlayTone(cAudioSrv * const me);
static int16 AudioSrv_PlayTone(cAudioSrv * const me);
#endif


#ifdef __cplusplus
}
#endif

#endif /* AUDIOSRV_PRIV_H */

