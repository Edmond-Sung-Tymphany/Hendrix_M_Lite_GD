/**
 * @file        BluetoothSrv.h
 * @brief       it's the server to control the BT module
 * @author      Johnny Fan
 * @date        2014-05-11
 * @copyright   Tymphany Ltd.
 */


#ifndef ASE_TK_SERVER_H
#define ASE_TK_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "beo_ase_fep.pb.h"
#include "StorageDrv.h"
#include "GpioDrv.h"
#include "seq.h"

/* ASE_TK cmd event, with ASE_TK_REQ_SIG*/
REQ_EVT(AseTkCmdEvt)
    FepAseMessage fepAseCmd;
END_REQ_EVT(AseTkCmdEvt)

REQ_EVT(FepCmdEvt)
    AseFepMessage aseFepCmd;
END_REQ_EVT(FepCmdEvt)


IND_EVT(AseTkStateIndEvt)
    AseFepMessage aseFepCmd;
    union{
        int vol;         //!<read this for a AseFepMessage_MsgId_AseFepVolumeChangedEvent event
        int soundMode;  //!<read this for a AseFepMessage_MsgId_AseFepSetPositionSoundModeReq event
    }payload;
    bool bIsComWdgTimeOut; /* Flag indicates the status of communication watchdog. */
END_IND_EVT(AseTkStateIndEvt)

SUBCLASS(cAseTkSrv, cServer)
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
    NetworkInfo_State ethernet_state;
    NetworkInfo_State wifi_state;
    NetworkInfo_State soft_ap_state;
    bool wifi_configured;
    WiFi_Quality wifi_quality;
}AsetkNetworkInfo;


/* Implement these so the controller can launch the server */
void AseTkSrv_StartUp(cPersistantObj *me);
void AseTkSrv_ShutDown(cPersistantObj *me);

#ifdef GTEST
  /* State function definitions */
QState AseTkSrv_Initial(cBluetoothSrv * const me, QEvt const * const e);
QState AseTkSrv_PreActive(cBluetoothSrv * const me, QEvt const * const e);
QState AseTkSrv_Active(cBluetoothSrv * const me, QEvt const * const e);
QState AseTkSrv_DeActive(cBluetoothSrv * const me, QEvt const * const e);
#endif

void AseTkSrv_SendFepAseCmd(FepAseCommand_Command msg);
void AseTkSrv_SendFepAseCmdRelativeVol(int32_t relativeVolume);
void AseTkSrv_SendFepAseCmdAbsoluteVol(int32_t absoluteVolume);
void AseTkSrv_SendFepAseEvent(FepAseEvent_Event event);
void AseTkSrv_SendLog(const char * str, const char * file, uint32 line);
void AseTkSrv_SendServiceLog(const char * str, const char * file, uint32 line);
void AseTkSrv_PlayComfortTone(char* tone_name);
void AseTkSrv_SendProductInfo(const char *prod_name, FepAseProductInfo_ProductId prod_id);
void AseTkSrv_ReplyVolumeFade(uint8 vol);
void AseTkSrv_ReplySetAudioInput(bool success);
void AseTkSrv_ReplyAudioPcmFormatCommand(bool success);
void AseTkSrv_SendVersion();
void AseTkSrv_SendPowerStatus();
AsetkNetworkInfo* AseTkSrv_GetDecodedNetworkInfo();
void AseTkSrv_QueryInternalSpeakerCompensation();

#define INT_SPK_TYPE_NUM  (InternalSpeaker_Type_FULLRANGE+1)
#define INT_SPK_POS_NUM   (InternalSpeaker_Position_CENTRE+1)


#ifdef __cplusplus
}
#endif

#endif /* ASE_TK_SERVER_H */

