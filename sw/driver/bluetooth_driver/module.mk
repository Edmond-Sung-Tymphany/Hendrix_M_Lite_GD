local_dir   := driver/bluetooth_driver

ifneq "$(filter HAS_ROM_BASE_BT,$(TP_FEATURE))" ""
local_src   := rom_base/BluetoothDrv.c
endif
ifneq "$(filter HAS_WITRON,$(TP_FEATURE))" ""
local_src   := witron/BluetoothWitronDrv.c
endif

sources     += $(addprefix $(local_dir)/,$(local_src))
