local_dir   := driver/key_driver
local_src   := KeyDrv.c

ifneq "$(filter HAS_ADC_KEY,$(TP_FEATURE))" ""
local_src   += AdcKeyDrv.c
endif

ifneq "$(filter HAS_GPIO_KEY,$(TP_FEATURE))" ""
local_src   += GpioKeyDrv.c
endif

sources     += $(addprefix $(local_dir)/,$(local_src))

