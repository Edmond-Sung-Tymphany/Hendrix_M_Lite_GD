/**
 * @file        PowerDrvNew.h
 * @brief       New Power related driver implementation on STM32
 * @author      Wesley Lee, Johnny Fan 
 * @date        2014-07-16
 * @copyright   Tymphany Ltd.
 */

#ifndef POWERDRV_H
#define POWERDRV_H

#ifdef __cplusplus
extern "C" {
#endif
#include "cplus.h"
#include "attachedDevices.h"
#include "PowerSrv.h"
#include "product.config"

#ifndef NDEBUG
extern bool dbg_print_batt_ena;
#endif

// For a sequence of operation, the pointer of function for each sub-task without the delay
typedef void (*seqFunc)(void);


#define ON_STATE      (1)
#define OFF_STATE     (0)

typedef struct tPowerResetStage{
    seqFunc resetFunc;
    uint16 delaytime;
} tPowerResetStage;

CLASS(cPowerDrv)
    uint8 step;
#ifdef EXTERNAL_BATTERY
    int8 ejectBatteryStep;
#endif
    /* private data */
METHODS
    /* public functions */
void    PowerDrv_Ctor(cPowerDrv *me);
void    PowerDrv_Xtor(cPowerDrv *me);



uint16  PowerDrv_InitialPower(cPowerDrv *me);
void    PowerDrv_DeinitialPower(cPowerDrv *me);
#ifdef HAS_AWAKE_WITHOUT_BT
uint16  PowerDrv_AwakePower(cPowerDrv *me);
#endif

#ifdef HAS_HW_VERSION_TAG
bool  PowerDrv_GetHwVersionRawVoltage(cPowerDrv *me, int16 *rawVoltage);
#endif

#ifdef IS_POWER_SWITCH_ON
bool    PowerDrv_IsMainSwitchOn();
#endif
void  PowerDrv_HandleDcDetInterrupt();

#ifdef HAS_BATTERY
#ifdef EXTERNAL_BATTERY
int32 PowerDrv_EjectExtBattery(cPowerDrv *me);
void  PowerDrv_HandleExtBattDetInterrupt();
#endif
void    PowerDrv_RegisterIntEvent(QActive* pRequestor);
void    PowerDrv_UnRegisterIntEvent();
void    PowerDrv_SetExtraCommand(cPowerDrv *me, eExtraCommand extraCommand);
void    PowerDrv_SetInputSource(cPowerDrv *me, eInputSource inputSource);
void    PowerDrv_GetInputSourceState(cPowerDrv *me, tInputSourceState* inputSourceState);
bool    PowerDrv_GetBatteryVol(cPowerDrv *me, tBatteryVol* batteryVol);
void    PowerDrv_SetBatterySource(cPowerDrv *me, eBatterySource batterySource);
void    PowerDrv_PowerSaveSleep();
#endif //HAS_BATTERY

bool PowerDrv_IsHwVerCorrect(const char * hwVerString);

bool PowerDrv_IsHwSupported(const char * hwVerString);


END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* POWERDRV_H */

