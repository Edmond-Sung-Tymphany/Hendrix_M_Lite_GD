/**
 * @file        AsetkSrv_priv.h
 * @brief       it's the server to control the ASE-TK
 * @author      Johnny Fan
 * @date        2014-05-11
 * @copyright   Tymphany Ltd.
 */

#ifndef ASETK_SERVER_PRIVATE_H
#define ASETK_SERVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "beo_ase_fep.pb.h"
#include "AseTkSrv.h"
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
QState AseTkSrv_Initial(cAseTkSrv * const me, QEvt const * const e);
QState AseTkSrv_PreActive(cAseTkSrv * const me, QEvt const * const e);
QState AseTkSrv_Active(cAseTkSrv * const me, QEvt const * const e);
QState AseTkSrv_PreDeActive(cAseTkSrv * const me, QEvt const * const e);
QState AseTkSrv_DeActive(cAseTkSrv * const me, QEvt const * const e);

static void AseTkSrv_HandleAseCommand(cAseTkSrv * const me, AseFepMessage message);
static void AseTkSrv_TimeOutHandler(uint8* data, uint8 size);
static void AseTkSrv_SendMessage(FepAseMessage* pMessage);
static void AseTkSrv_OnReceive(void* p);
static void AseTkSrv_EnableComWdg(cAseTkSrv * const me);
static void AseTkSrv_DisableComWdg(cAseTkSrv * const me);
static void AseTkSrv_FeedComWdg(cAseTkSrv * const me);
static void AseTkSrv_CheckComWdgTimer(cAseTkSrv * const me);
static void AseTkSrv_SendAliveReq(void);

static bool data_to_message(AseFepMessage* p_message, char* buff, uint32 size);
static uint32 message_to_data(FepAseMessage* p_message, char* buff, uint32 size_of_buff);
static int yahdlc_seq_advance(int seq);

static void AseTkSrv_updateStblStatus(cAseTkSrv * const me, uint32 stbl_status);

static cNvmDrv AseTkSrv_storageDrv;
static void AseTkSrv_moduleSysDis(void *me);
static void AseTkSrv_moduleResetNDis(void *me);
static void AseTkSrv_DeInitUart(void *me);
static void AseTkSrv_moduleSysEn(void *me);
static void AseTkSrv_moduleResetNEn(void *me);
static void AseTkSrv_InitUart(void *me);

#ifdef __cplusplus
}
#endif

#endif /* ASETK_SERVER_PRIVATE_H */
