local_dir   := bsp/stm32/Libraries/STM32F0xx_StdPeriph_Driver/src

local_src   :=
local_src   += stm32f0xx_exti.c
local_src   += stm32f0xx_misc.c
local_src   += stm32f0xx_pwr.c
local_src   += stm32f0xx_rcc.c
local_src   += stm32f0xx_syscfg.c
local_src   += stm32f0xx_tim.c

ifneq "$(filter HAS_ADC,$(TP_FEATURE))" ""
local_src   += stm32f0xx_adc.c
endif

ifneq "$(filter HAS_GPIO,$(TP_FEATURE))" ""
local_src   += stm32f0xx_gpio.c
endif

ifneq "$(filter HAS_I2C,$(TP_FEATURE))" ""
local_src   += stm32f0xx_i2c.c
endif

ifneq "$(filter HAS_NVM,$(TP_FEATURE))" ""
local_src   += stm32f0xx_flash.c
endif

ifneq "$(filter HAS_UART,$(TP_FEATURE))" ""
local_src   += stm32f0xx_usart.c
endif


sources     += $(addprefix $(local_dir)/,$(local_src))

