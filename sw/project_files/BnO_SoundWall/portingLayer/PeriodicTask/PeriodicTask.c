#include "product.config"

#include <stdint.h>
#include "commontypes.h"
#include "stm32f0xx.h"
#include "PeriodicTask.h"
#ifdef HAS_IOE_LED
#include "IoeLedDrv.h"
#endif


void PeriodicTask_Init(void)
{
#ifdef HAS_IOE_LED
#ifndef LED_ONLY_IN_ACTIVE_STATUS
    IoeLed_Init();
#endif
#endif
}

void PeriodicTask_Execute(void)
{
#ifdef HAS_IWDG
    IWDG_ReloadCounter();
#endif
#ifdef HAS_IOE_LED
    IoeLed_Refresh();
#endif
}

