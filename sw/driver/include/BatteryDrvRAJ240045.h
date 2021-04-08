/**
* @file BatteryDrvRAJ240045.h
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 16-Dec-2015
* @copyright Tymphany Ltd.
*/

#ifndef BATTERY_DRV_RAJ240045_H
#define BATTERY_DRV_RAJ240045_H

#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "SettingSrv.h"

#define RAJ240045_REG_ADDR_LEN          (1)  // register addr size: 1byte
#define RAJ240045_REG_DATA_LEN          (2)  // register data size: 2byte

#define RAJ240045_FW_VER_ADDR           (0x00)
#define RAJ240045_AVG_TEMP_ADDR         (0x08) /* average temperature */
#define RAJ240045_TOTAL_VOL_ADDR        (0x09) /* total voltage */
#define RAJ240045_CURRENT_ADDR          (0x0A) /* current */
#define RAJ240045_AVG_CURRENT_ADDR      (0x0B) /* average current = 33.3% * current + 66.6% * previous current (get current sample per 250ms */
#define RAJ240045_RELATIVE_SOC_ADDR     (0x0D) /* relative state of charge*/
#define RAJ240045_ABSOLUTE_SOC_ADDR     (0x0E) /* absolute state of charge*/
#define RAJ240045_REMAINING_CAP_ADDR    (0x0F) /* Remaining capacity */
#define RAJ240045_FULL_CHARGE_CAP_ADDR  (0x10) /* full charge capacity */
#define RAJ240045_CHARGE_CURRENT_ADDR   (0x14) /* charging current */
#define RAJ240045_CHARGE_VOL_ADDR       (0x15) /* charging voltage */
#define RAJ240045_BATT_STATUS_ADDR      (0x16) /* battery status */
#define RAJ240045_CYCLE_COUNT_ADDR      (0x17) /* cycle count */
#define RAJ240045_DESIGN_CAP_ADDR       (0x18) /* design capacity */
#define RAJ240045_DESIGN_VOL_ADDR       (0x19) /* design voltage */
#define RAJ240045_SERIAL_NUMBER_ADDR    (0x1C) /* serial number */
#define RAJ240045_HW_VER_ADDR           (0x20) /* Hardware Version */
#define RAJ240045_CELL_VOLT_1_ADDR      (0x3F) /* Cell 1 voltage */
#define RAJ240045_CELL_VOLT_2_ADDR      (0x3E) /* Cell 2 voltage */
#define RAJ240045_CELL_TEMP_1_ADDR      (0x4A) /* Cell 1 temperature */
#define RAJ240045_CELL_TEMP_2_ADDR      (0x4B) /* Cell 2 temperature */
#define RAJ240045_PACK_STATUS_ADDR      (0x41) /* pack status: include WDOG status */
#define RAJ240045_SAFETY_STATUS_ADDR    (0x51) /* safety status */
#define RAJ240045_PF_STATUS_ADDR        (0x53) /* PF status */

#define RAJ240045_SOH_ADDR              (0x60) /* State of Health */

#define RAJ240045_SHUT_DOWN_ADDR        (0x48)

/* BatteryStatus: (0x16) */
//Status data format
#define RAJ240045_STATUS_BIT_INIT           (0x0080)   //charger plug-in
#define RAJ240045_STATUS_BIT_DISCHARGE      (0x0040)   //discharge
#define RAJ240045_STATUS_BIT_FULL_CHARGE    (0x0020)   //full charger

/* SafetyStatus: (0x51) */
#define RAJ240045_SAFETY_STATUS_DOT_DUT     (0x8000)   // Discharge Over Temperature / Discharge Under Temperature
#define RAJ240045_SAFETY_STATUS_COT_CUT     (0x4000)   // Charge Over Temperature / Charge Under Temperature
#define RAJ240045_SAFETY_STATUS_SW_OD       (0x2000)   // Over current Discharge (SW)
#define RAJ240045_SAFETY_STATUS_SW_OC       (0x1000)   // Over Charge (SW)
#define RAJ240045_SAFETY_STATUS_HW_OD       (0x0800)   // Over current Discharge (HW)
#define RAJ240045_SAFETY_STATUS_UNDER_VOLT  (0x0080)   // Under Voltage
#define RAJ240045_SAFETY_STATUS_OVER_VOLT   (0x0040)   // Over Voltage
#define RAJ240045_SAFETY_STATUS_PF          (0x0020)   // Permanent Failures
#define RAJ240045_SAFETY_STATUS_SHORT       (0x0001)   // Short Circuit

/* Fault Log*/
#define RAJ240045_FAULTS_LOG_OVP_CNT            (0x0)
#define RAJ240045_FAULTS_LOG_UVP_CNT            (0x1)
#define RAJ240045_FAULTS_LOG_COT_CNT            (0x2)
#define RAJ240045_FAULTS_LOG_CUT_CNT            (0x3)
#define RAJ240045_FAULTS_LOG_DOT_CNT            (0x4)
#define RAJ240045_FAULTS_LOG_DUT_CNT            (0x5)
#define RAJ240045_FAULTS_LOG_AFEE_CNT           (0x6)
#define RAJ240045_FAULTS_LOG_DOC_CNT            (0x8)
#define RAJ240045_FAULTS_LOG_COC_CNT            (0x9)
#define RAJ240045_FAULTS_LOG_CHRG_AUTH_FAIL_CNT (0xA)
#define RAJ240045_FAULTS_LOG_CHRG_OVP_CNT       (0xB)
#define RAJ240045_FAULTS_LOG_CHRG_COT_CNT       (0xC)
#define RAJ240045_FAULTS_LOG_ADC_SAT_FAIL_CNT   (0xD)
#define RAJ240045_FAULTS_LOG_AFECF_COUNT        (0xE)
#define RAJ240045_FAULTS_LOG_HWDOC_COUNT        (0x20)
#define RAJ240045_FAULTS_LOG_HWCOC_COUNT        (0x21)
#define RAJ240045_FAULTS_LOG_HWBCD_COUNT        (0x22)
#define RAJ240045_FAULTS_LOG_HWSCP_COUNT        (0x23)
#define RAJ240045_FAULTS_LOG_PFCBF_COUNT        (0x10)
#define RAJ240045_FAULTS_LOG_PFOVP_COUNT        (0x12)
#define RAJ240045_FAULTS_LOG_PFCOC_COUNT        (0x13)
#define RAJ240045_FAULTS_LOG_PFFETF_COUNT       (0x14)
#define RAJ240045_FAULTS_LOG_PFUVP              (0x16)
#define RAJ240045_FAULTS_LOG_MAX_CELL_VOLT      (0x100)
#define RAJ240045_FAULTS_LOG_MIN_CELL_VOLT      (0x101)
#define RAJ240045_FAULTS_LOG_MAX_CHRG_TEMP      (0x102)
#define RAJ240045_FAULTS_LOG_MIN_CHRG_TEMP      (0x103)
#define RAJ240045_FAULTS_LOG_MAX_DISCHRG_TEMP   (0x104)
#define RAJ240045_FAULTS_LOG_MIN_DISCHRG_TEMP   (0x105)
#define RAJ240045_FAULTS_LOG_MAX_CHRG_CURR      (0x106)
#define RAJ240045_FAULTS_LOG_MAX_DISCHRG_CURR   (0x108)
#define RAJ240045_FAULTS_LOG_BATTERY_STATUS     (0x110)
#define RAJ240045_FAULTS_LOG_AVG_CURRENT        (0x111)
#define RAJ240045_FAULTS_LOG_TEMP1              (0x112)
#define RAJ240045_FAULTS_LOG_TEMP2              (0x112)
#define RAJ240045_FAULTS_LOG_MOS_TEMP1          (0x114)
#define RAJ240045_FAULTS_LOG_CELL_VOLT1         (0x120)
#define RAJ240045_FAULTS_LOG_CELL_VOLT2         (0x121)

#define RAJ240045_FAULTS_LOG_NUM                (38)

/* PackStatus: (0x41) */
#define RAJ240045_PACK_STATUS_WDOG          (0x0040)   // wdog triggered, will clear after battery MCU reboot


typedef struct _tBattRegData {
    uint8      regAddr;
    uint8      valueLength;
    eSettingId settingId;
    uint16     regValue;
    bool       valid;
} tBattRegData;


CLASS(cBatteryDrv)
    uint8       deviceI2cAddr;
    tI2CDevice  *pI2CConfig;
    cI2CDrv     * pI2cObj;
    bool        isCreated;
METHODS

void BatteryDrv_Ctor(cBatteryDrv * me, cI2CDrv *pI2cObj);
void BatteryDrv_Xtor(cBatteryDrv * me);

eTpRet BatteryDrv_readRegValue(cBatteryDrv * me, tBattRegData *pBattData);
void BatteryDrv_shutDown(cBatteryDrv * me);

#endif //#ifndef BATTERY_DRV_RAJ240045_H
