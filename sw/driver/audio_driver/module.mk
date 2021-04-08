local_dir   := driver/audio_driver
local_src   := $(TP_MCU_FAMILY)/AudioDrv.c

sources     += $(addprefix $(local_dir)/,$(local_src))

