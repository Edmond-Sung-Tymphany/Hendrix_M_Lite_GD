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
#include "seq.h"

//#define INVERT_ENABLE

#ifdef INVERT_ENABLE
#define SET_GPIO(x,y)  GpioDrv_ClearBit((x),(y))
#define CLEAR_GPIO(x,y) GpioDrv_SetBit((x),(y))
#else
#define SET_GPIO(x,y)  GpioDrv_SetBit((x),(y))
#define CLEAR_GPIO(x,y) GpioDrv_ClearBit((x),(y))
#endif

#define     BT_PWR_ENABLE(x)         GpioDrv_SetBit(&(x),BT_PWR_EN)
#define     BT_PWR_DISABLE(x)        GpioDrv_ClearBit(&(x),BT_PWR_EN)

#define     BT_RESET_ENABLE(x)         GpioDrv_ClearBit(&(x),BT_RESET_PIN)
#define     BT_RESET_DISABLE(x)        GpioDrv_SetBit(&(x),BT_RESET_PIN)

typedef enum
{
    BT_SINGLE_PRESS = 0,
    BT_LONG_PRESS,
    BT_VLONG_PRESS,
    BT_VVLONG_PRESS,
    BT_DOUBLE_PRESS,
    BT_TRIPLE_PRESS,
    BT_HOLD_HIGH,
    BT_HOLD_LOW,
    BT_MAX_PRESS,
} eBtGpioPressType;

typedef struct
{
    uint8 maxStep;
    uint32 onTime;
    uint32 internalTime;
} tBtGpioPressConfig;


typedef struct
{
    eGPIOId gpio;
    eBtGpioPressType pressType;
} tBtCmdstruct;

#define INITIAL_STEP 0

/* private functions / data */
static void EXTI_Config(bool isTurnOnInterrupt);
static void BluetoothDrv_ReadBtLedStatus(eGPIOId btInputId);

static uint8 BluetoothDrv_GetBTModuleStatus(uint32 pulseTime, eGPIOId ledId);
static void BluetoothDrv_SendStatusToSever(cBluetoothDrv *me, uint8 btStatus);
static void BluetoothDrv_SendCmdDoneToServer(cBluetoothDrv *me, eBtCmd cmd);

#ifdef HAS_BT_SEQ_CONTROL
static void BtDrv_PwrOnStage1(void* me);
static void BtDrv_PwrOnStage2(void* me);
static void BtDrv_PwrOnStage3(void* me);
static void BtDrv_PwrOnStage4(void* me);


static void BtDrv_PwrOffStage1(void* me);
static void BtDrv_PwrOffStage2(void* me);
static void BtDrv_PwrOffStage3(void* me);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_DRIVER_PRIVATE_H */
