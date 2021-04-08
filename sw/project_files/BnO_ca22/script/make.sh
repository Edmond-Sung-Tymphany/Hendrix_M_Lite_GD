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
build_debug_en="1"
build_release_en="1"

#ota generator
ota_debug_en="0"
ota_release_en="1"

#clean build
clean_en="1"

#zip enable
zip_en="1"

#dsp auto build 
dsp_auto_build="0"
dsp_ver_header="DSP/ADAU1761_IC_1_PARAM.h"
dsp_ver_param="MOD_SWVERSION_DCINPALG1_VALUE"


#********************************
# Project Environment           *
#********************************
project=ca22
hw_ver=MP

#folder
ota_folder="ota"
tmp_folder="tmp_files"

#pre-built bootloader
#  not define means re-build new piu/ubl
#  have define means use old image
#piu_prebuilt_hex="ca17_piu_swv3.0.0_release.hex"
#ubl_prebuilt_hex="ca17_ubl_swv3.0.0_release.hex"

#hex name
piu_build_hex="piu.hex"
ubl_build_hex="ubl.hex"
fw_build_hex="app.hex"

#intermediate files
tmp_hex="tmp.hex"

#log files
tmp_log="hexmerger_t.log"
out_log="hexmerger.log"



#********************************
# Main Flow                     *
#********************************
build_start

