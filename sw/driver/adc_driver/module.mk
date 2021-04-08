local_dir   := driver/adc_driver
local_src   := $(TP_MCU_FAMILY)/AdcDrv.c

sources     += $(addprefix $(local_dir)/,$(local_src))

