local_dir   := driver/led_driver
local_src   := LedDrv.c

ifneq "$(filter LED_HAS_IOEXPANDER,$(TP_FEATURE))" ""
local_src   += IoExpanderLedDrv.c
endif

ifneq "$(filter LED_HAS_PWM,$(TP_FEATURE))" ""
local_src   += $(TP_MCU_FAMILY)/PwmLedDrv.c
endif

sources     += $(addprefix $(local_dir)/,$(local_src))

