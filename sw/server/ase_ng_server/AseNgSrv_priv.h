/**
 * @file        AseNgSrv_priv.h
 * @brief       it's the server to control the ASE-NG
 * @author      Dmitry Abdulov
 * @date        2016-12-15
 * @copyright   Tymphany Ltd.
 */

#ifndef ASENG_SERVER_PRIVATE_H
#define ASENG_SERVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"
#include "pb_encode.h"
#include "pb_decode.h"
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


#include "AseNgSrv.h"
#include "deviceTypes.h"
#include "DebugSSrv.h"

#ifdef HAS_NVM
#include "NvmDrv.h"
#endif

#define ASETK_TUNNEL_BUFFER_SIZE (SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE)
#define ASETK_NETWORK_INFO_BUFFER_SIZE 4

typedef enum
{
    ASETK_TUNNEL_COMMAND_ECHO_REQ              = 0,
    ASETK_TUNNEL_COMMAND_ECHO_RESP             = 1,
    ASETK_TUNNEL_COMMAND_BI_REQ                = 2,
    ASETK_TUNNEL_COMMAND_BI_RESP               = 3,
    ASETK_TUNNEL_COMMAND_SETT_REQ              = 4,
    ASETK_TUNNEL_COMMAND_SETT_RESP             = 5,
    ASETK_TUNNEL_COMMAND_MUTE_REQ              = 6,
    ASETK_TUNNEL_COMMAND_MUTE_RESP             = 7,
    ASETK_TUNNEL_COMMAND_TP_MONITOR_START_REQ  = 8,
    ASETK_TUNNEL_COMMAND_TP_MONITOR_START_RESP = 9,
    ASETK_TUNNEL_COMMAND_TP_MONITOR_REQ        = 10,
    ASETK_TUNNEL_COMMAND_TP_MONITOR_RESP       = 11,
    ASETK_TUNNEL_COMMAND_SETT_W_REQ            = 12,
    ASETK_TUNNEL_COMMAND_SETT_W_RESP           = 13,

    // Add new standard commands here:

    ASETK_TUNNEL_COMMAND_STANDARD_MAX          = 100, // Above this ID commands are project specific   
}eAseTkTunnelCommand;

typedef enum
{
  ASETK_TUNNEL_MO_MESSAGE_ID = 0,
  ASETK_TUNNEL_MO_SIZE       = 1,
  ASETK_TUNNEL_MO_DATA       = 2,

  ASETK_TUNNEL_MO_MUTE_TYPE  = 1,
  ASETK_TUNNEL_MO_MUTE_SETT  = 2,
  ASETK_TUNNEL_MO_MUTE_RET   = 3,
}eAseTkTunnelMessageOffset;

typedef struct sOutPutTunnelMessage
{
    uint8 OutputTunnelBuffer[ASETK_TUNNEL_BUFFER_SIZE];
    size_t MessageSize;
} sOutPutTunnelMessage;

  /* State function definitions */
QState AseNgSrv_Initial(cAseNgSrv * const me, QEvt const * const e);
QState AseNgSrv_PreActive(cAseNgSrv * const me, QEvt const * const e);
QState AseNgSrv_Active(cAseNgSrv * const me, QEvt const * const e);
QState AseNgSrv_PreDeActive(cAseNgSrv * const me, QEvt const * const e);
QState AseNgSrv_DeActive(cAseNgSrv * const me, QEvt const * const e);

static void AseNgSrv_HandleAseCommand(cAseNgSrv * const me, Proto_Core_AseFepMessage message);
static void AseNgSrv_TimeOutHandler(uint8* data, uint8 size);
static void AseNgSrv_SendMessage(Proto_Core_FepAseMessage* pMessage);
static void AseNgSrv_OnReceive(void* p);
static void AseNgSrv_EnableComWdg(cAseNgSrv * const me);
static void AseNgSrv_DisableComWdg(cAseNgSrv * const me);
static void AseNgSrv_FeedComWdg(cAseNgSrv * const me);
static void AseNgSrv_CheckComWdgTimer(cAseNgSrv * const me);
static void AseNgSrv_SendAliveReq(void);

static bool data_to_message(Proto_Core_AseFepMessage* p_message, char* buff, uint32 size);

static uint32 message_to_data(Proto_Core_FepAseMessage* p_message, char* buff, uint32 size_of_buff);
static int yahdlc_seq_advance(int seq);

static void AseNgSrv_updateStblStatus(cAseNgSrv * const me, uint32 stbl_status);

static cNvmDrv AseNgSrv_storageDrv;
static void AseNgSrv_moduleSysDis(void *me);
static void AseNgSrv_moduleResetNDis(void *me);
static void AseNgSrv_DeInitUart(void *me);
static void AseNgSrv_moduleSysEn(void *me);
static void AseNgSrv_moduleResetNEn(void *me);
static void AseNgSrv_InitUart(void *me);

#ifdef __cplusplus
}
#endif

#endif /* ASENG_SERVER_PRIVATE_H */
