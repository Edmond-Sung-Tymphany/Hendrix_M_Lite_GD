local_dir   := driver/DSP_Driver

ifneq "$(filter HAS_ADAU1761,$(TP_FEATURE))" ""
local_src   := adau1761_drv/dspAdau1761Drv.c
endif
ifneq "$(filter HAS_CS42526,$(TP_FEATURE))" ""
local_src   := CS42526_drv/cs42526_drv.c
endif
ifneq "$(filter HAS_TLV320AIC3254,$(TP_FEATURE))" ""
local_src   := TLV320AIC3254_drv/DspDrv.c
endif

sources     += $(addprefix $(local_dir)/,$(local_src))
