/**
 * @file        BluetoothDrv_priv.h
 * @brief       It's the bluetooth driver to control the ROM base CSR module by toggling GPIO
 * @author      Johnny Fan
 * @date        2014-05-13
 * @copyright   Tymphany Ltd.
 */
#ifndef BLUETOOTH_DRIVER_PRIVATE_H
#define BLUETOOTH_DRIVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "BluetoothDrv.h"
#include "deviceTypes.h"

//#define INVERT_ENABLE

#ifdef INVERT_ENABLE
#define SET_GPIO(x,y)  GpioDrv_ClearBit((x),(y))
#define CLEAR_GPIO(x,y) GpioDrv_SetBit((x),(y))
#else
#define SET_GPIO(x,y)  GpioDrv_SetBit((x),(y))
#define CLEAR_GPIO(x,y) GpioDrv_ClearBit((x),(y))
#endif

#define     BT_3V3_ENABLE(x)         GpioDrv_ClearBit(&(x),BT_3V3_EN)
#define     BT_3V3_DISABLE(x)        GpioDrv_SetBit(&(x),BT_3V3_EN)

#define     BT_RESET_ENABLE(x)         GpioDrv_ClearBit(&(x),BT_RESET_PIN)
#define     BT_RESET_DISABLE(x)        GpioDrv_SetBit(&(x),BT_RESET_PIN)

static void BluetoothDrv_uartSend(uint8 *data, uint16 size);
static void BtDrv_SendReq(Proto_McuBt_ReqResp type);
static void BtDrv_SendBtMcuEvent(Proto_BtMcu_Event_Type type);


static void BtDrv_PwrOnStage1(void* me);
static void BtDrv_PwrOnStage2(void* me);
static void BtDrv_PwrOnStage3(void* me);
static void BtDrv_PwrOnStage4(void* me);
static void BtDrv_PwrOffStage1(void* me);
static void BtDrv_PwrOffStage2(void* me);
static void BtDrv_PwrOffStage3(void* me);

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_DRIVER_PRIVATE_H */
