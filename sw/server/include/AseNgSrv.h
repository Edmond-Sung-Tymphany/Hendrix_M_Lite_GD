/**
 * @file        AseNgSrv.h
 * @brief       it's the server to control the ase ng module
 * @author      Dmitry Abdulov
 * @date        2016-12-15
 * @copyright   Tymphany Ltd.
 */


#ifndef ASE_NG_SERVER_H
#define ASE_NG_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wpl.pb.h"
#include "ase-fep.pb.h"
#include "ase-fep-ReqResp.pb.h"
#include "common.pb.h"
#include "core.pb.h"
#include "dsp.pb.h"
#include "eeb.pb.h"
#include "fep-ase.pb.h"
#include "fep-ase-ReqResp.pb.h"
#include "firmware-update.pb.h"
#include "hdmi.pb.h"
#include "power-link.pb.h"
#include "production.pb.h"
#include "system.pb.h"
#include "StorageDrv.h"
#include "GpioDrv.h"
#include "seq.h"

/* ASE_TK cmd event, with ASE_TK_REQ_SIG*/
REQ_EVT(AseNgCmdEvt)
    Proto_Core_FepAseMessage fepAseCmd;
END_REQ_EVT(AseNgCmdEvt)


REQ_EVT(AseFepCmdEvt)
    Proto_Core_AseFepMessage aseFepCmd;
END_REQ_EVT(AseFepCmdEvt)

IND_EVT(AseNgStateIndEvt)
    Proto_Core_AseFepMessage aseFepCmd;
    union{
        int vol;         //!<read this for a AseFepMessage_MsgId_AseFepVolumeChangedEvent event
        int soundMode;  //!<read this for a AseFepMessage_MsgId_AseFepSetPositionSoundModeReq event
    }payload;
    bool bIsComWdgTimeOut; /* Flag indicates the status of communication watchdog. */
END_IND_EVT(AseNgStateIndEvt)

SUBCLASS(cAseNgSrv, cServer)
    /* private data */
    cGpioDrv        gpio;
    cSeq            seq;
    cStorageDrv     *pStorageDrv;
    QActive         *pSender;
    int32           comWdgTimer; /* Communication Watchdog Timer */
    int8            aliveReqInProg; /* the number of alive request in progress */
METHODS
    /* public functions */
END_CLASS


typedef struct {
    Proto_System_NetworkInfo_NetworkInterface_State ethernet_state;
    Proto_System_NetworkInfo_NetworkInterface_State wifi_state;
    Proto_System_NetworkInfo_NetworkInterface_State soft_ap_state;
    bool wifi_configured;
    Proto_System_NetworkInfo_NetworkInterface_WiFi_Quality wifi_quality;
}AseNgNetworkInfo;

/* Implement these so the controller can launch the server */
void AseNgSrv_StartUp(cPersistantObj *me);
void AseNgSrv_ShutDown(cPersistantObj *me);

#ifdef GTEST
  /* State function definitions */
QState AseNgSrv_Initial(cAseNgSrv * const me, QEvt const * const e);
QState AseNgSrv_PreActive(cAseNgSrv * const me, QEvt const * const e);
QState AseNgSrv_Active(cAseNgSrv * const me, QEvt const * const e);
QState AseNgSrv_DeActive(cAseNgSrv * const me, QEvt const * const e);
#endif

void AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp msg);
void AseNgSrv_SendFepAseCmdRelativeVol(int32_t relativeVolume);
void AseNgSrv_SendFepAseCmdAbsoluteVol(int32_t absoluteVolume);
void AseNgSrv_SendFepAseEvent(Proto_FepAse_Event_Type event);
void AseNgSrv_SendLog(const char * str, const char * file, uint32 line);
void AseNgSrv_SendServiceLog(const char * str, const char * file, uint32 line);
void AseNgSrv_ReplyVolumeFade(uint32 req_id, bool result);
void AseNgSrv_GenericResponse(bool success, uint32 req_id, Proto_AseFep_ReqResp type);
void AseNgSrv_ReplyAudioPcmFormatCommand(bool success);
void AseNgSrv_SendVersion(uint32 req_id);
void AseNgSrv_SendPowerStatus();
AseNgNetworkInfo* AseNgSrv_GetDecodedNetworkInfo();
void AseNgSrv_QueryInternalSpeakerCompensation();
void AseNgSrv_FillNetworkInfo(Proto_System_NetworkInfo_NetworkInterface * p_net_info);
void AseNgSrv_FillTunnelMsg(uint8 * pbuff, uint8 size);
void AseNgSrv_SendSignalStatus(bool hasSignal);
void AseNgSrv_PlayComfortTone(char *toneName);

#define INT_SPK_TYPE_NUM  (Proto_Dsp_InternalSpeaker_Type_FULLRANGE+1)
#define INT_SPK_POS_NUM   (Proto_Dsp_InternalSpeaker_Position_CENTRE+1)


#ifdef __cplusplus
}
#endif

#endif /* ASE_NG_SERVER_H */

