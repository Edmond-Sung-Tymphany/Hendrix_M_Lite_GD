/**
*  @file      BnO_MainApp_util.c
*  @brief     util function for BnO mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "trace.h"
#include "controller.h"
#include "Setting_id.h"
#include "AseTkSrv.h"
#include "LedSrv.h"
#include "SettingSrv.h"
#include "BnO_MainApp.h"

/*****************************************************************
 * Definition
 *****************************************************************/
#define AUX_IN_STATUS_BITMASK       AUXIN_JACK


/*****************************************************************
 * Function Implemenation
 *****************************************************************/

/* Note: The Api: BnO_MainApp_VolumeDown() and BnO_MainApp_VolumeUp() is just for HW bring-up in ES stage,
 * they can be removed if not need in later stages.
 */
void BnO_MainApp_VolumeDown(cMainApp * const me, uint8 step)
{
#ifndef DEBUG_QUICK_BOOT_NO_AUDIO
    // FIX-ME: avoid un-init setting server
    if (!Setting_IsReady(SETID_VOLUME))
        return;

    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
    if(vol >= step)
    {
        vol-=step;
    }
    else
    {
        vol = MIN_VOLUME;
    }
    AudioSrv_SetVolume(vol); //pass to DSPDrv1451_SetVol()
    TP_PRINTF("[%d] v = %d\r\n", __LINE__, vol);
#endif    
}

void BnO_MainApp_VolumeUp(cMainApp * const me, uint8 step)
{
#ifndef DEBUG_QUICK_BOOT_NO_AUDIO
    // FIX-ME: avoid un-init setting server
    if (!Setting_IsReady(SETID_VOLUME))
        return;

    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
    if((step + vol)<= MAX_VOLUME)
    {
        vol+=step;
    }
    else
    {
        vol = MAX_VOLUME;
    }
    AudioSrv_SetVolume(vol); //pass to DSPDrv1451_SetVol()
    TP_PRINTF("[%d] v = %d\r\n", __LINE__, vol);
#endif
}

void BnO_MainApp_SendLedReq(cMainApp* me, eLedIndID ledIndID)
{
    ASSERT(ledIndID < LED_IND_ID_MAX);
    //TP_PRINTF("BnO_MainApp_SendLedReq: ledIndID=%d\r\n", ledIndID);
#ifdef HAS_LEDS    
    LedSrv_SetPatt((QActive*)me, me->ledInds[ledIndID].leds, me->ledInds[ledIndID].patternId);
#endif    
}

void BnO_MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}

void BnO_MainApp_ResumeVol(cMainApp * const me)
{
#ifndef DEBUG_QUICK_BOOT_NO_AUDIO
    // Resume default volume
    if(!Setting_IsReady(SETID_VOLUME))
    {
        AudioSrv_SetVolume(DEFAULT_VOLUME);
    }
#endif    
}

eAudioSource BnO_MainApp_GetNextAvialableSource(cMainApp * const me)
{
    eAudioSource audioSrc = me->audioSource;
#ifdef HAS_AUXIN
    /* update jack in status */
    bool jackInStatus = *(bool*)Setting_Get(SETID_IS_AUXIN_PLUG_IN); //use one byte to store jack in status in setting DB.
    me->sourceHandler[AUDIO_SOURCE_AUXIN].bIsValid = (jackInStatus>>AUX_IN_STATUS_BITMASK) & 0x01;
#endif
    uint8 i = 0;
    for(i = 0; i < AUDIO_SOURCE_MAX; i++)
    {
        if(TRUE == me->sourceHandler[audioSrc].bIsValid)
        {
            break;
        }

        /* goto check next audio source*/
        audioSrc++;
        if(audioSrc >= AUDIO_SOURCE_MAX)
        {
            audioSrc = 0;
        }
    }
    return audioSrc;
}
