local_dir   := driver/dma_driver
local_src   := $(TP_MCU_FAMILY)/DmaDrv.c

sources     += $(addprefix $(local_dir)/,$(local_src))

