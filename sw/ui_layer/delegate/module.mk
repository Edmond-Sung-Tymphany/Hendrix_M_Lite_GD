local_dir   := ui_layer/delegate
local_src   := delegate.c

ifneq "$(filter HAS_MENU,$(TP_FEATURE))" ""
local_src   += menu_dlg/MenuDlg.c
endif

ifneq "$(filter HAS_IR_LEARNING,$(TP_FEATURE))" ""
local_src   += irlearning_dlg/IrlearningDlg.c
endif

ifneq "$(filter HAS_POWER_DELEGATE,$(TP_FEATURE))" ""
local_src   += power_dlg/PowerDlg.c
endif

ifneq "$(filter HAS_BT_DELEGATE,$(TP_FEATURE))" ""
local_src   += bluetooth_dlg/BluetoothDlg.c
endif

sources     += $(addprefix $(local_dir)/,$(local_src))

	