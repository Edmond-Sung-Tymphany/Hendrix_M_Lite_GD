local_dir   := driver/display_driver

ifneq "$(filter HAS_TM1629,$(TP_FEATURE))" ""
local_src   := tm1629_driver/Tm1629Drv.c
endif

ifeq "$(TP_MCU_FAMILY)" "posix"
local_src   := posix/DisplayDrv.c
endif

sources     += $(addprefix $(local_dir)/,$(local_src))
