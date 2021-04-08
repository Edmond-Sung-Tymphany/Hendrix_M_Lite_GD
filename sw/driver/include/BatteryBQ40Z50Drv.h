/**
*  @file      BatteryBQ40Z50Drv.h
*  @brief     Public header file for  BatteryBQ40Z50Drv
*  @version   v0.1
*  @author    Alex.Li
*  @date      2017/8/28
*  @copyright Tymphany Ltd.
*/

#ifndef BATTERYBQ40Z50DRV_H
#define BATTERYBQ40Z50DRV_H

#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "I2CDrv.h"
#include "SettingSrv.h"

//Should put cmd defination in priv.h, we move it to here to suit old power driver
//TODO: Other driver/server should get battery info through public Get functions
/*  ManufacturerAccess()  */
#define BQ40Z50_MAC_CMD                  (0x00)  
#define BQ40Z50_MAC_SHUTDOWN             (0x0010)//Shut down mode       
                                                                        
#define BQ40Z50_TEMP_CMD                 (0x08)//Temperature()          
#define BQ40Z50_VOLTAGE_CMD              (0X09)//Voltage()              
#define BQ40Z50_CURRENT_CMD              (0x0A)//Current()              
#define BQ40Z50_AVRCURRENT_CMD           (0x0B)//AverageCurrent()       
#define BQ40Z50_RSCHARGE_CMD             (0x0D)//RelativeStateOfCharge()
#define BQ40Z50_ASCHARGE_CMD             (0x0E)//AbsoluteStateOfCharge()
#define BQ40Z50_REMAINCAPACITY_CMD       (0x0F)//RemainingCapacity()    
#define BQ40Z50_FULLCHARGECAPACITY_CMD   (0x10)//FullChargeCapacity()   
#define BQ40Z50_CHARGINGCURRENT_CMD      (0x14)//ChargingCurrent()
#define BQ40Z50_CHARGINGVOLTAGE_CMD      (0x15)//ChargingVoltage()
#define BQ40Z50_BATTERYSTATUS_CMD        (0x16)//BatteryStatus()        
#define BQ40Z50_CYCLECOUNT_CMD           (0x17)//CycleCount()           
#define BQ40Z50_DESIGNCAPACITY_CMD       (0x18)//DesignCapacity()       
#define BQ40Z50_DESIGNVOLTAGE_CMD        (0x19)//DesignVoltage()
#define BQ40Z50_MANUFACTURERDATE_CMD     (0x1B)//ManufacturerDate()
#define BQ40Z50_SERIALNUMBER_CMD         (0x1C)//SerialNumber()         
#define BQ40Z50_MANUFACTURERNAME_CMD     (0x20)//ManufacturerName()     
#define BQ40Z50_CELLVOLTAGE2_CMD         (0x3F)//CellVoltage2()         
#define BQ40Z50_CELLVOLTAGE1_CMD         (0x3E)//CellVoltage1()           
#define BQ40Z50_SAFETYSTATUS_CMD         (0x51)//SafetyStatus()
#define BQ40Z50_PFSTATUS_CMD             (0x53)//PFStatus()
#define BQ40Z50_DASTATUS_CMD             (0x72)//DAStatus2()
                                     
#define BQ40Z50_DEVICENAME_CMD           (0x21)//DeviceName()                      
#define BQ40Z50_SOHEALTH_CMD             (0x4f)//State-of-Health (SoH)  

/* SafetyStatus: (0x51) */
#define BQ40Z50_SAFETY_STATUS_OTD_UTD     (0x08002000)   // Overtemperature During Discharge / Undertemperature During Discharge
#define BQ40Z50_SAFETY_STATUS_OTC_UTC     (0x04001000)   // Overtemperature During Charge / Undertemperature During Charge
#define BQ40Z50_SAFETY_STATUS_OCD1_OCD2   (0x00000030)   // Overcurrent During Discharge 1 / Overcurrent During Discharge 2
#define BQ40Z50_SAFETY_STATUS_OC          (0x00400000)   // Over Charge
#define BQ40Z50_SAFETY_STATUS_CUV         (0x00000001)   // Cell Undervoltage
#define BQ40Z50_SAFETY_STATUS_COV         (0x00000010)   // Cell Overvoltage
#define BQ40Z50_SAFETY_STATUS_SHORT       (0x00000F00)   // Short-circuit

/* Battery Status: (0x16) */
#define BQ40Z50_BATTERY_STATUS_OCA        (0x8000)   //Over Charged Alarm
#define B140Z50_BATTERY_STATUS_DSG        (0x0040)   //Discharg/Charge :0 means in charge mode
#define BQ40Z50_BATTERY_STATUS_FC         (0x0020)   //Fully Charged

typedef struct _tBattRegData_BQ40Z50 {
    uint8      regAddr;
    uint8      valueLength;
    eSettingId settingId;
    uint16     regValue;
    bool       valid;
} tBattRegData_BQ40Z50;

CLASS(cBatteryDrv_BQ40Z50)
uint8       deviceI2cAddr;
tI2CDevice  *pI2CConfig;
cI2CDrv     * pI2cObj;
bool        isCreated;
METHODS

void BatteryDrv_Ctor_BQ40Z50(cBatteryDrv_BQ40Z50 * me, cI2CDrv *pI2cObj);
void BatteryDrv_Xtor_BQ40Z50(cBatteryDrv_BQ40Z50 * me);

void BatteryDrv_ShutDown_BQ40Z50(cBatteryDrv_BQ40Z50 * me);
int16 BatteryDrv_GetTemperature_BQ40Z50(cBatteryDrv_BQ40Z50 * me);
uint16 BatteryDrv_GetVoltage_BQ40Z50(cBatteryDrv_BQ40Z50 * me);
uint16 BatteryDrv_GetCurrent_BQ40Z50(cBatteryDrv_BQ40Z50 * me);
uint16 BatteryDrv_GetAverageCurrent_BQ40Z50(cBatteryDrv_BQ40Z50 * me);
uint16 BatteryDrv_GetRelativeStateOfCharge_BQ40Z50(cBatteryDrv_BQ40Z50 * me);
uint16 BatteryDrv_GetAbsoluteStateOfCharge_BQ40Z50(cBatteryDrv_BQ40Z50 * me);
uint16 BatteryDrv_GetRemainingCapacity_BQ40Z50(cBatteryDrv_BQ40Z50 * me);
uint16 BatteryDrv_GetFullChargeCapacity_BQ40Z50(cBatteryDrv_BQ40Z50 * me);
uint16 BatteryDrv_ReadWordCmd_BQ40Z50(cBatteryDrv_BQ40Z50 * me, uint8 cmd);
eTpRet BatteryDrv_readRegValue_BQ40Z50(cBatteryDrv_BQ40Z50 * me, tBattRegData_BQ40Z50 *pBattData);
END_CLASS

#endif
