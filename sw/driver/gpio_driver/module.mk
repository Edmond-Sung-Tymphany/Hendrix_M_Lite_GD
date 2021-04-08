local_dir   := driver/gpio_driver
local_src   := $(TP_MCU_FAMILY)/GpioDrv.c

sources     += $(addprefix $(local_dir)/,$(local_src))

