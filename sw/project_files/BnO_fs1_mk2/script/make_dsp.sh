#!/bin/sh
#*************************************************
# Automatic Build Script - Setting file          *
#                                                *
# Usage                                          *
#   make_dsp.sh [--disable-log]                  * 
#                                                *
#*************************************************

#********************************
# Include                       *
#********************************
. ../../../scripts/ase_platform_build_script/make_lib.sh


#********************************
# Build Setting                 *
#********************************
#build generator
build_debug_en="1"
build_release_en="1"

#ota generator
ota_debug_en="0"
ota_release_en="1"

#clean build
clean_en="1"

#zip enable
zip_en="0"

#dsp auto build
dsp_auto_build="1"
dsp_ver_header="DSP/ADAU1451_IC_1_PARAM.h"
dsp_ver_param="MOD_DC1_DCINPALG145X1VALUE_VALUE"

#disable log for Jenkins build
if [ "$1" == "--disable-log" ]; then
	disable_log="1"
fi


#********************************
# Project Environment           *
#********************************
project=BnO_fs1_mk2
hw_ver=ES1

#folder
ota_folder="ota"
tmp_folder="tmp_files"

#pre-built bootloader
#  not define means re-build new piu/ubl
#  have define means use old image
piu_prebuilt_hex=""
ubl_prebuilt_hex=""

#hex name
piu_build_hex="${project}_piu.hex"
ubl_build_hex="${project}_ubl.hex"
fw_build_hex="${project}_fw.hex"

#intermediate files
tmp_hex="tmp.hex"

#log files
tmp_log="hexmerger_t.log"
out_log="hexmerger.log"



#********************************
# Main Flow                     *
#********************************
build_start

