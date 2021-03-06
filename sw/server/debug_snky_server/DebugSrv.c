/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Debug Server
                  -------------------------

                  SW Module Document




@file        DebugSrv.c
@brief       Debug server
@author      Dmitry.Abdulov
@date        2014-04-21
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2014-04-21     Dmitry.Abdulov
DESCRIPTION: First Draft. Generated by newclass.py
SCO/ERROR  :
-------------------------------------------------------------------------------
 *  * Example using debug print out
 * \msc
 * client, server, driver;
 *              --- [label = "Client send request\command to Server"];
 * client->server   [label = "SendToServer(DEBUG_SIG, event)"];
 *              --- [label = "Server perform print output by calls Uart Driver"];
 * server=>driver   [label = "UartDrv_Write()"];
 *              --- [label = "Server send response back to Client"];
 * server->client   [label = "QACTIVE_POST()"];
 * \endmsc

*/

#include "bsp.h"
#include "trace.h"
#include "DebugSrv.h"
#include "./DebugSrv_priv.h"
#include "controller.h"
#include "crc16.h"
#include "attachedDevices.h"
#include "ringbuf.h"
#include "UartDrv.h"
#include "MainApp.h"

#ifdef HAS_KEYS
#include "KeySrv.h"
#endif

#ifdef HAS_POWER_CONTROL
#ifdef USE_POWER_SNEAKY_SERVER
#include "PowerSrvSnky.h"
#else
#include "PowerSrv.h"
#endif
#endif

#ifdef HAS_ALLPLAY
#include "AllPlaySrv.h"
#endif

#ifdef HAS_LEDS
#if defined( LED_USE_IOEXPANDER)
#include "LedSrv_IoExpander.h"
#elif defined(LED_USE_MCU_PWM_PIN)
#include "LedSrv.h"
#endif
#endif

#ifdef HAS_BLUETOOTH
#include "BluetoothSrv.h"
#endif

#ifdef HAS_AUDIO_CONTROL
#include "AudioSrv.h"
#endif

#ifdef HAS_SETTING
#include "SettingSrv.h"
#endif

Q_DEFINE_THIS_FILE

#define CAST_ME cDebugSrv * debugSrv = (cDebugSrv *) me;

/* private state functions */
static QState DebugSrv_Initial(cDebugSrv * const me, QEvt const * const e);
static QState DebugSrv_Ready(cDebugSrv * const me, QEvt const * const e);
static QState DebugSrv_DeActive(cDebugSrv * const me, QEvt const * const e);

/*____________________________________________________________________________*/
static void DebugSrvResetOutBuf( void );
static void uartRxCallback(void* p);
#ifdef ENABLE_WAKEUP_BY_UART
static void uartWakeUpCallback();
#endif
static void DebugSrvParseAndRun(cDebugSrv * const me);
static void DebugSrvResetInBuf( void );

/*static */void DebugSrvInputSourceCtor();
static void DebugSrvInputSourceXtor();
/*____________________________________________________________________________*/
static bool DebugSrvCheckCRC( const unsigned char *byte_stream, unsigned length );
static void DebugSrvHandleRespMsg(eSignal sig, uint8* pmsg);
/*____________________________________________________________________________*/

/*______________________________________________________________________________________________________________*/
/*
 *  SEQ  | SIG   | SRV ID | SIZE_MSB | SIZE_LSB | DATA0 | ... | CRC16_MSB | CRC16_LSB |
 *       |       |        |          |          |       |     |           |           |
 * ?	SEQ: (1 byte), sequence number, starts at 0 and gets incremented with each message, rolls over
 * ?	SIG(1 byte):  signal ID (one of from ?typedef enum {?}eSignal;?  please reference to ?tymphany_platform\sw\include\signals.h?)
 * ?	SRV_ID(1 byte): Target AO(active object)id(one of from ?typedef enum {?}eServerID;?  please reference to?tymphany_platform\sw\include\object_ids.h? )
 * ?	SIZE(2 bytes):  message size(size of byte stream)
 * ?	DATA(?):  encapsulated specific (according SIG)data structure
 * ?	CRC16: (2 bytes) uses CRC-CCITT (used in Xmodem, Bluetooth etc.)
 * http://en.wikipedia.org/wiki/Cyclic_redundancy_check.
 * Check your CRC here:
 * http://depa.usst.edu.cn/chenjq/www2/SDesign/JavaScript/CRCcalculation.htm
 * ex: CRC(0x01,0x02,0x03) -> 0x6131
 */
typedef enum
{
    SEQ_IDX,
    SIG_IDX,
    SRVID_IDX,
    SIZEMSB_IDX,
    SIZELSB_IDX,
    DATASTART_IDX,
}eDbgSnkyMsgIndx;

#define DMSG_MIN_SIZE   0x07

/* Internal event queue - Size as needed */
#define DBGSRV_INT_QUEUE_SIZE   5
static QEvt const *DebugEvtQue[DBGSRV_INT_QUEUE_SIZE];

#define DBG_UART_TX_BUF_SIZE ((SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE)* DBGSRV_INT_QUEUE_SIZE) + 1
#define DBG_UART_RX_BUF_SIZE (SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE) + 1
static uint8 uartTxBuf[DBG_UART_TX_BUF_SIZE];
static uint8 uartRxBuf[DBG_UART_RX_BUF_SIZE];
static cRingBuf txBuf;
static cRingBuf rxBuf;

cUartDrv  p_debug_dev;
#define CLI_INPUT_BUFFER_SIZE (SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE)
#define CLI_OUTPUT_BUFFER_SIZE (SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE)

static bool bIsAllowToWakeUp = FALSE;

static uint8 in_buf_byte_stream[CLI_INPUT_BUFFER_SIZE];
static uint8 in_buf_bs_idx = 0;
static uint8 out_buf_byte_stream[CLI_OUTPUT_BUFFER_SIZE];



static tDbgSnkyMsgEvt dmsg;

static uint16 dbg_sig_evt_size_map[] = {
#ifdef HAS_KEYS
  [KEY_DEBUG_REQ_SIG]               = sizeof(KeyDebugReqEvt),
  [KEY_DEBUG_RESP_SIG]              = sizeof(KeyDebugRespEvt),
  [KEY_STATE_SIG]                   = sizeof(KeyStateEvt),
#endif
#ifdef HAS_LEDS
  [LED_REQ_SIG]                     = sizeof(LedReqEvt),
  [LED_RESP_SIG]                    = sizeof(LedRespEvt),
#endif
#ifdef HAS_POWER_CONTROL
  [POWER_DEBUG_REQ_SIG]             = sizeof(PowerSrvDebugReqEvt),
  [POWER_DEBUG_RESP_SIG]            = sizeof(PowerSrvDebugRespEvt),
#endif
#ifdef HAS_ALLPLAY
  [ALLPLAY_STATE_SIG]               = sizeof(AllPlayStateIndEvt),
#endif
#ifdef HAS_BLUETOOTH
  [BT_REQ_SIG]                      = sizeof(BtCmdEvt),
  [BT_STATE_SIG]                    = sizeof(BtStatusEvt),
#endif
#ifdef HAS_AUDIO_CONTROL
  [AUDIO_REQ_SIG]                   = sizeof(AudioLineInjackDetecionEvt),
  [AUDIO_RESP_SIG]                  = sizeof(AudioLineInjackDetecionRespEvt),
  [AUDIO_MUSIC_DETECT_STATE_SIG]    = sizeof(AudioMusicDetectStateEvt),
  [AUDIO_STATE_SIG]                 = sizeof(AudioStateEvt),
#endif
  [DEBUG_SIG]                       = sizeof(DebugPrintEvt),
  [DEBUG_RESP_SIG]                  = sizeof(DebugSnkyResp),
  [SYSTEM_STATE_SIG]                = sizeof(SystemStatusEvt),
#ifdef HAS_SETTING
  [SETTING_START_REQ_SIG]           = sizeof(SettingStartReqEvt),
  [SETTING_START_RESP_SIG]          = sizeof(SettingStartRespEvt),
  [SETTING_DATA_REQ_SIG]            = sizeof(SettingDataReqEvt),
  [SETTING_DATA_RESP_SIG]           = sizeof(SettingDataRespEvt),
  [SETTING_END_REQ_SIG]             = sizeof(SettingEndReqEvt),
  [SETTING_END_RESP_SIG]            = sizeof(SettingEndRespEvt),
#endif
};
/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/

void DebugSrv_StartUp(cPersistantObj *me)
{
     CAST_ME;

    QTimeEvt_ctorX(&debugSrv->timeEvt, (QActive*)debugSrv, DEBUG_TIMEOUT_SIG, 0);
	/* start up the object and let it run. Called by the controller */
    Server_Ctor((cServer*)me, Q_STATE_CAST(DebugSrv_Initial));
    DebugSrvInputSourceCtor();
    /* active object start */
    QActive_start((QActive*)debugSrv, DEBUG_SRV_ID, DebugEvtQue, Q_DIM(DebugEvtQue), (void *)0, 0U, (QEvt *)0);
    /*OK, all done, let's roll.  */
}

void DebugSrv_ShutDown(cPersistantObj *me)
{
	/* Clean memory and shut-down. Called by the controller */
    Server_Xtor((cServer*)me);
    DebugSrvInputSourceXtor();
}
/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
static QState DebugSrv_Initial(cDebugSrv * const me, QEvt const * const e)
{
    /* Subscribe for need signals here */
    QActive_subscribe((QActive*) me, SYSTEM_STATE_SIG);
#ifdef HAS_KEYS
    QActive_subscribe((QActive*) me, KEY_STATE_SIG); // use the macros!
#endif
#ifdef HAS_ALLPLAY
    QActive_subscribe((QActive*) me, ALLPLAY_STATE_SIG);
#endif
#ifdef HAS_BLUETOOTH
    QActive_subscribe((QActive*) me, BT_STATE_SIG);
#endif
#ifdef HAS_AUDIO_CONTROL
    QActive_subscribe((QActive*) me, AUDIO_MUSIC_DETECT_STATE_SIG);
    QActive_subscribe((QActive*) me, AUDIO_STATE_SIG);
#endif
    return Q_TRAN(&DebugSrv_DeActive);
}


static QState DebugSrv_Ready(cDebugSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        DebugSrvResetInBuf();
        DebugSrvResetOutBuf();
        return Q_RET_HANDLED;
    }
    case CMD_READY_CALLBACK_SIG:
    { /*fill up meta-data from message which saved in input buffer */
        dmsg.seq = in_buf_byte_stream[SEQ_IDX];
        dmsg.signal = (eSignal)in_buf_byte_stream[SIG_IDX];
        dmsg.target_srv_id = (eServerID)in_buf_byte_stream[SRVID_IDX];

        if (DebugSrvCheckCRC(&in_buf_byte_stream[0], dmsg.msg_size))
        {
            DebugSrvParseAndRun(me);
        }
        else
        { //TODO: send back system signal about crc error
          DebugSnkyResp* error_resp = Q_NEW(DebugSnkyResp, DEBUG_RESP_SIG);
          error_resp->evtReturn = RET_FAIL;
          SendToServer(DEBUG_SRV_ID, (QEvt*)error_resp);
        }
            DebugSrvResetInBuf();
            DebugSrvResetOutBuf();
        return Q_RET_HANDLED;
    }
    case DEBUG_SIG:
    case DEBUG_RESP_SIG:
    case SETTING_START_REQ_SIG:
    case SETTING_START_RESP_SIG:
    case SETTING_DATA_REQ_SIG:
    case SETTING_DATA_RESP_SIG:
    case SETTING_END_REQ_SIG:
    case SETTING_END_RESP_SIG:
    case ALLPLAY_STATE_SIG:
    case POWER_DEBUG_RESP_SIG:
    case LED_RESP_SIG:
    case KEY_DEBUG_RESP_SIG:
    case KEY_STATE_SIG:
    case BT_STATE_SIG:
    case AUDIO_RESP_SIG:
    case AUDIO_MUSIC_DETECT_STATE_SIG:
    case AUDIO_STATE_SIG:
    case SYSTEM_STATE_SIG:
    {
        DebugSrvHandleRespMsg((eSignal)e->sig, (uint8*)e);
        return Q_RET_HANDLED;
    }
	case SYSTEM_SLEEP_REQ_SIG:
    {
        CommonReqEvt* pReq = (CommonReqEvt*)e;
        CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
        return Q_TRAN(&DebugSrv_DeActive);
    }
    case Q_EXIT_SIG:
    {
        return Q_RET_HANDLED;
    }
    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}

static QState DebugSrv_DeActive(cDebugSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
#ifdef ENABLE_WAKEUP_BY_UART
        bIsAllowToWakeUp = TRUE;
        UartDrv_EnableWakeUp(&p_debug_dev);
#else
        DebugSrvInputSourceXtor();
#endif
        return Q_RET_HANDLED;
    }
#ifdef ENABLE_WAKEUP_BY_UART
    case CMD_WAKEUP_SIG:
    {
        KeyStateEvt* ke = Q_NEW(KeyStateEvt, KEY_STATE_SIG);
        ke->keyEvent = KEY_EVT_SHORT_PRESS;
        ke->keyId = POWER_KEY;
        QF_PUBLISH((QEvt*)ke, me);
        return Q_RET_HANDLED;
    }
#endif
    case SYSTEM_ACTIVE_REQ_SIG:
    {
#ifdef ENABLE_WAKEUP_BY_UART
        bIsAllowToWakeUp = FALSE;
#else
        DebugSrvInputSourceCtor();
#endif
        CommonReqEvt* pReq = (CommonReqEvt*) e;
        CommonEvtResp((QActive*) me, pReq->sender, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
        return Q_TRAN(&DebugSrv_Ready);
    }
    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
/**
 * debug server input source constructor - create uart driver instance
 * create tx,rx ringbuffer instance and register uart callback function
 */
/*static */void DebugSrvInputSourceCtor()
{
    RingBuf_Ctor(&txBuf, uartTxBuf, DBG_UART_TX_BUF_SIZE);
    RingBuf_Ctor(&rxBuf, uartRxBuf, DBG_UART_RX_BUF_SIZE);
    UartDrv_Ctor(&p_debug_dev, (tUARTDevice*) getDevicebyId(DEBUG_ID, NULL), &txBuf, &rxBuf);
    UartDrv_RegisterRxCallback(&p_debug_dev, uartRxCallback);
#ifdef ENABLE_WAKEUP_BY_UART
    UartDrv_RegisterWakeUpCallback(&p_debug_dev, uartWakeUpCallback);
#endif
}
/**
 * debug server input source destructor destrot uart driver and ringbuffers objects
 * to save power, while switching into deactive state
 */
static void DebugSrvInputSourceXtor()
{
    UartDrv_Xtor(&p_debug_dev);
    RingBuf_Xtor(&txBuf);
    RingBuf_Xtor(&rxBuf);
}

static void DebugSrvResetOutBuf( void ){ memset( out_buf_byte_stream, 0, sizeof(out_buf_byte_stream) ); RingBuf_Reset(&txBuf); }

static void uartRxCallback(void* p)
{
    if (in_buf_bs_idx == (SIZELSB_IDX + 1)) //update msg size asap as we read SIZELSB_IDX bytes of msg
    {
        dmsg.msg_size = in_buf_byte_stream[SIZEMSB_IDX] << 8;
        dmsg.msg_size |= in_buf_byte_stream[SIZELSB_IDX];
    }
    if (in_buf_bs_idx <= dmsg.msg_size)
    {
        in_buf_byte_stream[in_buf_bs_idx] = RingBuf_PopData(&rxBuf);
        in_buf_bs_idx++;
        if (in_buf_bs_idx == dmsg.msg_size)
        { /*we got dmsg.msg_size bytes of msg - ready to unpack and forward the payload*/
            DebugCmdEvt* cmdEvt = Q_NEW(DebugCmdEvt, CMD_READY_CALLBACK_SIG);
            SendToServer(DEBUG_SRV_ID, (QEvt*) cmdEvt);
        }
    }
}
#ifdef ENABLE_WAKEUP_BY_UART
static void uartWakeUpCallback()
{
  if (bIsAllowToWakeUp)
  {
       DebugSrvResetInBuf();
       DebugSrvResetOutBuf();
       DebugCmdEvt* cmdEvt = Q_NEW(DebugCmdEvt, CMD_WAKEUP_SIG);
       SendToServer(DEBUG_SRV_ID, (QEvt*) cmdEvt);
       bIsAllowToWakeUp = FALSE;
  }
}
#endif
static void DebugSrvParseAndRun(cDebugSrv * const me)
{
    uint8* ptr_src;
    uint8* ptr_dest;
    QEvt* piratube;
    DebugSrvResetOutBuf();

    ptr_src = (uint8*)&in_buf_byte_stream[DATASTART_IDX];
    ptr_src += sizeof(QEvt); // shift to avoid copy useless header
    piratube = (QEvt *)QF_newX_((QEvtSize)(dmsg.msg_size - DMSG_MIN_SIZE), (uint16_t)0,  dmsg.signal);
    ptr_dest = (uint8*)piratube;
    ptr_dest += sizeof(QEvt); // shift to avoid overwrite header
    memcpy(ptr_dest, ptr_src, dmsg.msg_size - DMSG_MIN_SIZE - sizeof(QEvt)); // fill up data evt
    if (MAX_PUB_SIG > dmsg.signal) /* publish signal*/
    {
        QF_PUBLISH(piratube, me);
    }
    else if((MAX_PUB_SIG < dmsg.signal) && (MAX_RESP_SIG > dmsg.signal)) /* resp signal*/
         {
        SendToServer(dmsg.target_srv_id, piratube);
    }
         else /*req signal*/
    {
            memcpy (ptr_dest, (uint8*)&me, sizeof(QActive*)); //fill sender field with QActive pointer to DebugSrv
            SendToServer(dmsg.target_srv_id, piratube);
    }
    DebugSrvResetInBuf();
}

static void DebugSrvResetInBuf( void )
{
    memset (in_buf_byte_stream, 0, sizeof(in_buf_byte_stream));
    in_buf_bs_idx = 0;
    dmsg.msg_size = DMSG_MIN_SIZE;
    RingBuf_Reset(&rxBuf);
}

static bool DebugSrvCheckCRC( const unsigned char *byte_stream, unsigned length )
{
    unsigned short calc_crc = crc16( byte_stream, length-2 );
    unsigned short rec_crc = (((unsigned short)byte_stream[length-2])<<8) | byte_stream[length-1] ;
    if( rec_crc==calc_crc )
    {
        return TRUE;
    }
    return FALSE;
}

static unsigned char get_msb( unsigned short x ){ return (x>>8)&0xFF; }

static unsigned char get_lsb( unsigned short x ){ return x&0xFF; }

static void calc_crc_msb_lsb( const unsigned char *byte_stream, unsigned length, unsigned char *msb, unsigned char *lsb )
{
    unsigned short crc = crc16( byte_stream, length );
    *msb = get_msb(crc);
    *lsb = get_lsb(crc);
}
/* pack message with size and CRC */
static void seal_msg(uint16 bytestream_size)
{
    uint8 crc_msb, crc_lsb;
    out_buf_byte_stream[SIZEMSB_IDX] = (bytestream_size & 0xFF00) >> 8;
    out_buf_byte_stream[SIZELSB_IDX] = (bytestream_size & 0x00FF);

    calc_crc_msb_lsb( out_buf_byte_stream, bytestream_size-2, &crc_msb, &crc_lsb );

    out_buf_byte_stream[bytestream_size-2] = crc_msb;
    out_buf_byte_stream[bytestream_size-1] = crc_lsb;

}

static void DebugSrvHandleRespMsg(eSignal sig, uint8* pmsg)
{
    uint16 size = dbg_sig_evt_size_map[sig];
    if (size)
    {
        out_buf_byte_stream[SEQ_IDX] = ++dmsg.seq;
        out_buf_byte_stream[SIG_IDX] = sig;
        out_buf_byte_stream[SRVID_IDX] = 0; //ignored in resp msg to pc client

        memcpy(&out_buf_byte_stream[DATASTART_IDX], pmsg, size);

        seal_msg(size + DMSG_MIN_SIZE);

        UartDrv_Write(&p_debug_dev, &out_buf_byte_stream[0], size + DMSG_MIN_SIZE);
    }
}

void DebugSrvPrintStr(char* pMsg)
{
    uint8 size = strlen(pMsg);
    if (size > MAX_PRINTSTR_SIZE) { size = MAX_PRINTSTR_SIZE;  }

    DebugPrintEvt* print_msg = Q_NEW(DebugPrintEvt, DEBUG_SIG);
    memcpy(&print_msg->msg[0], pMsg, size);
    print_msg->size = size;
    print_msg->sender = NULL;
    SendToServer(DEBUG_SRV_ID, (QEvt*)print_msg);
}
