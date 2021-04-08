local_dir   := driver/i2c_driver
local_src   := $(TP_MCU_FAMILY)/I2CDrv.c

sources     += $(addprefix $(local_dir)/,$(local_src))

