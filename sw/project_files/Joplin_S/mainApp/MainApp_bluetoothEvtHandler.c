
/**
*  @file      MainApp_BluetoothEvtHandler.c
*  @brief     Bluetooth event handler of mainApp
*  @author    Colin Chen
*  @date      16-Nov.-2017
*  @copyright Tymphany Ltd.
*/

#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "trace.h"
#include "BluetoothSrv.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "tym_qp_lib.h"
#include "MainApp_priv.h"
#include "tym.pb.h"
#include "BluetoothSrv.h"

#define MAINAPP_BT_DEBUG_ENABLE
#ifndef MAINAPP_BT_DEBUG_ENABLE
#undef  TP_PRINTF
#define TP_PRINTF(...)
#endif


static void MainApp_BtMcuEventHandler(cMainApp * const me, Proto_BtMcu_Event *pBtMcuEvent)
{
    switch (pBtMcuEvent->type)
    {
        case Proto_BtMcu_Event_Type_BT_PAIRING_ENABLED:
        {
            TP_PRINTF("BtMcuEvent: enabled pairing \r\n");
            if (me->audioSource == AUDIO_SOURCE_BT)
            {
                MainApp_SendLedReq(me, LED_IND_ID_BT_PAIRING_ENABLED);
            }
            break;
        }

        case Proto_BtMcu_Event_Type_BT_A2DP_CONNECTED:
        {
            TP_PRINTF("BtMcuEvent: a2dp connected \r\n");
            if (me->audioSource == AUDIO_SOURCE_BT)
            {
                MainApp_SendLedReq(me, LED_IND_ID_BT_MODE);
            }
            break;
        }
        case Proto_BtMcu_Event_Type_BT_A2DP_DISCONNECTED:
        {
            TP_PRINTF("BtMcuEvent: a2dp disconnected \r\n");
            break;
        }
        case Proto_BtMcu_Event_Type_BT_CONNECTION_STATE:
        {
            TP_PRINTF("BtMcuEvent: bt connection state: %d \r\n", pBtMcuEvent->OneOf.connState);
            break;
        }
        default:
            TP_PRINTF("BtMcuEvent: %d=unknown\r\n", pBtMcuEvent->type);
            break;
    }
}

static void MainApp_BtMcuReqHandler(cMainApp * const me, Proto_BtMcu_Req *pBtMcuReq)
{
    switch (pBtMcuReq->type)
    {
        case Proto_BtMcu_ReqResp_MCU_APPLICATION_IS_RUNNING:
        {
            TP_PRINTF("btMcuReq: communication heartbeat  \r\n");
            break;
        }

        default:
            TP_PRINTF("btMcuReq: %d=unknown\r\n", pBtMcuReq->type);
            break;
    } 
}

static void MainApp_BtMcuRespHandler(cMainApp * const me, Proto_BtMcu_Resp *pBtMcuResp)
{
    switch (pBtMcuResp->type)
    {
        case Proto_McuBt_ReqResp_BT_PAIRING_ON:
        {
            TP_PRINTF("btMcuReq: communication heartbeat  \r\n");
            break;
        }

        default:
            TP_PRINTF("btMcuReq: %d=unknown\r\n", pBtMcuResp->type);
            break;
    } 
}

QState MainApp_BluetoothEvtHandler(cMainApp * const me, QEvt const * const e)
{
    Proto_Tym_BtMcuMessage *pMessage = (Proto_Tym_BtMcuMessage *)((BtRxEvt *)e)->pData;
    switch (pMessage->which_OneOf)
    {
        case Proto_Tym_BtMcuMessage_btMcuEvent_tag:
            MainApp_BtMcuEventHandler(me, &(pMessage->OneOf.btMcuEvent));
            break;
        case Proto_Tym_BtMcuMessage_btMcuReq_tag:
            MainApp_BtMcuReqHandler(me, &(pMessage->OneOf.btMcuReq));
            break;
        case Proto_Tym_BtMcuMessage_btMcuResp_tag:
            MainApp_BtMcuRespHandler(me, &(pMessage->OneOf.btMcuResp));
            break;
        default:
            //TP_PRINTF("Unknown BtMcuMessage: which_OneOf=%d\r\n", pMessage->which_OneOf);
            break;
    }

    return Q_UNHANDLED(); //do not transit state
}


