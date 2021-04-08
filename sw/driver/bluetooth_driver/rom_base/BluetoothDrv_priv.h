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

#include  "projBsp.h"

#include "BluetoothDrv.h"

//#define INVERT_ENABLE 

#ifdef INVERT_ENABLE
#define SET_GPIO(x,y)  GpioDrv_ClearBit((x),(y))
#define CLEAR_GPIO(x,y) GpioDrv_SetBit((x),(y))
#else 
#define SET_GPIO(x,y)  GpioDrv_SetBit((x),(y))
#define CLEAR_GPIO(x,y) GpioDrv_ClearBit((x),(y))
#endif  

/* BT power enable */
#define     BT_PWR_EN                GPIO_10
#define     BT_PWR_ENABLE(x)         GpioDrv_SetBit(&(x),BT_PWR_EN)
#define     BT_PWR_DISABLE(x)        GpioDrv_ClearBit(&(x),BT_PWR_EN)

typedef enum
{
    BT_DRV_STATUS_LINES_STATE_0 = 0,
    BT_DRV_STATUS_LINES_STATE_1 = 1,
    BT_DRV_STATUS_LINES_STATE_2 = 2,
    BT_DRV_STATUS_LINES_STATE_3 = 3,
}eBtDrvStatusLinesState;

typedef enum
{
    BT_SINGLE_PRESS = 0,    
    BT_LONG_PRESS,
    BT_VLONG_PRESS,
    BT_VVLONG_PRESS,
    BT_DOUBLE_PRESS,
    BT_MAX_PRESS,
}eBtGpioPressType;

typedef struct
{
    uint8 maxStep;
    uint32 onTime;
    uint32 internalTime;
}tBtGpioPressConfig;

typedef struct _bt_cmd_queue
{
    eBtCmd cmd;
    uint32 btStateMask;
    int32 deadLoopTimeOut;
} s_bt_cmd_queue;

typedef struct 
{
    eGPIOId gpio;
    eBtGpioPressType pressType;
    uint32 busyPeriodAfter;
}tBtCmdstruct;

void btSetBtStatus(cBluetoothDrv* me, eBtStatus status);
void btSetIsBusy  (cBluetoothDrv* me, bool state);

void btCmdPulseTmrCb(void* pCbPara);
void btCmdRelaxTmrCb(void* pCbPara);
void btCmdStartRecTmr(uint32 timeOut);
void btCmdQueueAdd(eBtCmd cmd, uint32 btStateMask, uint32 deadLoopTimeoOut);
void btCmdGo(cBluetoothDrv *me, eBtCmd cmd);
void btCmdTimeUp(cBluetoothDrv *me);
bool btCmdQueueGo();

void btStatStartDebTmr(uint32 timeOut);

#define INITIAL_STEP 0

/* private functions / data */
static void BluetoothDrv_SendStatusToSever(cBluetoothDrv *me, eBtStatus btStatus);
static void BluetoothDrv_SendCmdDoneToServer(cBluetoothDrv *me, eBtCmd cmd);

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_DRIVER_PRIVATE_H */
