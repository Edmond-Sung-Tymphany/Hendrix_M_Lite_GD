local_dir   := driver/storage_driver
local_src   := StorageDrv.c

ifneq "$(filter HAS_NVM,$(TP_FEATURE))" ""
local_src   += $(TP_MCU_FAMILY)/NvmDrv.c
endif

sources     += $(addprefix $(local_dir)/,$(local_src))

