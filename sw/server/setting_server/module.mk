local_dir   := server/setting_server

ifneq "$(filter HAS_LIGHT_SETTING_SRV,$(TP_FEATURE))" ""
local_src   := SettingSrv_light.c
else
local_src   := SettingSrv.c
endif
sources     += $(addprefix $(local_dir)/,$(local_src))

