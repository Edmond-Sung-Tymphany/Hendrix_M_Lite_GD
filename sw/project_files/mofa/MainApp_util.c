/**
*  @file      MainApp_KeyEvtHandler.c
*  @brief     Key event handler of MOFA
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "controller.h"
#include "AllPlaySrv.h"
#include "LedSrv.h"
#include "MainApp_priv.h"

/*****************************************************************
 * Definition
 *****************************************************************/

#define AUX_IN_STATUS_BITMASK       AUXIN_JACK
#define RCA_IN_STATUS_BITMASK       RCA_IN_JACK
#define OPT_IN_STATUS_BITMASK       SPDIF_IN_JACK
/*****************************************************************
 * Function Implemenation
 *****************************************************************/

void MainApp_SendLedReq(cMainApp* me, ePattern patternId)
{
    LedSrv_SetPatt((QActive*)me, ALL_LED, patternId);
}

void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}

void MainApp_SetExternalSource(cMainApp * const me)
{
    char *pSourceName = me->sourceHandler[me->audioSource].pSourceName;
    tExternalSourceInfo externalSourceInfo = {
        .name[0] = "AllPlay",
        .interruptible = TRUE,
        .volumeCtrlEnabled = TRUE
    };
    sprintf(externalSourceInfo.name, pSourceName);
    AllPlaySrv_SetExternalSource(externalSourceInfo);
}

void MainApp_ResumeVol(cMainApp * const me)
{
    // Resume the previous volume
    uint8 vol = DEFAULT_VOLUME;
    if(Setting_IsReady(SETID_VOLUME))
    {
        /* Resume the previous volume only if previous volume is lower than default,
        * else use default level if previous volume is higher than default).
        */
        vol = *(uint8*)Setting_Get(SETID_VOLUME);
        if (vol > DEFAULT_VOLUME)
        {
            vol = DEFAULT_VOLUME;
        }
    }
    AudioSrv_SetVolume(vol);
    /* update allplay volume for the mobile app. vol. bar display */
    uint8 allplayVol = MainApp_mcuVol2AllplayVol(vol);
    AllPlaySrv_SetVol(allplayVol);
}

eAudioSource MainApp_GetNextAvialableSource(cMainApp * const me)
{
    eAudioSource audioSrc = me->audioSource;
    /* update jack in status */
    bool jackInStatus = *(bool*)Setting_Get(SETID_IS_AUXIN_PLUG_IN); //use one byte to store jack in status in setting DB.
    me->sourceHandler[AUXIN].bIsValid = (jackInStatus>>AUX_IN_STATUS_BITMASK) & 0x01;
    me->sourceHandler[RCA_IN].bIsValid = (jackInStatus>>RCA_IN_STATUS_BITMASK) & 0x01;
    me->sourceHandler[OPT_IN].bIsValid = (jackInStatus>>OPT_IN_STATUS_BITMASK) & 0x01;

    int i = 0;
    for(i = 0; i < MAX_SOURCE; i++)
    {
        if(TRUE == me->sourceHandler[audioSrc].bIsValid)
        {
            break;
        }

        /* goto check next audio source*/
        audioSrc++;
        if(audioSrc >= MAX_SOURCE)
        {
            audioSrc = 0;
        }
    }
    return audioSrc;
}
