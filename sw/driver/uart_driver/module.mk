local_dir   := driver/uart_driver
local_src   := $(TP_MCU_FAMILY)/UartDrv.c

sources     += $(addprefix $(local_dir)/,$(local_src))

