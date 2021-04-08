#include "product.config"

#ifdef HAS_PERIODIC_SERVER_TASK

#include <stdint.h>
#include "commontypes.h"
#include "stm32f0xx.h"
#include "PeriodicTask.h"
#if defined(HAS_SYSTEM_CONTROL) || defined(HAS_DC_IN) || defined(HAS_BATTERY_DETECT)
#include "SystemDrv.h"
#endif
#ifdef HAS_GPIO_LED
#include "GpioLed.h"
#endif

void PeriodicTask_Init(void)
{
#ifdef HAS_GPIO_LED
    GLED_Init();
#endif
}

void PeriodicTask_Execute(void)
{
#ifdef HAS_SYSTEM_CONTROL
    SystemDrv_PowerSwitchUpdate();
#endif
#ifdef HAS_DC_IN
    SystemDrv_PowerDCInDetect();
#endif
#ifdef HAS_BATTERY_DETECT
    SystemDrv_PowerBatteryDetect();
#endif
#ifdef HAS_BATTERY_NTC
    SystemDrv_BatteryNTCValue();
#endif
#ifdef HAS_GPIO_LED
    GLED_PeriodicRefresh();
#endif
#ifdef HAS_BAT_CHARGE_STATUS
    SystemDrv_CheckBatteryChargeStatus();
#endif
#ifdef HAS_IWDG
    IWDG_ReloadCounter();
#endif
}

#endif  // HAS_PERIODIC_SERVER_TASK

