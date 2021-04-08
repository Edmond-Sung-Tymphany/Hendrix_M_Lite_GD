local_dir   := driver/ir_receiver_driver

ifneq "$(filter HAS_NEC_IR,$(TP_FEATURE))" ""
local_src   := NEC/NecIrDrv.c
endif
ifneq "$(filter HAS_IR_LEARNING,$(TP_FEATURE))" ""
local_src   := IR_LEARNING/IrLearningDrv.c
endif
sources     += $(addprefix $(local_dir)/,$(local_src))

