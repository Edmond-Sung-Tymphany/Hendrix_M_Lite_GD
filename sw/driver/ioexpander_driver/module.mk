local_dir   := driver/ioexpander_driver

ifneq "$(filter HAS_AW9110,$(TP_FEATURE))" ""
local_src   := aw9110b/IoExpanderDrv.c
endif
ifneq "$(filter HAS_AW9523,$(TP_FEATURE))" ""
local_src   := IoExpanderDrv.c
endif

sources     += $(addprefix $(local_dir)/,$(local_src))

