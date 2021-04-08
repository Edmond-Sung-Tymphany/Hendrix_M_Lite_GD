#!/bin/sh
#*************************************************
# Automatic Build Script - Setting file          *
#                                                *
#*************************************************

#********************************
# Include                       *
#********************************
. ./make_lib.sh


#********************************
# Build Setting                 *
#********************************
#build generator
build_debug_en="0"
build_release_en="1"

#ota generator
ota_debug_en="1"
ota_release_en="1"

#clean build
clean_en="0"

#zip enable
zip_en="1"

#dsp auto build 
dsp_auto_build="0"
dsp_ver_header="DSP/ADAU1761_IC_1_PARAM.h"
dsp_ver_param="MOD_SWVERSION_DCINPALG1_VALUE"

#********************************
# Project Environment           *
#********************************
project=SVS_SB3000
hw_ver=EVT

# *.Config path
if   [ "SVS_PB3000" == "${project}" ]; then
config_path="conf/PB/product.config"
ble_path="D:/Projects/SVS3000/Firmware/ble/SVS_PB3000_BLE_swv5.0.2_20180411"
dsp_path="dsp/PB/"
elif [ "SVS_SB3000" == "${project}" ]; then
config_path="conf/SB/product.config"
ble_path="D:/Projects/SVS3000/Firmware/ble/SVS_SB3000_BLE_swv5.0.2_20180411"
dsp_path="dsp/SB/"
else
config_path="product.config"
dsp_path="dsp/PB/"
fi

#folder
ota_folder="upgrade"
tmp_folder="tmp_files"
dsp_folder="dsp"
ble_folder="ble"

#pre-built bootloader
#  not define means re-build new piu/ubl
#  have define means use old image
piu_prebuilt_hex=""
#ubl_prebuilt_hex="ca17_ubl_swv3.0.0_release.hex"

#hex name
piu_build_hex="piu.hex"
fw_build_hex="${project}.hex"

#intermediate files
tmp_hex="tmp.hex"

#log files
disable_log=""
tmp_log="hexmerger_t.log"
out_log="hexmerger.log"



#********************************
# Main Flow                     *
#********************************
build_start

