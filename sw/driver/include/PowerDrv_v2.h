/**
 * @file        PowerDrv_v2.h
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
#include "signals.h"
#include "product.config"
#include "PowerDrv.config"

/* the DC_IN status*/
typedef enum
{
    /* DC < 11.5 or DC off */
    DC_IN_STA_OFF = 0,
    /* DC > 11.5*/
    DC_IN_STA_ON,
    DC_IN_STA_MAX
} eDcInSta;

/* the charging status*/
typedef enum
{
    /* battery charging is done*/
    CHARGER_STA_CHARGING_DONE = 0,
    /* battery is charging */
    CHARGER_STA_CHARGING,
    /* charger error stat: broken battery or no battery*/
    CHARGER_STA_ERROR,
    CHARGER_STA_MAX
} eChargerState;

typedef enum _eBatteryInfoId
{
//    BATTERY_INFO_ID_FW_VER = 0, // 0x100, means v1.0
//    BATTERY_INFO_ID_HW_VER,
    BATTERY_INFO_ID_MANUFACTURER_NAME = 0,
    BATTERY_INFO_ID_MANUFACTURER_DATE,
    BATTERY_INFO_ID_DESIGN_CAP,
    BATTERY_INFO_ID_DESIGN_VOL,
    BATTERY_INFO_ID_SERIAL,
    BATTERY_IFNO_ID_CONST_REGION_END,

    BATTERY_INFO_ID_TOTAL_VOL = BATTERY_IFNO_ID_CONST_REGION_END,
    BATTERY_INFO_ID_CURRENT,
    BATTERY_INFO_ID_ASOC,
    BATTERY_INFO_ID_REMAINING_CAP,
    BATTERY_INFO_ID_FULL_CH_CAP,
    BATTERY_INFO_ID_CHARGE_CURRENT,
    BATTERY_INFO_ID_CHARGE_VOL,
    BATTERY_INFO_ID_STATUS,
    BATTERY_INFO_ID_CYCLE,
    BATTERY_INFO_ID_CELL_VOLT_1,
    BATTERY_INFO_ID_CELL_VOLT_2,
    BATTERY_INFO_ID_DISCARDABLE_REGION_END,

    //BATTERY_INFO_ID_AVG_TEMP = BATTERY_INFO_ID_DISCARDABLE_REGION_END,
    BATTERY_INFO_ID_CELL_TEMP_1 = BATTERY_INFO_ID_DISCARDABLE_REGION_END,
    BATTERY_INFO_ID_CELL_TEMP_2,
    BATTERY_INFO_ID_AVG_CURRENT,
    BATTERY_INFO_ID_RSOC,
    BATTERY_INFO_ID_SAFETY_HIGH,
    BATTERY_INFO_ID_SAFETY_LOW,
    BATTERY_INFO_ID_SOH,
//    BATTERY_INFO_ID_PACK_STATUS,
    BATTERY_INFO_ID_PF_STATUS_HIGH,
    BATTERY_INFO_ID_PF_STATUS_LOW,

    BATTERY_INFO_ID_MAX
} eBatteryInfoId;

typedef enum _eBattHealth
{
    BATT_HEALTH_UNKNOWN = 0,
    BATT_HEALTH_CRITICAL,
    BATT_HEALTH_POOR,
    BATT_HEALTH_GOOD,
} eBattHealth;

typedef enum _eHwVer
{
    HW_TYPE_MP= 0,
    HW_TYPE_PVT,
    HW_TYPE_DVT2,
    HW_TYPE_DVT1,
    HW_TYPE_EVT2,
    HW_TYPE_EVT1,
    HW_TYPE_ES3,
    HW_TYPE_ES2,
    HW_TYPE_ES1,
    HW_TYPE_PRE_ES,
    HW_TYPE_NUM
} eHwVer;


// For a sequence of operation, the pointer of function for each sub-task without the delay
typedef void (*seqFunc)(void);

#define ON_STATE      (1)
#define OFF_STATE     (0)

typedef struct tPowerResetStage
{
    seqFunc resetFunc;
    uint16 delaytime;
} tPowerResetStage;

IND_EVT(PowerDrvTempAlarmEvt)
uint8 tempLevel;
END_IND_EVT(PowerDrvTempAlarmEvt)

CLASS(cPowerDrv)
uint32 timer;
uint32 period;
uint32 timetick;
eBatteryInfoId curId;
uint8 step;
#ifdef EXTERNAL_BATTERY
int8 ejectBatteryStep;
#endif
/* private data */
METHODS
/* public functions */
void    PowerDrv_Ctor(cPowerDrv *me);
void    PowerDrv_Xtor(cPowerDrv *me);
void    PowerDrv_EnterSleepMode();
void    PowerDrv_OverheatHandling(cPowerDrv *me, bool overheat2);
void    PowerDrv_Update(cPowerDrv *me);
uint16  PowerDrv_InitialPower(cPowerDrv *me);
void    PowerDrv_DeinitialPower(cPowerDrv *me);
void    PowerDrv_UpdateTempLevel(int16 tempNew, int16 *pTemp, eTempLevel *pLevel, const sRange *levels, uint32 numLevel);
void    PowerDrv_Get(cPowerDrv *me, ePowerGetId getId);
void    PowerDrv_Set(cPowerDrv *me, ePowerSetId setId, bool enable);
void    PowerDrv_WFMute(void);
void    PowerDrv_WFUnMute(void);
void    PowerDrv_TWUnMute(void);
void    PowerDrv_TWMute(void);
uint8   TW_READ_FAULT(void);
uint8   WF_READ_FAULT(void);

#define PowerDrv_SoftReboot()
#define PowerDrv_JumpToBootloader()

END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* POWERDRV_H */

