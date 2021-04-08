local_dir   := driver/power_driver

ifneq "$(filter HAS_POWER_LIGHT, $(TP_FEATURE))" ""
local_src   := $(TP_MCU_FAMILY)/PowerDrv_light.c
else
local_src   := $(TP_MCU_FAMILY)/PowerDrv.c
endif

sources     += $(addprefix $(local_dir)/,$(local_src))

