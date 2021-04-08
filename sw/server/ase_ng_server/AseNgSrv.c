/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Ase_Ng Server
                  -------------------------
                  SW Module Document




@file        AseNgSrv.c
@brief       it's the server to control the ase_ng module
@author      Dmitry Abdulov
@date        2016-12-15
@copyright (c) Tymphany Ltd. All rights reserved.

-------------------------------------------------------------------------------
*/
#include "stm32f0xx.h"
#include "product.config"
#include "AseNgSrv_priv.h"
#include "AseNgSrv.h"
#include "bsp.h"
#include "controller.h"
#include "SettingSrv.h"
#include "trace.h"
#include "ringbuf.h"
#include "UartDrv.h"
#include "yahdlc.h"
#include "bl_common.h"
#include "fep_addr.h"


#define CAST_ME cAseNgSrv * AseNgSrv = (cAseNgSrv *) me;


/********************************************/
/*************Definition*********************/
/********************************************/
/* To avoid send too many log to ASE-TK, this file print to UART only
 */
#if defined(ASE_TK_DEBUG_ENABLE) && !defined(NDEBUG)
    #define ASENG_TP_PRINTF(...)  ProjBsp_Printf(/*toAse:*/FALSE, __FILE__, __LINE__, __VA_ARGS__)
#else
    #define ASENG_TP_PRINTF(...)
#endif  // NDEBUG



/********************************************/
/***************CONFIG***********************/
/********************************************/
#define ASE_UART_TX_BUF_SIZE     (0x416 * 4)
#define ASE_UART_RX_BUF_SIZE     (0x416)

/* the time (ms) per timeout signal */
#define ASETK_SRV_PER_TICK_TIME_MS  (10)
#define ASETK_SRV_COM_WDG_TIME_MS (10*1000) // 10 seconds,  the value must be mutiple of ASETK_SRV_PER_TICK_TIME_MS

#define ASETK_SRV_MAX_ALIVE_REQ (3)

/* the buffer size used to decode the message, change it according to message size */
#define ASETK_SRV_DECODE_BUFFER_SIZE (300)

#define YAHDLC_SEQ_NO_MAX           (7)

#define ASE_TK_MODULE_GPIO_DELAY    (100)
#define ASE_TK_MODULE_OFF_DELAY     ASETK_SRV_PER_TICK_TIME_MS
#define ASE_NPB_PRE_POWERON_DELAY  (200)
#define ASE_NPB_POWERON_DELAY      (2600)

/********************************************/
/***************variables***********************/
/********************************************/

static uint8        uartTxBuf[ASE_UART_TX_BUF_SIZE];
static uint8        uartRxBuf[ASE_UART_RX_BUF_SIZE];
static cRingBuf     txBuf;
static cRingBuf     rxBuf;
cUartDrv            aseTk_uart;
/* Internal event queue */
static QEvt const *AseTkEvtQue[ASETK_SRV_EVENT_Q_SIZE];
static QEQueue deferredReqQue;
static QEvt const *pDeferredReqQueSto[3];

static Proto_Core_AseFepMessage rMessage2;
static uint32 msgReqId;

static uint8 glInputTunnelBuffer[ASETK_TUNNEL_BUFFER_SIZE];
static uint8 glOutputTunnelBuffer[ASETK_TUNNEL_BUFFER_SIZE];
static bool  glEnabledTpMonitorEmul = FALSE;


static Proto_Dsp_InternalSpeaker glIntSpeaker[INT_SPK_TYPE_NUM][INT_SPK_POS_NUM];
static AseNgNetworkInfo glNetworkInfo;

static void AseNgSrv_AseFepTunnelHandler();

enum InternalSignals
{
    ASETK_TIMEOUT_SIG = MAX_SIG,
};

//#ifdef __ICCARM__
//static const uint32 Fw_version@"FW_VERSION_SECT" =
//    (SW_MAJOR_VERSION << 24) | (SW_MINOR_VERSION1 << 16) |
//    (SW_MINOR_VERSION2 << 8) | (SW_MINOR_VERSION3);
//#endif


//static tSeqSection AseTkModuleInitSection[] =
//{
///*when we switch from OFF ASE-NPB will be low already and
//   to power on module we have to low it for more than 2 sec
//*/
//    {&AseNgSrv_moduleSysEn,     ASE_NPB_PRE_POWERON_DELAY},
//    {&AseNgSrv_moduleSysDis,    ASE_NPB_POWERON_DELAY},
//    {&AseNgSrv_moduleSysEn,     ASE_TK_MODULE_GPIO_DELAY},
//    {&AseNgSrv_moduleResetNDis, ASE_TK_MODULE_GPIO_DELAY},
//    {&AseNgSrv_moduleResetNEn,  0},
//};

static tSeqSection AseTkModuleInitSection[] =
{
/*when we switch from OFF ASE-NPB will be low already and
   to power on module we have to low it for more than 2 sec
*/
    {&AseNgSrv_moduleResetNDis, 50},
    {&AseNgSrv_moduleSysEn,     ASE_NPB_PRE_POWERON_DELAY},
    {&AseNgSrv_moduleSysDis,    ASE_NPB_POWERON_DELAY},
    {&AseNgSrv_moduleSysEn,     50},
    {&AseNgSrv_moduleResetNEn,  0},
};

static tSeqSection AseTkModuleDestroySection[] =
{
    /* ASEII-379: 
     *   When power down, FEP should keep NPB/RST pins high. ASE will power down by itself
     *   - FEP pull RST/NPB low: ASE consume 43mA (previous behavior)
     *   - FEP do not touch NSP/NPB: : ASE consume 0.2mA (now)
     */
    //{&AseNgSrv_moduleSysDis,    ASE_TK_MODULE_OFF_DELAY},
    //{&AseNgSrv_moduleResetNDis, ASE_TK_MODULE_OFF_DELAY},
    {&AseNgSrv_DeInitUart,       0},
};

//void send_ver_resp();

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void AseNgSrv_initCallbackBuf(cAseNgSrv * const me)
{
    memset(glInputTunnelBuffer, 0, sizeof(glInputTunnelBuffer));
    memset(glOutputTunnelBuffer, 0, sizeof(glOutputTunnelBuffer));
    memset(glIntSpeaker, 0, sizeof(glIntSpeaker));
    memset(&glNetworkInfo, 0, sizeof(glNetworkInfo));
    
    /* Init state. They are important on ASE bring up stage becuase we might not successful decocde ASE event
     */
    glNetworkInfo.ethernet_state= Proto_System_NetworkInfo_NetworkInterface_State_CONNECTED;
    glNetworkInfo.soft_ap_state= Proto_System_NetworkInfo_NetworkInterface_State_CONNECTED;
    glNetworkInfo.wifi_state= Proto_System_NetworkInfo_NetworkInterface_State_CONNECTED;
    glNetworkInfo.wifi_quality= Proto_System_NetworkInfo_NetworkInterface_WiFi_Quality_GOOD;
    
    //Set FepAseMsg.req.id
    msgReqId= 0;
}


static void AseNgSrv_updateStblStatus(cAseNgSrv * const me, uint32 stbl_status)
{
    cStorageDrv *pStorageDrv = me->pStorageDrv;
    uint32 stbl_content[] =
    {
        stbl_status,
        *(uint32*)FEP_ADDR_UBL_CHECKSUM,
        *(uint32*)FEP_ADDR_FIRMWARE_CHECKSUM,
    };

    if(pStorageDrv->ErasePage)
    {
        pStorageDrv->ErasePage(pStorageDrv, FEP_ADDR_STBL_STATUS);
    }
    pStorageDrv->SetValue(pStorageDrv, FEP_ADDR_STBL_STATUS,
                                (uint8*)stbl_content, sizeof(stbl_content));
}

static bool AseNgSrv_isModulePowerUp(cAseNgSrv * const me)
{
    // check if the module enable pin and reset_n pin is configured before
    bool set_rstn  = GpioDrv_isBitSet(&me->gpio, GPIO_OUT_ASE_RST_N);
    bool set_sysEn = GpioDrv_isBitSet(&me->gpio, GPIO_OUT_ASE_SYS_EN);

    return (set_rstn && set_sysEn);
}

static void AseNgSrv_moduleSysDis(void *me)
{
    CAST_ME;
    GpioDrv_ClearBit(&AseNgSrv->gpio, GPIO_OUT_ASE_SYS_EN);
}

static void AseNgSrv_moduleResetNDis(void *me)
{
    CAST_ME;
    GpioDrv_ClearBit(&AseNgSrv->gpio, GPIO_OUT_ASE_RST_N);
}

static void AseNgSrv_DeInitUart(void *me)
{
    //Deinit driver
    UartDrv_Xtor(&aseTk_uart);
    RingBuf_Xtor(&txBuf);
    RingBuf_Xtor(&rxBuf);
}

static void AseNgSrv_moduleSysEn(void *me)
{
    cAseNgSrv *aseTkSrv = (cAseNgSrv*) me;
    GpioDrv_SetBit(&aseTkSrv->gpio, GPIO_OUT_ASE_SYS_EN);
}

static void AseNgSrv_moduleResetNEn(void *me)
{
    cAseNgSrv *aseTkSrv = (cAseNgSrv*) me;
    GpioDrv_SetBit(&aseTkSrv->gpio, GPIO_OUT_ASE_RST_N);
}

static void AseNgSrv_InitUart(void *me)
{
    //Init driver
    RingBuf_Ctor(&txBuf, uartTxBuf, ArraySize(uartTxBuf));
    RingBuf_Ctor(&rxBuf, uartRxBuf, ArraySize(uartRxBuf));
    UartDrv_Ctor(&aseTk_uart, (tUARTDevice*) getDevicebyIdAndType(ASETK_DEV_ID, UART_DEV_TYPE, NULL), &txBuf, &rxBuf);
    UartDrv_RegisterRxCallback(&aseTk_uart, AseNgSrv_OnReceive);
}

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void AseNgSrv_StartUp(cPersistantObj *me)
{
    CAST_ME;
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(AseNgSrv);
    QS_OBJ_DICTIONARY(AseNgSrv_PreActive);
    QS_OBJ_DICTIONARY(AseNgSrv_Active);
    QS_OBJ_DICTIONARY(AseNgSrv_DeActive);

    AseNgSrv->pStorageDrv = (cStorageDrv *)&AseNgSrv_storageDrv;
    StorageDrv_Ctor(AseNgSrv->pStorageDrv,
                    (tStorageDevice*)getDevicebyId(INT_FLASH_DEV_ID, NULL));
    //RingBuf_Ctor(&txBuf, uartTxBuf, ArraySize(uartTxBuf));
    //RingBuf_Ctor(&rxBuf, uartRxBuf, ArraySize(uartRxBuf));
    //UartDrv_Ctor(&aseTk_uart, (tUARTDevice*) getDevicebyIdAndType(ASETK_DEV_ID, UART_DEV_TYPE, NULL), &txBuf, &rxBuf);

    /* start up the object and let it run. including the timer*/
    Server_Ctor((cServer*)me, Q_STATE_CAST(&AseNgSrv_Initial), ASETK_TIMEOUT_SIG,
                AseTkEvtQue, Q_DIM(AseTkEvtQue), ASENG_SRV_ID);
    QEQueue_init(&deferredReqQue, pDeferredReqQueSto, Q_DIM(pDeferredReqQueSto));
}

void AseNgSrv_ShutDown(cPersistantObj *me)
{
    /* Clean memory and shut-down. Called by the controller */
    Server_Xtor((cServer*)me);
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */

QState AseNgSrv_Initial(cAseNgSrv * const me, QEvt const * const e)
{
    //bool moduleUp;
    me->gpio.gpioConfig = (tGPIODevice*) getDevicebyIdAndType(ASETK_DEV_ID, GPIO_DEV_TYPE, NULL);

    //moduleUp = AseNgSrv_isModulePowerUp(me);    // get status before initialize
    GpioDrv_Ctor(&me->gpio, me->gpio.gpioConfig);
    AseNgSrv_DisableComWdg(me);
    AseNgSrv_initCallbackBuf(me);

    /*Set default value */
    const bool connected = FALSE;
    Setting_Set(SETID_ASETK_CONNECTED, &connected);
    /*
    * We assumed ASETK module may already been power on in PIU.
    * But ASETK server should stay at deactive state before receive SYSTEM_ACTIVE_REQ_SIG.
    */
    return Q_TRAN(&AseNgSrv_DeActive);
}

/* This state is used to as wait state. Wait until power on */
QState AseNgSrv_PreActive(cAseNgSrv * const me, QEvt const * const e)
{
    switch(e->sig)
    {
        case Q_ENTRY_SIG:
        {
            AseNgSrv_InitUart(me);
            /* First boot: ASE-TK was powered on by PIU
             * Wakeup from OFF: ASE-TK will power up here
             * Forst boot, use ST-Link online debug: because PIU is not appear, ASe-TK wil power up here
             * After upgrade UBL or Firmware: ASE-TK was powered on before upgrade.
             */
            eFepFirmwareStatus fw_status = bl_getFirmwareStatus();

            //Note here is early stage, can not print any message
            if (FEP_FIRMWARE_NEW == fw_status)
            {
                ASENG_TP_PRINTF("ASE-TK has was booted up before upgrade\r\n");
                AseNgSrv_EnableComWdg(me);

                //ASE-TK Init operation
//                AseNgSrv_QueryInternalSpeakerCompensation();
//                AseNgSrv_SendPowerStatus();
            }
            else if (FEP_FIRMWARE_POWERED_ASETK == fw_status)
            {
                ASENG_TP_PRINTF(" ASE-TK has was powered on by PIU. \r\n");
                bl_setFirmwareStatus(FEP_FIRMWARE_NORMAL);  // avoid skipping ASE-TK boot-up for next power cycle.
            }
            else
            {
                ASENG_TP_PRINTF(" Reset and power on ASE!!! \r\n");
                Seq_Ctor(&me->seq, me, AseTkModuleInitSection, ArraySize(AseTkModuleInitSection));
            }

            PersistantObj_RefreshTick((cPersistantObj*)me, ASETK_SRV_PER_TICK_TIME_MS);
            return Q_HANDLED();
        }
        case ASETK_TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, ASETK_SRV_PER_TICK_TIME_MS);
            if (!Seq_isSeqFinished(&me->seq))
            {
                Seq_Refresh(&me->seq, ASETK_SRV_PER_TICK_TIME_MS);
            }
            else
            {
                CommonEvtResp((QActive*)me, me->pSender, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
                return Q_TRAN(&AseNgSrv_Active);
            }
            return Q_HANDLED();
        }
        /* receive request from MainApp */
        case ASE_TK_REQ_SIG:
        {
            QActive_defer((QActive *)me, &deferredReqQue, e);
            return Q_HANDLED();
        }
        case SYSTEM_SLEEP_REQ_SIG:
        {
            QActive_defer((QActive *)me, &deferredReqQue, e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        default:
            /* Not list all the QP signals here, so doesn't put a assert in the default*/
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/*   Active state - first state where "normal" service begins  */
QState AseNgSrv_Active(cAseNgSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            QActive_recall((QActive*)me, &deferredReqQue);
            PersistantObj_RefreshTick((cPersistantObj*)me, ASETK_SRV_PER_TICK_TIME_MS);

            AseNgSrv_initCallbackBuf(me);

            return Q_HANDLED();
        }
        case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*) e;
            me->pSender = pReq->sender;
            return Q_TRAN(AseNgSrv_PreDeActive);
        }
#ifdef HAS_PRODUCTION_TEST_DISABLE_ASE_TK_UART
        case PT_ASE_DISABLE_UART_REQ_SIG:
        {
            return Q_TRAN(AseNgSrv_PreDeActive);
        }
#endif
        /* receive request from MainApp */
        case ASE_TK_REQ_SIG:
        {
            AseNgCmdEvt* evt = (AseNgCmdEvt*) e;
            Proto_Core_FepAseMessage* pMsg = &(evt->fepAseCmd);
            AseNgSrv_SendMessage(pMsg);
            return Q_HANDLED();
        }
        case ASETK_TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, ASETK_SRV_PER_TICK_TIME_MS);
            AseNgSrv_CheckComWdgTimer(me);
            return Q_HANDLED();
        }
        /* receive commands from ASE_TK module */
        case ASETK_RECEIVE_COMMAND:
        {
            AseFepCmdEvt* evt = (AseFepCmdEvt*)e ;

#ifdef HAS_PRODUCTION_TEST_RECEIVE_ASE_TK_NOTICE
            {
                const bool connected = TRUE;
                Setting_Set(SETID_ASETK_CONNECTED, &connected);
                //ASENG_TP_PRINTF("\r\n\r\n\r\n\r\n *** ASE-TK init = 1 ***\r\n\r\n\r\n\r\n");
            }
#endif
            AseNgSrv_HandleAseCommand(me, evt->aseFepCmd);
            AseNgSrv_FeedComWdg(me);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            AseNgSrv_DisableComWdg(me);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* This state is used to as wait state. Wait until power off */
QState AseNgSrv_PreDeActive(cAseNgSrv * const me, QEvt const * const e)
{
    switch(e->sig)
    {
        case Q_ENTRY_SIG:
        {
            Seq_Ctor(&me->seq, me, AseTkModuleDestroySection, ArraySize(AseTkModuleDestroySection));
            PersistantObj_RefreshTick((cPersistantObj*)me, ASETK_SRV_PER_TICK_TIME_MS);

#ifdef HAS_PRODUCTION_TEST_RECEIVE_ASE_TK_NOTICE
            {
                const bool connected = FALSE;
                Setting_Set(SETID_ASETK_CONNECTED, &connected);
                //ASENG_TP_PRINTF("\r\n\r\n\r\n\r\n *** ASE-TK init = 0 ***\r\n\r\n\r\n\r\n");
            }
#endif
            return Q_HANDLED();
        }
        case ASETK_TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, ASETK_SRV_PER_TICK_TIME_MS);
            if (!Seq_isSeqFinished(&me->seq))
            {
                Seq_Refresh(&me->seq, ASETK_SRV_PER_TICK_TIME_MS);
            }
            else
            {
                CommonEvtResp((QActive*)me, me->pSender, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
                return Q_TRAN(AseNgSrv_DeActive);
            }
            return Q_HANDLED();
        }
        case SYSTEM_ACTIVE_REQ_SIG:
        {
            QActive_defer((QActive *)me, &deferredReqQue, e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        default:
            /* Not list all the QP signals here, so doesn't put a assert in the default*/
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/*   DeActive state - Use this state to ramp down the server  */
QState AseNgSrv_DeActive(cAseNgSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            QActive_recall((QActive*)me, &deferredReqQue);
            return Q_HANDLED();
        }
#ifdef HAS_PRODUCTION_TEST_DISABLE_ASE_TK_UART
        case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*) e;
            CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
            return Q_HANDLED();
        }
#endif
        case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*) e;
            me->pSender = pReq->sender;
            return Q_TRAN(&AseNgSrv_PreActive);
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}




/****************************************************
 * Public Function
 ****************************************************/
void AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp msg)
{
    AseNgCmdEvt* evt = Q_NEW(AseNgCmdEvt, ASE_TK_REQ_SIG);

    memset((void*)&evt->fepAseCmd,0, sizeof(evt->fepAseCmd));
    evt->fepAseCmd.which_OneOf  = Proto_Core_FepAseMessage_fepAseReq_tag;

    evt->fepAseCmd.OneOf.fepAseReq.has_type = TRUE;
    evt->fepAseCmd.OneOf.fepAseReq.type = msg;
    evt->fepAseCmd.OneOf.fepAseReq.has_id = TRUE;
    evt->fepAseCmd.OneOf.fepAseReq.id = msg;
    evt->fepAseCmd.OneOf.fepAseReq.which_data = 0;
    
    SendToServer(ASENG_SRV_ID,(QEvt*)evt);
}

void AseNgSrv_SendFepAseCmdRelativeVol(int32_t relativeVolume)
{
    AseNgCmdEvt* evt = Q_NEW(AseNgCmdEvt, ASE_TK_REQ_SIG);

    evt->fepAseCmd.which_OneOf  = Proto_Core_FepAseMessage_fepAseReq_tag;
    evt->fepAseCmd.OneOf.fepAseReq.has_type = true;
    evt->fepAseCmd.OneOf.fepAseReq.type = Proto_FepAse_ReqResp_VOLUME_CHANGE;
    
    //When req have data, .which_data need assign correct tag
    evt->fepAseCmd.OneOf.fepAseReq.which_data = Proto_FepAse_Req_relativeVolumeChange_tag;
    evt->fepAseCmd.OneOf.fepAseReq.data.relativeVolumeChange.has_volumeChange= TRUE;
    evt->fepAseCmd.OneOf.fepAseReq.data.relativeVolumeChange.volumeChange= relativeVolume;

    SendToServer(ASENG_SRV_ID,(QEvt*)evt);
}

void AseNgSrv_SendFepAseCmdAbsoluteVol(int32_t absoluteVolume)
{
    AseNgCmdEvt* evt = Q_NEW(AseNgCmdEvt, ASE_TK_REQ_SIG);

    evt->fepAseCmd.which_OneOf  = Proto_Core_FepAseMessage_fepAseReq_tag;
    evt->fepAseCmd.OneOf.fepAseReq.has_type = true;
    evt->fepAseCmd.OneOf.fepAseReq.type = Proto_FepAse_ReqResp_VOLUME_CHANGE;
    
    //When req have data, .which_data need assign correct tag
    evt->fepAseCmd.OneOf.fepAseReq.which_data= Proto_FepAse_Req_absoluteVolume_tag;
    evt->fepAseCmd.OneOf.fepAseReq.data.absoluteVolume.has_volume= TRUE;
    evt->fepAseCmd.OneOf.fepAseReq.data.absoluteVolume.volume= absoluteVolume;

    SendToServer(ASENG_SRV_ID,(QEvt*)evt);
}

//Send Events without data
void AseNgSrv_SendFepAseEvent(Proto_FepAse_Event_Type event)
{
    AseNgCmdEvt* evt = Q_NEW(AseNgCmdEvt, ASE_TK_REQ_SIG);

    evt->fepAseCmd.which_OneOf = Proto_Core_FepAseMessage_fepAseEvent_tag;
    evt->fepAseCmd.OneOf.fepAseEvent.has_type = TRUE;
    evt->fepAseCmd.OneOf.fepAseEvent.type = event;
    evt->fepAseCmd.OneOf.fepAseEvent.which_data = 0;

    SendToServer(ASENG_SRV_ID,(QEvt*)evt);
}


static bool AseNgSrv_encodeVersionInfoDetail_Ubl(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    //FepAseMessage const *pMsg = (FepAseMessage const *)arg;

    static uint8 product_name[]  = TP_PRODUCT;
    static uint8 ubl_name[] = "ubl";
    char *bootloader_version     = bl_readVersion( (void*)FEP_ADDR_UBL_VER );
    static uint8 ubl_meta[]= "m-ubl";

    ASENG_TP_PRINTF("Send Version to ASE: ubl %s, meta=%s\r\n", bootloader_version, ubl_meta);

    return pb_encode_tag(stream, PB_WT_STRING, 1) && //1 == Proto_FirmwareUpdate_VersionInfo_Module_name_tag
           pb_encode_string(stream, ubl_name, strlen(ubl_name)) &&
           pb_encode_tag(stream, PB_WT_STRING, 2) && //2 == Proto_FirmwareUpdate_VersionInfo_Module_version_tag
           pb_encode_string(stream, bootloader_version, strlen(bootloader_version)) &&
           pb_encode_tag(stream, PB_WT_STRING, 3) && //3 == Proto_FirmwareUpdate_VersionInfo_Module_metadata_tag
           pb_encode_string(stream, ubl_meta, strlen(ubl_meta));
}


static bool AseNgSrv_encodeVersionInfoDetail_App(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    //FepAseMessage const *pMsg = (FepAseMessage const *)arg;
    static uint8 app_name[] = "app";
    static uint8 app_meta[]= "m-app";

    /* Addition "d" may cause ASE upgrade FEP (assuem FEP is old version)
     * thus let release/debug have the same version
     */
    static uint8 application_version[]   = PRODUCT_VERSION_MCU;

    ASENG_TP_PRINTF("Send Version to ASE: app %s, meta=%s\r\n", application_version, app_meta);

    return pb_encode_tag(stream, PB_WT_STRING, 1) && //1 == Proto_FirmwareUpdate_VersionInfo_Module_name_tag
           pb_encode_string(stream, app_name, strlen(app_name)) &&
           pb_encode_tag(stream, PB_WT_STRING, 2) && //2 == Proto_FirmwareUpdate_VersionInfo_Module_version_tag
           pb_encode_string(stream, application_version, strlen(application_version)) &&
           pb_encode_tag(stream, PB_WT_STRING, 3) && //3 == Proto_FirmwareUpdate_VersionInfo_Module_metadata_tag
           pb_encode_string(stream, app_meta, strlen(app_meta));
}

static bool AseNgSrv_encodeVersionInfo(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    Proto_FirmwareUpdate_VersionInfo msg_ubl;
    Proto_FirmwareUpdate_VersionInfo msg_app;

    msg_ubl.module.funcs.encode = AseNgSrv_encodeVersionInfoDetail_Ubl;
    msg_ubl.module.arg = NULL;
    msg_app.module.funcs.encode = AseNgSrv_encodeVersionInfoDetail_App;
    msg_app.module.arg = NULL;

    return pb_encode_tag_for_field(stream, Proto_FirmwareUpdate_VersionInfo_fields) &&
           pb_encode_submessage(stream, Proto_FirmwareUpdate_VersionInfo_fields, &msg_ubl) &&
           pb_encode_tag_for_field(stream, Proto_FirmwareUpdate_VersionInfo_fields) &&
           pb_encode_submessage(stream, Proto_FirmwareUpdate_VersionInfo_fields, &msg_app);
}

void AseNgSrv_SendVersion(uint32 req_id)
{
    
    Proto_Core_FepAseMessage   fepAseMsg;
    fepAseMsg.which_OneOf = Proto_Core_FepAseMessage_fepAseResp_tag;
    fepAseMsg.OneOf.fepAseResp.has_id = TRUE;
    fepAseMsg.OneOf.fepAseResp.id  = req_id;
    fepAseMsg.OneOf.fepAseResp.has_genericResponse = TRUE;
    fepAseMsg.OneOf.fepAseResp.genericResponse.has_status = TRUE;
    fepAseMsg.OneOf.fepAseResp.genericResponse.status = Proto_Core_GenericResponse_Status_DONE;
    fepAseMsg.OneOf.fepAseResp.has_type = TRUE;
    fepAseMsg.OneOf.fepAseResp.type = Proto_AseFep_ReqResp_FIRMWARE_UPDATE_VERSION_INFO;
    fepAseMsg.OneOf.fepAseResp.which_data = Proto_FepAse_Resp_firmwareUpdateVersionInfo_tag;
    fepAseMsg.OneOf.fepAseResp.data.firmwareUpdateVersionInfo.module.funcs.encode = AseNgSrv_encodeVersionInfo;
    fepAseMsg.OneOf.fepAseResp.data.firmwareUpdateVersionInfo.module.arg = NULL;

    AseNgSrv_SendMessage(&fepAseMsg);

}

static bool AseNgSrv_encodeLogText(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_string(stream, *arg, strlen(*arg));
}

static bool AseNgSrv_encodeLogFile(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_string(stream, *arg, strlen(*arg));
}

AseNgNetworkInfo* AseNgSrv_GetDecodedNetworkInfo()
{
    return &glNetworkInfo;
}


void AseNgSrv_QueryInternalSpeakerCompensation()
{
//    if( !Setting_IsReady(SETID_ASETK_CONNECTED) || !(*(bool*)Setting_Get(SETID_ASETK_CONNECTED)) )
//    {
//        ASENG_TP_PRINTF("AseNgSrv_QueryInternalSpeakerCompensation: ignore because ASE-TK was not ready \r\n");
//        return;
//    }
//
//    ASENG_TP_PRINTF("AseNgSrv_QueryInternalSpeakerCompensation \r\n");
//    FepAseMessage msg;
//    msg.which_OneOf = (pb_size_t)FepAseMessage_fepAseReq_tag;
//    msg.OneOf.fepAseReq.request= FepAseReq_Request_GET_INTERNAL_SPEAKER_COMPENSATION;
//    AseNgSrv_SendMessage(&msg);
}


void AseNgSrv_SendLog(const char * str, const char * file, uint32 line)
{
    if( !Setting_IsReady(SETID_ASETK_CONNECTED) || !(*(bool*)Setting_Get(SETID_ASETK_CONNECTED)) )
    {
        return;
    }
    Proto_Core_FepAseMessage   fepAseMsg;
    fepAseMsg.which_OneOf = Proto_Core_FepAseMessage_fepAseEvent_tag;
    fepAseMsg.OneOf.fepAseEvent.has_type= TRUE;
    fepAseMsg.OneOf.fepAseEvent.type = Proto_FepAse_Event_Type_LOG_MESSAGE;
    fepAseMsg.OneOf.fepAseEvent.which_data = Proto_FepAse_Event_logMessage_tag;
    fepAseMsg.OneOf.fepAseEvent.data.logMessage.msg.funcs.encode = AseNgSrv_encodeLogText;
    fepAseMsg.OneOf.fepAseEvent.data.logMessage.msg.arg = (void*) str;
    AseNgSrv_SendMessage(&fepAseMsg);
}



static bool AseNgSrv_encodeComfortToneName(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_string(stream, *arg, strlen(*arg));
}


/* Play audio cue on ASE. 
 * parameter toneName can set to (ASEII-432):
 *   - action_interaction_a2.wav
 *   - action_interaction_a.wav
 *   - action_interaction_b.wav
 *   - event_status_successful.wav
 *   - event_status_unsuccessful.wav
 *   - mode_battery_low.wav
 *   - mode_product_off.wav
 *   - mode_product_on.wav
 */
void AseNgSrv_PlayComfortTone(char *toneName)
{
    AseNgCmdEvt* evt = Q_NEW(AseNgCmdEvt, ASE_TK_REQ_SIG);

    memset((void*)&evt->fepAseCmd,0, sizeof(evt->fepAseCmd));
    evt->fepAseCmd.which_OneOf  = Proto_Core_FepAseMessage_fepAseReq_tag;

    evt->fepAseCmd.OneOf.fepAseReq.has_type = TRUE;
    evt->fepAseCmd.OneOf.fepAseReq.type = Proto_FepAse_ReqResp_PLAY_AUDIO_CUE;
    evt->fepAseCmd.OneOf.fepAseReq.has_id = FALSE;
    evt->fepAseCmd.OneOf.fepAseReq.which_data = Proto_FepAse_Req_audioCue_tag;
    evt->fepAseCmd.OneOf.fepAseReq.data.audioCue.name.funcs.encode = AseNgSrv_encodeComfortToneName;
    
    //When req have data, .which_data need assign correct tag    
    /* Callback AseNgSrv_encodeComfortToneNameo will finish on AseNgSrv_SendMessage()
     * thus it is save to pass local parameter "toneName" to callback
     */
    evt->fepAseCmd.OneOf.fepAseReq.data.audioCue.name.arg = (void*)toneName;
    
    SendToServer(ASENG_SRV_ID,(QEvt*)evt);
}


void AseNgSrv_ReplyVolumeFade(uint32 req_id, bool result)
{
    //TP_PRINTF("\r\n\r\n\r\n*** ASE: reply volume fading: req_id=%d, ret=%d ***\r\n\r\n\r\n", req_id, result);
    AseNgSrv_GenericResponse(result, req_id, Proto_AseFep_ReqResp_DSP_VOLUME_FADE);
}


__weak bool AseNgSrv_encodeInternalSpeakerCompensationDetails(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    return 1;
}

void AseNgSrv_SendInternalSpeakerCompensation()
{
    //Proto_Core_FepAseMessage msg;
    //msg.which_OneOf = (pb_size_t)Proto_FepAse_Resp_internalSpeakerCompensation_tag;
    //msg.OneOf.fepAseResp.data.internalSpeakerCompensation.error= Proto_Dsp_ResponseInternalSpeakerCompensation_Error_NO_ERROR;
    //msg.OneOf.fepAseResp.data.internalSpeakerCompensation.has_error= TRUE;
    //msg.OneOf.fepAseResp.data.internalSpeakerCompensation.internalSpeaker.funcs.encode = AseNgSrv_encodeInternalSpeakerCompensationDetails;
    //msg.OneOf.fepAseResp.data.internalSpeakerCompensation.internalSpeaker.arg = NULL;

    //AseNgSrv_SendMessage(&msg);
}


void AseNgSrv_GenericResponse(bool success, uint32 req_id, Proto_AseFep_ReqResp type)
{
      Proto_Core_FepAseMessage rpmessage;
      rpmessage.which_OneOf = Proto_Core_FepAseMessage_fepAseResp_tag;
      rpmessage.OneOf.fepAseResp.has_id = TRUE;
      rpmessage.OneOf.fepAseResp.id = req_id;
      rpmessage.OneOf.fepAseResp.has_type = TRUE;
      rpmessage.OneOf.fepAseResp.type = type;
      rpmessage.OneOf.fepAseResp.has_genericResponse = TRUE;
      rpmessage.OneOf.fepAseResp.genericResponse.has_status = TRUE;
      rpmessage.OneOf.fepAseResp.which_data = 0;
      if (success)
      {
      rpmessage.OneOf.fepAseResp.genericResponse.status = Proto_Core_GenericResponse_Status_DONE;
      }
      else
      {
         rpmessage.OneOf.fepAseResp.genericResponse.status = Proto_Core_GenericResponse_Status_ERROR;
//         rpmessage.OneOf.fepAseResp.genericResponse.status = Proto_Core_GenericResponse_Status_NOT_SUPPORTED;
      }

      AseNgSrv_SendMessage(&rpmessage);
}


void AseNgSrv_ReplySetPositionSoundMode(Proto_Dsp_RequestPositionSoundMode soundModeResp)
{
//    FepAseMessage msg;
//    msg.which_OneOf = (pb_size_t)FepAseMessage_fepAsePositionSoundModeResp_tag;
//    msg.OneOf.fepAsePositionSoundModeResp = soundModeResp;
//    AseNgSrv_SendMessage(&msg);
}

void AseNgSrv_ReplyAudioPcmFormatCommand(bool success)
{
    //TODO
    //FepAseMessage msg;
    //msg.which_OneOf = (pb_size_t)FepAseMessage_fepAseSetAudioPcmFormatCommandResp_tag;
}


static bool AseNgSrv_encodeTunnelStream(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    sOutPutTunnelMessage *pOutPutTunnelMessage = *arg;

    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_string(stream, pOutPutTunnelMessage->OutputTunnelBuffer, pOutPutTunnelMessage->MessageSize);
}

void AseNgSrv_SendTunnel(const uint8 * stream, size_t size)
{
//    FepAseMessage msg;
//    sOutPutTunnelMessage glOutPutTunnelMessage;
//
//    glOutPutTunnelMessage.MessageSize = size;
//    memcpy(glOutPutTunnelMessage.OutputTunnelBuffer, stream, glOutPutTunnelMessage.MessageSize);
//
//    msg.which_OneOf = (pb_size_t)FepAseMessage_fepAseTunnel_tag;
//    msg.OneOf.fepAseTunnel.data.funcs.encode = AseNgSrv_encodeTunnelStream;
//    msg.OneOf.fepAseTunnel.data.arg = (void*) &glOutPutTunnelMessage;
//    AseNgSrv_SendMessage(&msg);


      Proto_Core_FepAseMessage evt_message;

      evt_message.which_OneOf = Proto_Core_FepAseMessage_fepAseEvent_tag;
      evt_message.OneOf.fepAseEvent.has_type = TRUE;
      evt_message.OneOf.fepAseEvent.type = Proto_FepAse_Event_Type_TUNNEL;
      evt_message.OneOf.fepAseEvent.which_data = Proto_FepAse_Event_productionTunnel_tag;
      evt_message.OneOf.fepAseEvent.data.productionTunnel.has_data = TRUE;
      evt_message.OneOf.fepAseEvent.data.productionTunnel.data.size = size;

      memcpy((uint8*)&evt_message.OneOf.fepAseEvent.data.productionTunnel.data.bytes[0], stream, size);

      AseNgSrv_SendMessage(&evt_message);

}

void AseNgSrv_FillTunnelMsg(uint8 * pbuff, uint8 size)
{
    memset(glInputTunnelBuffer, 0x00, ASETK_TUNNEL_BUFFER_SIZE);
    if (size <= sizeof(glInputTunnelBuffer))
    {
        memcpy((uint8*)&glInputTunnelBuffer[0], pbuff, size);
    }
    AseNgSrv_AseFepTunnelHandler();
    asm("nop");
}

bool AseNgSrv_TunnelInputCb(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    memset(glInputTunnelBuffer, 0x00, ASETK_TUNNEL_BUFFER_SIZE);

    if (stream->bytes_left > sizeof(glInputTunnelBuffer))
    {
        return false;
    }
    if (!pb_read(stream, glInputTunnelBuffer, stream->bytes_left))
    {
        return false;
    }
    return true;
}

void AseNgSrv_FillNetworkInfo(Proto_System_NetworkInfo_NetworkInterface * p_net_info)
{
    if (p_net_info->has_type && p_net_info->has_state)
    {
        if (p_net_info->type == Proto_System_NetworkInfo_NetworkInterface_Type_WIFI) {
            ASENG_TP_PRINTF("wifi=%d", p_net_info->state);
            glNetworkInfo.wifi_state= p_net_info->state;
        }
        else if(p_net_info->type==Proto_System_NetworkInfo_NetworkInterface_Type_ETHERNET) {
            ASENG_TP_PRINTF("ethernet=%d", p_net_info->state );
            glNetworkInfo.ethernet_state= p_net_info->state;
        }
        else if(p_net_info->type==Proto_System_NetworkInfo_NetworkInterface_Type_SOFT_AP) {
            ASENG_TP_PRINTF("softAp=%d", p_net_info->state );
            glNetworkInfo.soft_ap_state= p_net_info->state;
        }
        else {
            ASENG_TP_PRINTF("unknown network type (%d) state = %d", p_net_info->has_type, p_net_info->state );
        }
    }

    if (p_net_info->has_wifi)
    {
        if(p_net_info->wifi.has_Configured)
        {
            glNetworkInfo.wifi_configured= p_net_info->wifi.Configured;
            ASENG_TP_PRINTF(", wifi_configured=%d", p_net_info->wifi.Configured);
        }
        if(p_net_info->wifi.has_quality)
        {
            glNetworkInfo.wifi_quality= p_net_info->wifi.quality;
            ASENG_TP_PRINTF(", wifi_quality=%d", p_net_info->wifi.quality);
        }
    }
    ASENG_TP_PRINTF("\r\n");
}

__weak Proto_Dsp_ResponseInternalSpeakerCompensation_Error AseNgSrv_Callback_SetInternalSpeakerCompensation(Proto_Dsp_InternalSpeaker_Position position, Proto_Dsp_InternalSpeaker_Type type, double gain)
{
    return Proto_Dsp_ResponseInternalSpeakerCompensation_Error_NO_ERROR;
}
bool AseNgSrv_SetInternalSpeakerCompensationCb(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
  //TODO: support calibration data read/write
    Proto_Dsp_InternalSpeaker intSpk;
    memset(&intSpk, 0x00, sizeof(intSpk));

    if (!pb_decode_noinit(stream, Proto_Dsp_InternalSpeaker_fields, &intSpk))
    {
        return false;
    }

    if (intSpk.has_compensation)
    {
        AseNgSrv_Callback_SetInternalSpeakerCompensation(intSpk.position, intSpk.type, intSpk.compensation);
    }
    //ASENG_TP_PRINTF("AseNgSrv_SetInternalSpeakerCompensationCb: pos=%d, type=%d, gain=%f\r\n", intSpk.position, intSpk.type, intSpk.compensation);

    AseNgSrv_SendInternalSpeakerCompensation();

    return true;
}

__weak void AseNgSrv_AseFepDefaultTunnelHandler(uint8* buffer, uint16 bufferSize)
{
}

static void AseNgSrv_AseFepTunnelHandler()
{
#ifdef HAS_ASE_NG_SERVICE_TOOL
    switch(glInputTunnelBuffer[ASETK_TUNNEL_MO_MESSAGE_ID])
    {
        case ASETK_TUNNEL_COMMAND_ECHO_REQ:
        {
            glInputTunnelBuffer[ASETK_TUNNEL_MO_MESSAGE_ID] = ASETK_TUNNEL_COMMAND_ECHO_RESP;
            AseNgSrv_SendTunnel(glInputTunnelBuffer, glInputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] + 2);
            break;
        }
#ifdef BnO_fs1
        case ASETK_TUNNEL_COMMAND_BI_REQ:
        {
            volatile uint16 tempValue;

            glInputTunnelBuffer[ASETK_TUNNEL_MO_MESSAGE_ID] = ASETK_TUNNEL_COMMAND_BI_RESP;
            glInputTunnelBuffer[ASETK_TUNNEL_MO_SIZE]       = 22;

            tempValue = 0;
            if( Setting_IsReady(SETID_BATTERY_CAPACITY_RSOC_USER) )
            {
                tempValue = (uint16) *(uint8*)Setting_Get(SETID_BATTERY_CAPACITY_RSOC_USER);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 0]  = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 1]  = (uint8)((tempValue)     & 0xFF);

            tempValue = 0;
            if( Setting_IsReady(SETID_BATTERY_CAPACITY_RSOC) )
            {
                tempValue = (uint16) *(uint8*)Setting_Get(SETID_BATTERY_CAPACITY_RSOC);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 2]  = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 3]  = (uint8)((tempValue)     & 0xFF);

            tempValue = 0;
            if( Setting_IsReady(SETID_BATTERY_HEALTH_SOH) )
            {
                tempValue = *(uint16*)Setting_Get(SETID_BATTERY_HEALTH_SOH);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 4]  = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 5]  = (uint8)((tempValue)     & 0xFF);

            tempValue = 0;
            if( Setting_IsReady(SETID_BATTERY_CYCLE) )
            {
                tempValue = *(uint16*)Setting_Get(SETID_BATTERY_CYCLE);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 6]  = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 7]  = (uint8)((tempValue)     & 0xFF);

            tempValue = 0;
            if( Setting_IsReady(SETID_BATTERY_TEMP) )
            {
                tempValue = *(uint16*)Setting_Get(SETID_BATTERY_TEMP);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 8]  = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 9]  = (uint8)((tempValue)     & 0xFF);

            tempValue = 0;
            if( Setting_IsReady(SETID_BATTERY_HW) )
            {
                tempValue = *(uint16*)Setting_Get(SETID_BATTERY_HW);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 10]  = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 11]  = (uint8)((tempValue)     & 0xFF);

            tempValue = 0;
            if( Setting_IsReady(SETID_BATTERY_SW) )
            {
                tempValue = *(uint16*)Setting_Get(SETID_BATTERY_SW);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 12] = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 13] = (uint8)((tempValue)     & 0xFF);

            tempValue = 0;
            if( Setting_IsReady(SETID_BATTERY_TOTAL_VOL) )
            {
                tempValue = *(uint16*)Setting_Get(SETID_BATTERY_TOTAL_VOL);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 14] = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 15] = (uint8)((tempValue)     & 0xFF);

            tempValue = 0;
            if( Setting_IsReady(SETID_BATTERY_CURRENT) )
            {
                tempValue = *(uint16*)Setting_Get(SETID_BATTERY_CURRENT);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 16] = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 17] = (uint8)((tempValue)     & 0xFF);

            tempValue = 0;
            if( Setting_IsReady(SETID_BATTERY_SAFETY_STATUS_HIGH))
            {
                tempValue = *(uint16*)Setting_Get(SETID_BATTERY_SAFETY_STATUS_HIGH);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 18] = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 19] = (uint8)((tempValue)     & 0xFF);

            tempValue = 0;
            if(Setting_IsReady(SETID_BATTERY_SAFETY_STATUS_LOW))
            {
                tempValue = *(uint16*)Setting_Get(SETID_BATTERY_SAFETY_STATUS_LOW);
            }
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 20] = (uint8)((tempValue>> 8) & 0xFF);
            glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 21] = (uint8)((tempValue)     & 0xFF);

            AseNgSrv_SendTunnel(glInputTunnelBuffer, 22 + 2);
            break;
        }
#endif
        case ASETK_TUNNEL_COMMAND_SETT_REQ:
        {
            eSettingId id = (eSettingId)glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA];
            uint8 size = Setting_GetSize(id);//glInputTunnelBuffer[ASETK_TUNNEL_MO_SIZE];

            //prepare reply data
            memset(glOutputTunnelBuffer, 0x00, ASETK_TUNNEL_BUFFER_SIZE);
            glOutputTunnelBuffer[ASETK_TUNNEL_MO_MESSAGE_ID] = ASETK_TUNNEL_COMMAND_SETT_RESP;

            if(id>=SETID_MAX)
            {
                ASENG_TP_PRINTF("\r\n\r\nERROR: AseNgSrv_AseFepTunnelHandler: sett_id(=%d) is out of range\r\n\r\n", id);
                glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] = 0;
            }
            else if(!Setting_IsReady(id))
            {
                ASENG_TP_PRINTF("\r\n\r\nWARNING: AseNgSrv_AseFepTunnelHandler: sett_id(=%d) is not ready\r\n\r\n", id);
                glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] = 0;
            }
            else if (Setting_GetSize(id)!=size)
            {
                ASENG_TP_PRINTF("\r\n\r\nWARNING: AseNgSrv_AseFepTunnelHandler: sett_id(=%d) should have size=%dbytes, but reuqired size=%d\r\n\r\n", id, Setting_GetSize(id), size);
                glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] = 0;
            }
            else if (size+2 > ASETK_TUNNEL_BUFFER_SIZE)
            {
                ASENG_TP_PRINTF("\r\n\r\nWARNING: AseNgSrv_AseFepTunnelHandler: sett_id(=%d) have large size(=%dbytes), buffer is not large enough\r\n\r\n", id, size);
                glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] = 0;
            }
            else
            {
                uint8 *pSettVal= (uint8*)Setting_Get(id);
                glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] = size;
                memcpy(&glOutputTunnelBuffer[ASETK_TUNNEL_MO_DATA],pSettVal,size);
                //TP_PRINTF("read-sett(id=%d,size=%d) => reply , ", id, size);
                //int i;
                //for(i=2 ; i<size+2 ; i++)
                //{
                //    TP_PRINTF("%02x ", glOutputTunnelBuffer[i]);
                //}
                //TP_PRINTF("\r\n");
            }
            AseNgSrv_SendTunnel(glOutputTunnelBuffer, size+2);

            break;
        }
        
        case ASETK_TUNNEL_COMMAND_SETT_W_REQ:
        {
            eSettingId id = (eSettingId)glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA];
            uint8 size = glInputTunnelBuffer[ASETK_TUNNEL_MO_SIZE];

            //prepare reply data
            memset(glOutputTunnelBuffer, 0x00, ASETK_TUNNEL_BUFFER_SIZE);
            glOutputTunnelBuffer[ASETK_TUNNEL_MO_MESSAGE_ID] = ASETK_TUNNEL_COMMAND_SETT_W_RESP;

            if(id>=SETID_MAX)
            {
                ASENG_TP_PRINTF("\r\n\r\nERROR: AseNgSrv_AseFepTunnelHandler: sett_id(=%d) is out of range\r\n\r\n", id);
                glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] = 0;
            }
            else if (Setting_GetSize(id)!=size)
            {
                ASENG_TP_PRINTF("\r\n\r\nWARNING: AseNgSrv_AseFepTunnelHandler: sett_id(=%d) should have size=%dbytes, but reuqired size=%d\r\n\r\n", id, Setting_GetSize(id), size);
                glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] = 0;
            }
            else if (size+2 > ASETK_TUNNEL_BUFFER_SIZE)
            {
                ASENG_TP_PRINTF("\r\n\r\nWARNING: AseNgSrv_AseFepTunnelHandler: sett_id(=%d) have large size(=%dbytes), buffer is not large enough\r\n\r\n", id, size);
                glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] = 0;
            }
            else
            {
                Setting_Set(id, &(glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA + 1]));
                glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] = 0;
            }
            AseNgSrv_SendTunnel(glOutputTunnelBuffer, size+2);

            break;
        }
#ifdef TP_MONITOR_OVER_TUNNEL
        case ASETK_TUNNEL_COMMAND_TP_MONITOR_START_REQ:
        {
            glEnabledTpMonitorEmul = TRUE;
            glInputTunnelBuffer[ASETK_TUNNEL_MO_MESSAGE_ID] = ASETK_TUNNEL_COMMAND_TP_MONITOR_START_RESP;
            AseNgSrv_SendTunnel(glInputTunnelBuffer, glInputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] + 2);
            break;
        }

        case ASETK_TUNNEL_COMMAND_TP_MONITOR_REQ:
        {
            uint8 size = glInputTunnelBuffer[ASETK_TUNNEL_MO_SIZE];
            DebugSSrv_UartRxCallbackEmulator(&glInputTunnelBuffer[ASETK_TUNNEL_MO_DATA], size);
            break;
        }
#endif        
        default:
        {
            AseNgSrv_AseFepDefaultTunnelHandler(glInputTunnelBuffer, ASETK_TUNNEL_BUFFER_SIZE);
            break;
        }
    }
#endif /* #ifdef HAS_ASE_NG_SERVICE_TOOL */
}

#ifdef TP_MONITOR_OVER_TUNNEL
void AseNgSrv_FepAseTpMonitorEmulSend(uint8* data, uint16 size)
{
    if (TRUE == glEnabledTpMonitorEmul)
    {
        glOutputTunnelBuffer[ASETK_TUNNEL_MO_MESSAGE_ID] = ASETK_TUNNEL_COMMAND_TP_MONITOR_RESP;
        glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] = size;

        memcpy(&glOutputTunnelBuffer[ASETK_TUNNEL_MO_DATA], data, (size <= ASETK_TUNNEL_BUFFER_SIZE) ? size : ASETK_TUNNEL_BUFFER_SIZE);

        glOutputTunnelBuffer[ASETK_TUNNEL_MO_MESSAGE_ID] = ASETK_TUNNEL_COMMAND_SETT_RESP;

        AseNgSrv_SendTunnel(glOutputTunnelBuffer, glOutputTunnelBuffer[ASETK_TUNNEL_MO_SIZE] + 2);
    }
}
#endif

//CA17/EZ3/S53 have no battery, thus do not support power status reply
void AseNgSrv_SendPowerStatus()
{
//    const uint8 battery_level_unknown = 0;
//    FepAseMessage msg;
//
//
//    msg.which_OneOf = (pb_size_t)FepAseMessage_fepAsePowerStatus_tag;
//
//    msg.OneOf.fepAsePowerStatus.acLineStatus = FepAsePowerStatus_ACLineStatus_PLUGGED;
//    msg.OneOf.fepAsePowerStatus.batteryStatus = FepAsePowerStatus_BatteryStatus_NO_BATTERY;
//    msg.OneOf.fepAsePowerStatus.has_batteryLevel = false;
//    msg.OneOf.fepAsePowerStatus.batteryLevel = battery_level_unknown;
//    msg.OneOf.fepAsePowerStatus.healthStatus = FepAsePowerStatus_BatteryHealthStatus_UNKNOWN;
//
//#ifdef HAS_BATTERY
//    if (Setting_IsReady(SETID_IS_DC_PLUG_IN))
//    {
//        msg.OneOf.fepAsePowerStatus.acLineStatus = (FepAsePowerStatus_ACLineStatus)*(bool*)Setting_Get(SETID_IS_DC_PLUG_IN);
//    }
//
//    if (Setting_IsReady(SETID_BATTERY_CAPACITY_LEVEL))
//    {
//        BatteryStatus batteryLevel = (BatteryStatus)*(uint8*)Setting_Get(SETID_BATTERY_CAPACITY_LEVEL);
//        switch(batteryLevel)
//        {
//        case BatteryStatus_NO_BATTERY:
//            msg.OneOf.fepAsePowerStatus.batteryStatus = FepAsePowerStatus_BatteryStatus_NO_BATTERY;
//            break;
//        case BatteryStatus_LEVEL_CRITICAL:
//            msg.OneOf.fepAsePowerStatus.batteryStatus = FepAsePowerStatus_BatteryStatus_LEVEL_CRITICAL;
//            break;
//        case BatteryStatus_LEVEL_LOW:
//            msg.OneOf.fepAsePowerStatus.batteryStatus = FepAsePowerStatus_BatteryStatus_LEVEL_LOW;
//            break;
//        case BatteryStatus_LEVEL_MID:
//            msg.OneOf.fepAsePowerStatus.batteryStatus = FepAsePowerStatus_BatteryStatus_LEVEL_MIDDLE;
//            break;
//        case BatteryStatus_LEVEL_EXTRA:
//        case BatteryStatus_LEVEL_HIGH:
//            msg.OneOf.fepAsePowerStatus.batteryStatus = FepAsePowerStatus_BatteryStatus_LEVEL_HIGH;
//            break;
//        default:
//            ASSERT(0);
//            break;
//        }
//    }
//
//    if (Setting_IsReady(SETID_BATTERY_CAPACITY_RSOC_USER))
//    {
//        msg.OneOf.fepAsePowerStatus.has_batteryLevel = true;
//        msg.OneOf.fepAsePowerStatus.batteryLevel = *(uint8*)Setting_Get(SETID_BATTERY_CAPACITY_RSOC_USER);
//    }
//
//    if (Setting_IsReady(SETID_BATTERY_HEALTH_LEVEL))
//    {
//        uint8 batteryHealthLevel = *(uint8*)Setting_Get(SETID_BATTERY_HEALTH_LEVEL);
//        switch(batteryHealthLevel)
//        {
//        case BATT_HEALTH_CRITICAL: //power off system
//        case BATT_HEALTH_POOR: //suggest service center to replace battery
//            msg.OneOf.fepAsePowerStatus.healthStatus = FepAsePowerStatus_BatteryHealthStatus_POOR;
//            break;
//        case BATT_HEALTH_GOOD:
//            msg.OneOf.fepAsePowerStatus.healthStatus = FepAsePowerStatus_BatteryHealthStatus_GOOD;
//            break;
//        case BATT_HEALTH_UNKNOWN:
//        default:
//            break;
//        }
//    }
//#endif
//
//    if( !Setting_IsReady(SETID_ASETK_CONNECTED) || !(*(bool*)Setting_Get(SETID_ASETK_CONNECTED)) )
//        ASENG_TP_PRINTF("AseNgSrv_SendPowerStatus: ignore because ASE-TK is not ready\r\n");
//    else
//        ASENG_TP_PRINTF("AseNgSrv_SendPowerStatus: DC=%d, batt-cap=%d%%, batt-level=%d, batt-nhealth=%d\r\n",
//                    msg.OneOf.fepAsePowerStatus.acLineStatus, msg.OneOf.fepAsePowerStatus.batteryLevel, msg.OneOf.fepAsePowerStatus.batteryStatus, msg.OneOf.fepAsePowerStatus.healthStatus);
//
//    AseNgSrv_SendMessage(&msg);
}


static void AseNgSrv_AseFepEventHandler(cAseNgSrv * const me, Proto_AseFep_Event aseFepEvent)
{
    switch (aseFepEvent.type)
    {
    case Proto_AseFep_Event_Type_BOOTED:
        //AseNgSrv_EnableComWdg(me);      
      
        //ASE Init operation
        //AseNgSrv_QueryInternalSpeakerCompensation();
        //AseNgSrv_SendPowerStatus();
        ASENG_TP_PRINTF("AseFepEvent: %d=BOOTED\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_SYSTEM_STATUS_STANDBY:
        ASENG_TP_PRINTF("AseFepEvent: %d=SYSTEM_STATUS_STANDBY\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_SYSTEM_STATUS_ON:
        ASENG_TP_PRINTF("AseFepEvent: %d=SYSTEM_STATUS_ON\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_SYSTEM_STATUS_ASE_RESTART:
	    /* ASE_RESTART usually cause by wifi certificate tool, means
		 * ASE will stay on test mode for long time, FEP should disable
		 * watch dog
		 */
        ASENG_TP_PRINTF("AseFepEvent: %d=SYSTEM_STATUS_ASE_RESTART\r\n", aseFepEvent.type); 
        AseNgSrv_DisableComWdg(me);
        break;
    case Proto_AseFep_Event_Type_FACTORY_RESET_DONE:
        ASENG_TP_PRINTF("AseFepEvent: %d=FACTORY_RESET_DONE\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_SYSTEM_STATUS_ON_NO_OPERATION:
        ASENG_TP_PRINTF("AseFepEvent: %d=SYSTEM_STATUS_ON_NO_OPERATION\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_TUNNEL:
        //ASENG_TP_PRINTF("AseFepEvent: %d=TUNNEL\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_SW_UPDATE_STARTED:
        ASENG_TP_PRINTF("AseFepEvent: %d=SW_UPDATE_STARTED\r\n", aseFepEvent.type); 
        AseNgSrv_DisableComWdg(me);
        break;
    case Proto_AseFep_Event_Type_SW_UPDATE_FINISHED:
        ASENG_TP_PRINTF("AseFepEvent: %d=SW_UPDATE_FINISHED\r\n", aseFepEvent.type); 
        AseNgSrv_EnableComWdg(me);
        break;
    case Proto_AseFep_Event_Type_SW_UPDATE_FAILED:
        ASENG_TP_PRINTF("AseFepEvent: %d=SW_UPDATE_FAILED\r\n", aseFepEvent.type); 
        AseNgSrv_EnableComWdg(me);
        break;
    case Proto_AseFep_Event_Type_COMFORT_TONE_START:
        ASENG_TP_PRINTF("AseFepEvent: %d=COMFORT_TONE_START\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_COMFORT_TONE_DONE:
        ASENG_TP_PRINTF("AseFepEvent: %d=COMFORT_TONE_DONE\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_VOLUME_CHANGED:
        ASENG_TP_PRINTF("AseFepEvent: %d=VOLUME_CHANGED", aseFepEvent.type);
        if( aseFepEvent.data.volume.has_volume )
        {
            ASENG_TP_PRINTF(", vol=%d", aseFepEvent.data.volume.volume);
            if( aseFepEvent.data.volume.has_fade_duration )
            {
                ASENG_TP_PRINTF(", fad_duration=%d", aseFepEvent.data.volume.fade_duration);
            }
        }
        ASENG_TP_PRINTF("\r\n");
        break;
    case Proto_AseFep_Event_Type_MUTE_CHANGED:
        ASENG_TP_PRINTF("AseFepEvent: %d=MUTE_CHANGED\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_NETWORK_INFO:
        ASENG_TP_PRINTF("\r\nAseFepEvent: %d=NETWORK_INFO\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_PLAYER_DATA:
        ASENG_TP_PRINTF("AseFepEvent: %d=PLAYER_DATA", aseFepEvent.type);
        if( aseFepEvent.data.playerData.has_state )
        {
            ASENG_TP_PRINTF(", state=%d", aseFepEvent.data.playerData.state); 
        }
        if( aseFepEvent.data.playerData.has_source )
        {
            ASENG_TP_PRINTF(", source=%d", aseFepEvent.data.playerData.source); 
        }
        ASENG_TP_PRINTF("\r\n");
        break;
    case Proto_AseFep_Event_Type_FACTORY_RESET_START:
        ASENG_TP_PRINTF("AseFepEvent: %d=PLAYER_DATA\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_BT_PAIRING_ENABLED:
        ASENG_TP_PRINTF("AseFepEvent: %d=PLAYER_DATA\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_BT_PAIRING_DISABLED:
        ASENG_TP_PRINTF("AseFepEvent: %d=PLAYER_DATA\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_BT_PAIRING_FAILED:
        ASENG_TP_PRINTF("AseFepEvent: %d=BT_PAIRING_FAILED\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_BT_PAIRING_SUCCEEDED:
        ASENG_TP_PRINTF("AseFepEvent: %d=BT_PAIRING_SUCCEEDED\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_BTLE_PAIRING_ENABLED:
        ASENG_TP_PRINTF("AseFepEvent: %d=BTLE_PAIRING_ENABLED\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_BTLE_PAIRING_DISABLED:
        ASENG_TP_PRINTF("AseFepEvent: %d=BTLE_PAIRING_DISABLED\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_BTLE_PAIRING_FAILED:
        ASENG_TP_PRINTF("AseFepEvent: %d=BTLE_PAIRING_FAILED\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_BTLE_PAIRING_SUCCEEDED:
        ASENG_TP_PRINTF("AseFepEvent: %d=BTLE_PAIRING_SUCCEEDED\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_LOG_MESSAGE_AVAILABLE:
        ASENG_TP_PRINTF("AseFepEvent: %d=LOG_MESSAGE_AVAILABLE\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_LOG_MESSAGE_UNAVAILABLE:
        ASENG_TP_PRINTF("AseFepEvent: %d=LOG_MESSAGE_UNAVAILABLE\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_DSP_TONE_TOUCH:
        Proto_Dsp_ToneTouch *pTTouch= &(aseFepEvent.data.dspToneTouch);
        ASENG_TP_PRINTF("AseFepEvent: %d=DSP_TONE_TOUCH, enabled=%d, Gx1=%f, Gx2=%f, Gy2=%f, Gz=%f, k5=%f, k6=%f\r\n", 
                        aseFepEvent.type, pTTouch->enabled, pTTouch->Gx1, pTTouch->Gx2, 
                        pTTouch->Gy2,     pTTouch->Gz,      pTTouch->k5,  pTTouch->k6);
        break;
    case Proto_AseFep_Event_Type_SYSTEM_STATUS_STANDBY_MULTIROOM:
        ASENG_TP_PRINTF("AseFepEvent: %d=SYSTEM_STATUS_STANDBY_MULTIROOM\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_SYSTEM_STATUS_TURNING_ON:
        ASENG_TP_PRINTF("AseFepEvent: %d=SYSTEM_STATUS_TURNING_ON\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_SYSTEM_STATUS_AUDIO_ONLY:
        ASENG_TP_PRINTF("AseFepEvent: %d=SYSTEM_STATUS_AUDIO_ONLY\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_BT_PLAYER_CONNECTED:
        ASENG_TP_PRINTF("AseFepEvent: %d=BT_PLAYER_CONNECTED\r\n", aseFepEvent.type); 
        break;
    case Proto_AseFep_Event_Type_BT_PLAYER_DISCONNECTED:
        ASENG_TP_PRINTF("AseFepEvent: %d=BT_PLAYER_DISCONNECTED\r\n", aseFepEvent.type); 
        break;
    default:
        ASENG_TP_PRINTF("AseFepEvent: %d=unknown\r\n", aseFepEvent.type); 
        break;
    }
}

static void AseNgSrv_SendBackResponse(cAseNgSrv * const me, uint32 req_id, Proto_AseFep_ReqResp type, Proto_Core_GenericResponse_Status status)
{
    Proto_Core_FepAseMessage   fepAseMsg;
    fepAseMsg.which_OneOf = Proto_Core_FepAseMessage_fepAseResp_tag;
    fepAseMsg.OneOf.fepAseResp.has_id = TRUE;
    fepAseMsg.OneOf.fepAseResp.id  = req_id;
    fepAseMsg.OneOf.fepAseResp.has_type = TRUE;
    fepAseMsg.OneOf.fepAseResp.type = type;
    fepAseMsg.OneOf.fepAseResp.has_genericResponse = TRUE;
    fepAseMsg.OneOf.fepAseResp.genericResponse.has_status = TRUE;
    fepAseMsg.OneOf.fepAseResp.genericResponse.status = status;
    fepAseMsg.OneOf.fepAseResp.which_data = 0;

    AseNgSrv_SendMessage(&fepAseMsg);
}

      

static void AseNgSrv_AseFepReqHandler(cAseNgSrv * const me, Proto_AseFep_Req aseFepReq)
{
    switch(aseFepReq.type)
    {
    case Proto_AseFep_ReqResp_FIRMWARE_UPDATE_VERSION_INFO:
        ASENG_TP_PRINTF("AseFepReq: %d=FIRMWARE_UPDATE_VERSION_INFO\r\n", aseFepReq.type);
        AseNgSrv_SendVersion(aseFepReq.id);
        break;
        
    case Proto_AseFep_ReqResp_FIRMWARE_UPDATE_CHUNK:
        ASENG_TP_PRINTF("AseFepReq: %d=FIRMWARE_UPDATE_CHUNK\r\n", aseFepReq.type);
        break;
        
    case Proto_AseFep_ReqResp_FIRMWARE_UPDATE_SWITCH_TO_BOOTLOADER:
      {
#ifdef HAS_REJECT_OTA_FROM_ASE
        AseNgSrv_SendBackResponse(me, aseFepReq.id, Proto_AseFep_ReqResp_FIRMWARE_UPDATE_SWITCH_TO_BOOTLOADER, Proto_Core_GenericResponse_Status_ERROR);
        return;
#endif
        ASENG_TP_PRINTF("\r\n<<< AseNgSrv_AseFepEventHandler: jump to bootaloder (Proto_AseFep_ReqResp_FIRMWARE_UPDATE_SWITCH_TO_BOOTLOADER) >>>\r\n\r\n");

        AseNgSrv_SendBackResponse(me, aseFepReq.id, Proto_AseFep_ReqResp_FIRMWARE_UPDATE_SWITCH_TO_BOOTLOADER, Proto_Core_GenericResponse_Status_DONE);


//        UartDrv_Xtor(&aseTk_uart);
//        UartDrv_Ctor(&aseTk_uart, (tUARTDevice*)getDevicebyIdAndType(ASETK2_DEV_ID, UART_DEV_TYPE, NULL), &txBuf, &rxBuf);

//        UartDrv_Xtor(&aseTk_uart);
//        UartDrv_Ctor(&aseTk_uart, (tUARTDevice*)getDevicebyIdAndType(ASETK2_DEV_ID, UART_DEV_TYPE, NULL), &txBuf, &rxBuf);
//        BSP_BlockingDelayMs(50);
//        UartDrv_Xtor(&aseTk_uart);




#ifdef ASE_TK_OTA
        AseNgSrv_updateStblStatus(me, FEP_STBL_UPGRADE_FIRMWARE);

        /* Kick dog before jump, to ensure firmware upgrade have enough time */
#ifdef HAS_IWDG
        IWDG_ReloadCounter();
#endif
        /* ASETK-298
         * Delay here to ensure FEP have enough time to reply ACK before jump */
        BSP_BlockingDelayMs(200);

        bl_jumpAddr(FEP_ADDR_UBL);
#endif

      break;
      }
      
    case Proto_AseFep_ReqResp_HDMI_ARC_STATUS:
        ASENG_TP_PRINTF("AseFepReq: %d=HDMI_ARC_STATUS\r\n", aseFepReq.type);
        break;
        
    case Proto_AseFep_ReqResp_HDMI_ARC_START:
        ASENG_TP_PRINTF("AseFepReq: %d=HDMI_ARC_START\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_HDMI_ARC_END:
        ASENG_TP_PRINTF("AseFepReq: %d=HDMI_ARC_END\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_HDMI_INPUT_SELECT:
        ASENG_TP_PRINTF("AseFepReq: %d=HDMI_INPUT_SELECT\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_POWER_STATUS:
        ASENG_TP_PRINTF("AseFepReq: %d=POWER_STATUS\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_LINE_IN_SENSITIVITY:
        ASENG_TP_PRINTF("AseFepReq: %d=LINE_IN_SENSITIVITY", aseFepReq.type);
        if( aseFepReq.data.lineInSensitivity.has_sensitivity )
        {
              ASENG_TP_PRINTF(", sensitivity=%d" , aseFepReq.data.lineInSensitivity.sensitivity);
        }
        ASENG_TP_PRINTF("\r\n");
        break;
    case Proto_AseFep_ReqResp_AUDIO_INPUT:
        ASENG_TP_PRINTF("AseFepReq: %d=AUDIO_INPUT", aseFepReq.type);
        if( aseFepReq.data.audioInput.has_local )
        {
              ASENG_TP_PRINTF(", local=%d(%s)" , aseFepReq.data.audioInput.local, ((aseFepReq.data.audioInput.local)?"AuxIn-Local":"ASE"));
        }
        if( aseFepReq.data.audioInput.has_input )
        {
            ASENG_TP_PRINTF(", input=%d(%s)", aseFepReq.data.audioInput.input, ((aseFepReq.data.audioInput.input)?"AUXIN":"WIFI/BT") ); 
        }
        ASENG_TP_PRINTF("\r\n");
        break;
    case Proto_AseFep_ReqResp_POSITION_SOUND_MODE:
        ASENG_TP_PRINTF("AseFepReq: %d=POSITION_SOUND_MODE", aseFepReq.type);
        if( aseFepReq.data.positionSoundMode.has_positionSoundMode )
        {
            if( aseFepReq.data.positionSoundMode.positionSoundMode.has_orientation )
                ASENG_TP_PRINTF( ", orie=%d", aseFepReq.data.positionSoundMode.positionSoundMode.orientation );
            if( aseFepReq.data.positionSoundMode.positionSoundMode.has_position )
                ASENG_TP_PRINTF( ", pos=%d", aseFepReq.data.positionSoundMode.positionSoundMode.position );
        }
        ASENG_TP_PRINTF("\r\n");
        break;
    case Proto_AseFep_ReqResp_INTERNAL_SPEAKER_COMPENSATION:
        ASENG_TP_PRINTF("AseFepReq: %d=INTERNAL_SPEAKER_COMPENSATION\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_POWER_LINK_ALL_SENSES_STATUS:
        ASENG_TP_PRINTF("AseFepReq: %d=POWER_LINK_ALL_SENSES_STATUS\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_POWER_LINK_SET_ON:
        ASENG_TP_PRINTF("AseFepReq: %d=POWER_LINK_SET_ON\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_POWER_LINK_SET_MUTE:
        ASENG_TP_PRINTF("AseFepReq: %d=POWER_LINK_SET_MUTE\r\n", aseFepReq.type);
        break;
        
    case Proto_AseFep_ReqResp_INTERNAL_AMPLIFIER_COMMAND:
        ASENG_TP_PRINTF("AseFepReq: %d=INTERNAL_AMPLIFIER_COMMAND\r\n", aseFepReq.type);
        break;
        
    case Proto_AseFep_ReqResp_FEP_APPLICATION_IS_RUNNING:
    {
        ASENG_TP_PRINTF("AseFepReq: %d=FEP_APPLICATION_IS_RUNNING....ping\r\n", aseFepReq.type);
        AseNgSrv_SendBackResponse(me, aseFepReq.id, Proto_AseFep_ReqResp_FEP_APPLICATION_IS_RUNNING, Proto_Core_GenericResponse_Status_DONE);
        break;
    }
    case Proto_AseFep_ReqResp_EEB_TELEGRAM_TRANSMIT:
        ASENG_TP_PRINTF("AseFepReq: %d=EEB_TELEGRAM_TRANSMIT\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_PRODUCTION_LED_MODE_SET:
        ASENG_TP_PRINTF("AseFepReq: %d=PRODUCTION_LED_MODE_SET", aseFepReq.type);   
        if (aseFepReq.data.ledMode.has_led)
        {
            ASENG_TP_PRINTF(", led=%d", aseFepReq.data.ledMode.led);
        }                       
        ASENG_TP_PRINTF(", onTimeMs=%d, offTimeMs=%d\r\n", aseFepReq.data.ledMode.offTimeMs, aseFepReq.data.ledMode.offTimeMs);          
        break;
        
    case Proto_AseFep_ReqResp_PRODUCTION_GET_BUTTON_STATE:
        ASENG_TP_PRINTF("AseFepReq: %d=PRODUCTION_GET_BUTTON_STATE\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_HDMI_CEC_SEND_STANDBY:
        ASENG_TP_PRINTF("AseFepReq: %d=HDMI_CEC_SEND_STANDBY\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_HDMI_CEC_SEND_POWER_UP_TV:
        ASENG_TP_PRINTF("AseFepReq: %d=HDMI_CEC_SEND_POWER_UP_TV\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_WPL_COMMAND:
        ASENG_TP_PRINTF("AseFepReq: %d=WPL_COMMAND\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_PUC_COMMAND_SEND:
        ASENG_TP_PRINTF("AseFepReq: %d=PUC_COMMAND_SEND\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_POWER_REQUEST:
        ASENG_TP_PRINTF("AseFepReq: %d=POWER_REQUEST", aseFepReq.type);
        if( aseFepReq.data.powerRequest.has_type )
        {
            ASENG_TP_PRINTF(", type=%d(%s)", aseFepReq.data.powerRequest.type, ((!aseFepReq.data.powerRequest.type)?"PowerOff":"Reboot"));
        }
        if( aseFepReq.data.powerRequest.has_delay_ms )
        {
            ASENG_TP_PRINTF(", delay_ms=%d", aseFepReq.data.powerRequest.delay_ms);
        }
        ASENG_TP_PRINTF("\r\n");
        break;
        
    case Proto_AseFep_ReqResp_HDMI_UHD_DEEP_COLOUR_ON:
        ASENG_TP_PRINTF("AseFepReq: %d=HDMI_UHD_DEEP_COLOUR_ON\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_HDMI_UHD_DEEP_COLOUR_OFF:
        ASENG_TP_PRINTF("AseFepReq: %d=HDMI_UHD_DEEP_COLOUR_OFF\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_POWERSUPPLY_VOLTAGE:
        ASENG_TP_PRINTF("AseFepReq: %d=POWERSUPPLY_VOLTAGE\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SEND_POWER_LINK_DATA:
        ASENG_TP_PRINTF("AseFepReq: %d=SEND_POWER_LINK_DATA\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_TOSLINK_OUT_ADJUST_SAMPLE_RATE:
        ASENG_TP_PRINTF("AseFepReq: %d=TOSLINK_OUT_ADJUST_SAMPLE_RATE\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_TOSLINK_OUT_VOLUME_REGULATION_ON:
        ASENG_TP_PRINTF("AseFepReq: %d=TOSLINK_OUT_VOLUME_REGULATION_ON\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_TOSLINK_OUT_VOLUME_REGULATION_OFF:
        ASENG_TP_PRINTF("AseFepReq: %d=TOSLINK_OUT_VOLUME_REGULATION_OFF\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_GET_HDMI_UHD_DEEP_COLOUR_STATUS:
        ASENG_TP_PRINTF("AseFepReq: %d=GET_HDMI_UHD_DEEP_COLOUR_STATUS\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_GET_HDMI_AUDIO_FORMAT:
        ASENG_TP_PRINTF("AseFepReq: %d=GET_HDMI_AUDIO_FORMAT\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_GET_HDMI_INPUT_SELECTED:
        ASENG_TP_PRINTF("AseFepReq: %d=GET_HDMI_INPUT_SELECTED\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_GET_HDMI_SENSE_STATUS:
        ASENG_TP_PRINTF("AseFepReq: %d=GET_HDMI_SENSE_STATUS\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SPEAKER_ENABLE_COMMAND:
        ASENG_TP_PRINTF("AseFepReq: %d=SPEAKER_ENABLE_COMMAND\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_HDMI_AUDIO_MODE_SELECT:
        ASENG_TP_PRINTF("AseFepReq: %d=HDMI_AUDIO_MODE_SELECT\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_LIGHT_SENSOR_TELEGRAM:
        ASENG_TP_PRINTF("AseFepReq: %d=LIGHT_SENSOR_TELEGRAM\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_GET_A2B_MODE:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_GET_A2B_MODE\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_SET_A2B_MODE:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_SET_A2B_MODE\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_SET_GAIN_AND_DELAY:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_SET_GAIN_AND_DELAY\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_SET_DRIVER_GAIN:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_SET_DRIVER_GAIN\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_SET_BASS_AND_ROOMEQ:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_SET_BASS_AND_ROOMEQ\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_GET_TOTAL_NODES:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_GET_TOTAL_NODES\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_DSP_PARAMETER:
        ASENG_TP_PRINTF("AseFepReq: %d=DSP_PARAMETER", aseFepReq.type);
        if( aseFepReq.data.dspParameter.has_type )
        {
            ASENG_TP_PRINTF(", type=%d", aseFepReq.data.dspParameter.type);
        }
        if( aseFepReq.data.dspParameter.has_value )
        {
            ASENG_TP_PRINTF(", value=%d", aseFepReq.data.dspParameter.value);
        }
        ASENG_TP_PRINTF("\r\n");
        break;

    case Proto_AseFep_ReqResp_SOUNDWALL_GET_POWER_MODE:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_GET_POWER_MODE\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_SET_POWER_MODE:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_SET_POWER_MODE\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_WRITE_DSP_PARAM:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_WRITE_DSP_PARAM\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_SET_MUTE_MODE:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_SET_MUTE_MODE\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_GET_NTC_INFO:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_GET_NTC_INFO\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_SYSTEM_RESTART:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_SYSTEM_RESTART\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_GET_GAIN_AND_DELAY:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_GET_GAIN_AND_DELAY\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_GET_DRIVER_GAIN:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_GET_DRIVER_GAIN\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_GET_BASS_AND_ROOMEQ:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_GET_BASS_AND_ROOMEQ\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_SOUNDWALL_SET_TEST_TONE:
        ASENG_TP_PRINTF("AseFepReq: %d=SOUNDWALL_SET_TEST_TONE\r\n", aseFepReq.type);
        break;
    case Proto_AseFep_ReqResp_DSP_VOLUME_FADE:
        ASENG_TP_PRINTF("AseFepReq: %d=DSP_VOLUME_FADE, req_id=%d", aseFepReq.type, aseFepReq.id);
        if( aseFepReq.data.dspAbsoluteVolume.has_volume )
        {
            ASENG_TP_PRINTF(", vol=%d", aseFepReq.data.dspAbsoluteVolume.volume);
        }
        if( aseFepReq.data.dspAbsoluteVolume.has_fade_duration )
        {
            ASENG_TP_PRINTF(", duration=%dms", aseFepReq.data.dspAbsoluteVolume.fade_duration);
        }
        ASENG_TP_PRINTF("\r\n");
        break;
    default:
        ASENG_TP_PRINTF("AseFepReq: %d=unknown\r\n", aseFepReq.type);
        break;
    }
}


static void AseNgSrv_AseFepRespHandler(cAseNgSrv * const me, Proto_AseFep_Resp aseFepResp)
{
    if( aseFepResp.has_type )
    {
#ifndef NDEBUG
        char str_g_resp[20]= "";
        if(  aseFepResp.has_genericResponse && aseFepResp.genericResponse.has_status )
        {
            snprintf(str_g_resp, sizeof(str_g_resp), ", gRsp=%d", aseFepResp.genericResponse.status);
        }
#endif        
        switch (aseFepResp.type)
        {
        case Proto_FepAse_ReqResp_PING:
            ASENG_TP_PRINTF("AseFepResp: %d=PING %s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_PLAY:
            ASENG_TP_PRINTF("AseFepResp: %d=PLAY%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_PAUSE:
            ASENG_TP_PRINTF("AseFepResp: %d=PAUSE%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_STOP:
            ASENG_TP_PRINTF("AseFepResp: %d=STOP%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_NEXT:
            ASENG_TP_PRINTF("AseFepResp: %d=NEXT%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_PREV:
            ASENG_TP_PRINTF("AseFepResp: %d=PREV%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_NEXT_SOURCE:
            ASENG_TP_PRINTF("AseFepResp: %d=NEXT_SOURCE%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_JOIN:
            ASENG_TP_PRINTF("AseFepResp: %d=JOIN%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_PLAY_PAUSE_TOGGLE:
            ASENG_TP_PRINTF("AseFepResp: %d=PLAY_PAUSE_TOGGLE%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_SOUND_SILENCE_TOGGLE:
            ASENG_TP_PRINTF("AseFepResp: %d=SOUND_SILENCE_TOGGLE%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_SOUND:
            ASENG_TP_PRINTF("AseFepResp: %d=SOUND%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_SILENCE:
            ASENG_TP_PRINTF("AseFepResp: %d=SILENCE%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_MUTE:
            ASENG_TP_PRINTF("AseFepResp: %d=MUTE%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_UNMUTE:
            ASENG_TP_PRINTF("AseFepResp: %d=UNMUTE%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_BT_PAIRING_ON:
            ASENG_TP_PRINTF("AseFepResp: %d=BT_PAIRING_ON%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_BT_PAIRING_OFF:
            ASENG_TP_PRINTF("AseFepResp: %d=BT_PAIRING_OFF%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_BTLE_PAIRING_ON:
            ASENG_TP_PRINTF("AseFepResp: %d=BTLE_PAIRING_ON%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_BTLE_PAIRING_OFF:
            ASENG_TP_PRINTF("AseFepResp: %d=BTLE_PAIRING_OFF%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_BT_PAIRING_TOGGLE:
            ASENG_TP_PRINTF("AseFepResp: %d=BT_PAIRING_TOGGLE%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_OFF:
            ASENG_TP_PRINTF("AseFepResp: %d=OFF, has_gen=%d, gen_has_status=%d, gen_status=%d\r\n", aseFepResp.type, aseFepResp.has_genericResponse, aseFepResp.genericResponse.has_status, aseFepResp.genericResponse.status);
            break;
        case Proto_FepAse_ReqResp_STORAGE:
            ASENG_TP_PRINTF("AseFepResp: %d=STORAGE%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_FACTORY_RESET:
            ASENG_TP_PRINTF("AseFepResp: %d=FACTORY_RESET, has_status=%d, status=%d\r\n", aseFepResp.type, aseFepResp.genericResponse.has_status, aseFepResp.genericResponse.status);
            break;
        case Proto_FepAse_ReqResp_NETWORK_SETUP:
            ASENG_TP_PRINTF("AseFepResp: %d=NETWORK_SETUP%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_STANDBY:
            ASENG_TP_PRINTF("AseFepResp: %d=STANDBY%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_ALL_STANDBY:
            ASENG_TP_PRINTF("AseFepResp: %d=ALL_STANDBY%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_NETWORK_INFO:
            ASENG_TP_PRINTF("AseFepResp: %d=NETWORK_INFO%s\r\n", aseFepResp.type, str_g_resp);
            break;
        case Proto_FepAse_ReqResp_VOLUME_CHANGE:
            ASENG_TP_PRINTF("AseFepResp: %d=VOLUME_CHANGE%s\r\n", aseFepResp.type, str_g_resp);
            if( aseFepResp.data.volume.has_volume )
            {
                ASENG_TP_PRINTF(", vol=%d", aseFepResp.data.volume.volume);
                if( aseFepResp.data.volume.has_fade_duration )
                {
                    ASENG_TP_PRINTF(", fad_duration=%d", aseFepResp.data.volume.fade_duration);
                }
            }
            break;
        case Proto_FepAse_ReqResp_WPL_COMMAND:
            ASENG_TP_PRINTF("AseFepResp: %d=WPL_COMMAND%s\r\n", aseFepResp.type, str_g_resp);
            break;
        default:
            ASENG_TP_PRINTF("AseFepResp: %d=unknown%s\r\n", aseFepResp.type, str_g_resp);
            break;
        }
    }
}

//static Proto_Core_AseFepMessage msg_buf[26] = {0};
//static uint8 mb_idx = 0;

static void AseNgSrv_HandleAseCommand(cAseNgSrv * const me, Proto_Core_AseFepMessage message)
{
//#ifdef ASE_TK_DEBUG_SHOW_RX_DATA
//    ASENG_TP_PRINTF("    ASE:%d 0x%X ", message.which_OneOf, message.OneOf.aseFepNetworkInfo.networkInfo.arg);
//#endif
//    if (mb_idx < 25)
//    {
//       msg_buf[mb_idx++] = message;
//    }
//    else
//    {
//        mb_idx = 0;
//        asm("nop");
//    }

    switch (message.which_OneOf)
    {
    case Proto_Core_AseFepMessage_aseFepEvent_tag:
        AseNgSrv_AseFepEventHandler(me, message.OneOf.aseFepEvent);
        break;
    case Proto_Core_AseFepMessage_aseFepReq_tag:
        AseNgSrv_AseFepReqHandler(me, message.OneOf.aseFepReq);
        break;
    case Proto_Core_AseFepMessage_aseFepResp_tag:
        AseNgSrv_AseFepRespHandler(me, message.OneOf.aseFepResp);
        break;
    default:
        ASENG_TP_PRINTF("Unknown AseFepMeg: which_OneOf=%d\r\n", message.which_OneOf);
        break;
    }

    /* publish ASE cmd */
    AseNgStateIndEvt* pAseNgStateEvt = Q_NEW(AseNgStateIndEvt, ASE_TK_STATE_SIG);
    pAseNgStateEvt->aseFepCmd = message;
    QF_PUBLISH(&pAseNgStateEvt->super, me);

}

/***************************************************/
/************send/receive func with ASE_TK*************/
/**************************************************/
/* time out handler function, re-send the package */
static void AseNgSrv_TimeOutHandler(uint8* data, uint8 size)
{
    ASENG_TP_PRINTF("time out and re_send\r\n");
    Proto_Core_FepAseMessage* pMessage = (Proto_Core_FepAseMessage*)data;
    AseNgSrv_SendMessage(pMessage);
}


/* send the message to ase module by uart */
static void AseNgSrv_SendMessage(Proto_Core_FepAseMessage* pMessage)
{
    if( !Setting_IsReady(SETID_ASETK_CONNECTED) || !(*(bool*)Setting_Get(SETID_ASETK_CONNECTED)) )
    {
        return;
    }
    //Sometime Uart is xtor but SETID_ASETK_CONNECTED=TRUE, here is workaround to avoid problem
    ASSERT(aseTk_uart.pConfig);
    if( !aseTk_uart.pConfig )
    {
        return;
    }

    static int seq = 0;
    static uint8_t buffer[ASETK_SRV_DECODE_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    size_t message_length = message_to_data(pMessage, (char*)buffer, sizeof(buffer));
    /* if data encode successfully */
    if(message_length>0)
    {
        /* Do not print ProductLog message, becuase it have too many events */
//        if( pMessage->which_OneOf != (pb_size_t)FepAseMessage_fepAseProductLog_tag ) {
//            ASENG_TP_PRINTF("AseNgSrv_SendMessage: which_OneOf=%d, fepAseCommand=%d\r\n", pMessage->which_OneOf, pMessage->OneOf.fepAseCommand.command);
//        }

        static uint8 dest_buff[ASETK_SRV_DECODE_BUFFER_SIZE];
        uint32 size = 0;
        yahdlc_control_t control;
        /* framing the data streaming */
        control.frame = YAHDLC_FRAME_DATA;
        control.seq_no = seq;
        seq = yahdlc_seq_advance(seq);
        yahdlc_frame_data(&control, (char*)buffer, message_length, (char*)dest_buff, (unsigned int*)&size);
        UartDrv_Write(&aseTk_uart, (uint8* )dest_buff, size);
    }
}
//static uint8 ver_resp[] = {0x7e,0xff,0x1c,0x1a,0x47,0x08,0x00,0x10,0x01,0x1a,0x02,0x08,0x00,0x62,0x3d,0x0a,0x15,0x0a,0x03,0x61,0x70,0x70,0x12,0x0e,0x30,0x2e,0x30,0x2e,0x30,0x2e,0x65,0x65,0x61,0x33,0x31,0x62,0x34,0x66,0x0a,0x15,0x0a,0x03,0x62,0x73,0x6c,0x12,0x0e,0x30,0x2e,0x30,0x2e,0x30,0x2e,0x66,0x63,0x33,0x34,0x30,0x30,0x32,0x30,0x0a,0x0d,0x0a,0x03,0x77,0x70,0x6c,0x12,0x06,0x32,0x30,0x30,0x2e,0x30,0x31,0x03,0x13,0x7e};
//static void send_ver_resp()
//{
//
//  UartDrv_Write(&aseTk_uart, (uint8* )ver_resp, 79);
//
//}

static inline void AseNgSrv_ACK(uint8 seq_no)
{
    yahdlc_control_t control;
    uint32 size = 0;
    uint8 dest_buff[ASETK_SRV_DECODE_BUFFER_SIZE];
    uint8* fake_src = dest_buff;    // fake src pointer as yahdlc does not accept src to be null

    control.frame = YAHDLC_FRAME_ACK;
    if (seq_no >= YAHDLC_SEQ_NO_MAX)
    {
        control.seq_no = 0;
    }
    else
    {
        control.seq_no= seq_no + 1;
    }
    yahdlc_frame_data(&control, (char*)fake_src, 1, (char*)dest_buff, (unsigned int*)&size);
    UartDrv_Write(&aseTk_uart, (uint8* )dest_buff, size);
    //ASENG_TP_PRINTF("ACK\r\n");
}

static inline void AseNgSrv_NACK(uint8 seq_no)
{
    yahdlc_control_t control;
    uint32 size = 0;
    uint8 dest_buff[ASETK_SRV_DECODE_BUFFER_SIZE];
    uint8* fake_src = dest_buff;    // fake src pointer as yahdlc does not accept src to be null

    control.frame = YAHDLC_FRAME_NACK;
    control.seq_no= seq_no;
    yahdlc_frame_data(&control, (char*)fake_src, 1, (char*)dest_buff, (unsigned int*)&size);
    UartDrv_Write(&aseTk_uart, (uint8* )dest_buff, size);
    //ASENG_TP_PRINTF("Send NACK to ASE\r\n");
}


//static uint8 raw_buffer[512] = {0};
//static uint16 rb_idx = 0;

/* Call back function receiving message from ase module */
/* After decode the commands, this function will send the message to AseNgSrv to handle the command */
static void AseNgSrv_OnReceive(void* p)
{
    static bool critical_section= FALSE;
    static uint8 buff[ASETK_SRV_DECODE_BUFFER_SIZE];
    static uint32 write_len = 0;
    static yahdlc_control_t control;
    tUartRxData *uartData = (tUartRxData *)p;

    if( critical_section )
    {
        ASENG_TP_PRINTF("\r\n\r\r\n*** AseNgSrv_OnReceive: re-enter error ***\r\n\r\n\r\n\n");
        ASSERT(0);
        return;
    }
    critical_section= TRUE;


    // ToDo: discard un-used data, i.e. data without enclosure of DLC start/end flag sequence @ uartRxBuf
    // ToDo: house-keeping for uartRxBuf, an approach is set a timer here, after timeout, clear the buffer
//    uint8 chhh = uartData->data;
//    if (rb_idx < 12)
//    {
//        raw_buffer[rb_idx++] = uartData->data;
//    }
//    else
//    {
//        asm("nop");
//    }

    int ret = yahdlc_get_data(&control, (char*)&uartData->data, sizeof(uint8), (char*)buff, (unsigned int*)&write_len);
    if(ret >= 0)
    {
#ifdef ASE_TK_DEBUG_SHOW_RX_DATA
        //Because we send log ASE-TK, if print control:, there is too many message
        //ASENG_TP_PRINTF("\r\ncontrol: %d %d ", control.frame, control.seq_no);
#endif
        /* do not handle the command here as it's still in interrupt call back function, send a signal instead */
        if(data_to_message(&rMessage2, (char*)buff, write_len)
            && (control.frame == YAHDLC_FRAME_DATA))
        {
            AseFepCmdEvt* evt = Q_NEW(AseFepCmdEvt, ASETK_RECEIVE_COMMAND);
            memcpy(&evt->aseFepCmd, &rMessage2, sizeof(Proto_Core_AseFepMessage));
            SendToServer(ASENG_SRV_ID,(QEvt*)evt);
            AseNgSrv_ACK(control.seq_no);
        }
        RingBuf_Reset(&rxBuf);
    }
    else if (ret == -EIO)
    {
        AseNgSrv_NACK(control.seq_no);
        /* throw the package if the CRC is not correct */
        RingBuf_Reset(&rxBuf);
    }
    else if (ret == -ENOMSG)
    {
    }
    else
    {
        ASSERT(0);
    }


    critical_section= FALSE;
}

static void AseNgSrv_EnableComWdg(cAseNgSrv * const me)
{
    /* Setup communication Watchdog timer */
    if( me->comWdgTimer==INVALID_VALUE) {
        ASENG_TP_PRINTF("\r\n AseNg Server communication watchdog is enabled. \r\n");
    }
    me->comWdgTimer = ASETK_SRV_COM_WDG_TIME_MS;
    me->aliveReqInProg = 0;
}

static void AseNgSrv_DisableComWdg(cAseNgSrv * const me)
{
    /* Setup communication Watchdog timer */
    ASENG_TP_PRINTF("\r\n AseNg Server communication watchdog is disabled. \r\n");
    me->comWdgTimer = INVALID_VALUE;
    me->aliveReqInProg = INVALID_VALUE;
}

static void AseNgSrv_FeedComWdg(cAseNgSrv * const me)
{
    if(me->aliveReqInProg == 0)
    {
        /* Restart Communication Watchdog Timer*/
        me->comWdgTimer = ASETK_SRV_COM_WDG_TIME_MS;
    }
}

static void AseNgSrv_CheckComWdgTimer(cAseNgSrv * const me)
{
    if(me->comWdgTimer > 0)
    {
        me->comWdgTimer -= ASETK_SRV_PER_TICK_TIME_MS;
        if(me->comWdgTimer <= 0)
        {/* communication watchdog timeis timeout */
            if(me->aliveReqInProg < ASETK_SRV_MAX_ALIVE_REQ)
            {
                me->comWdgTimer = ASETK_SRV_COM_WDG_TIME_MS;
                AseNgSrv_SendAliveReq();
                me->aliveReqInProg++;
            }
            else
            {
                /* If fail to get respondse from ASE-TK after 3 attempts,then publish ASE-TK
                  * communication watchdog timeout event. */
                ASENG_TP_PRINTF("\r\n Ase Server communication watchdog timeout!!\r\n");
                AseNgStateIndEvt* pAseNgStateEvt = Q_NEW(AseNgStateIndEvt, ASE_TK_STATE_SIG);
                pAseNgStateEvt->bIsComWdgTimeOut = TRUE;
                QF_PUBLISH(&pAseNgStateEvt->super, me);
            }
        }
    }
}



void AseNgSrv_SendSignalStatus(bool hasSignal)
{
    Proto_FepAse_Event_Type type = (hasSignal) ? Proto_FepAse_Event_Type_LINE_IN_SENSE_ACTIVE : Proto_FepAse_Event_Type_LINE_IN_SENSE_INACTIVE;
    AseNgSrv_SendFepAseEvent(type);
}


static void AseNgSrv_SendAliveReq(void)
{
    Proto_Core_FepAseMessage msg;

    msg.which_OneOf = Proto_Core_FepAseMessage_fepAseReq_tag;
    msg.OneOf.fepAseReq.has_type = TRUE;
    msg.OneOf.fepAseReq.type = Proto_FepAse_ReqResp_PING;
    msg.OneOf.fepAseReq.has_id = TRUE;
    msg.OneOf.fepAseReq.id = Proto_FepAse_ReqResp_PING;
    msg.OneOf.fepAseReq.which_data = 0;

    AseNgSrv_SendMessage(&msg);
}


/***************************************************/
/********Protocol buffer Encode Decode func *************/
/**************************************************/
const pb_field_t* decode_unionmessage_type(pb_istream_t *stream)
{
    pb_wire_type_t wire_type;
    uint32_t tag;
    bool eof;
    while (pb_decode_tag(stream, &wire_type, &tag, &eof))
    {
        if (wire_type == PB_WT_STRING)
        {
            const pb_field_t *field;
            for (field = Proto_Core_AseFepMessage_fields; field->tag != 0; field++)
            {
                if (field->tag == tag && (field->type & PB_LTYPE_SUBMESSAGE))
                {
                    /* Found our field. */
                    return field->ptr;
                }
            }
        }

        /* Wasn't our field.. */
        pb_skip_field(stream, wire_type);
    }

    return NULL;
}

bool decode_unionmessage_contents(pb_istream_t *stream, const pb_field_t fields[], void *dest_struct)
{
    pb_istream_t substream;
    bool status;
    if (!pb_make_string_substream(stream, &substream))
    {
        return false;
    }
    status = pb_decode(&substream, fields, dest_struct);
    pb_close_string_substream(stream, &substream);
    return status;
}

static bool data_to_message(Proto_Core_AseFepMessage* p_message, char* buff, uint32 size)
{
    bool status = false;

    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer((uint8*)buff, size);

    {
        /* Create a stream that reads from the buffer. */
        pb_istream_t stream = pb_istream_from_buffer((uint8*)buff, size);

        /* Now we are ready to decode the message. */
        status = pb_decode(&stream, Proto_Core_AseFepMessage_fields, p_message);
    }


    /* Check for errors... */
    if (!status)
    {
        uint32 ttype;
        ASENG_TP_PRINTF("Protocol_buffer: Can not analyze the protocol buffer\r\n");
        switch(p_message->which_OneOf)
        {
        case Proto_Core_AseFepMessage_aseFepEvent_tag:
          ttype = p_message->OneOf.aseFepEvent.type;
          break;
        case Proto_Core_AseFepMessage_aseFepReq_tag:
          ttype = p_message->OneOf.aseFepReq.type;
          break;
        case Proto_Core_AseFepMessage_aseFepResp_tag:
          ttype = p_message->OneOf.aseFepResp.type;
          break;
        default:
          break;
        }
        asm("nop");
        ASENG_TP_PRINTF("Pb_decode error! which_one_of: %d; type: %d; \r\n", p_message->which_OneOf, ttype);
        return FALSE;
    }
    return TRUE;
}


static uint32 message_to_data(Proto_Core_FepAseMessage* p_message, char* buff, uint32 size_of_buff)
{
    /* On FS1 v4.0.6, when send ASETK version, this function need
     *  472 (0x1D8) bytes on call stack
     */
    /* Create a stream that will write to our buffer. */
    pb_ostream_t stream = pb_ostream_from_buffer((uint8*)buff, size_of_buff);

    bool status = pb_encode(&stream, Proto_Core_FepAseMessage_fields, p_message);

    uint32 message_length = stream.bytes_written;

    /* Then just check for any errors.. */
    if (!status)
    {
        ASENG_TP_PRINTF("Protocol_buffer: Encoding failed\r\n");
        return 0;
    }
    return message_length;
}

static int yahdlc_seq_advance(int seq)
{
    const int yahdlc_seq_max = 7;   // yahdlc_control_t.seq_no with length of 3 bits

    if (yahdlc_seq_max <= seq)
    {
        seq = 0;
    }
    else
    {
        ++seq;
    }

    return seq;
}
