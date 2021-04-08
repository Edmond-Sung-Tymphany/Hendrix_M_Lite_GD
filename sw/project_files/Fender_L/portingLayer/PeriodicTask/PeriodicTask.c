#include "product.config"

#ifdef HAS_PERIODIC_SERVER_TASK

#include <stdint.h>
#include "commontypes.h"
#include "stm32f0xx.h"
#include "PeriodicTask.h"
#ifdef HAS_SYSTEM_CONTROL
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
#ifdef HAS_POWER_SWITCH_KEY
    SystemDrv_PowerSwitchUpdate();
#endif
#ifdef HAS_GPIO_LED
    GLED_PeriodicRefresh();
#endif
#ifdef HAS_IWDG
    IWDG_ReloadCounter();
#endif
}

#endif  // HAS_PERIODIC_SERVER_TASK

