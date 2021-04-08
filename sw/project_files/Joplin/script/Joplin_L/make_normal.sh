#!/bin/sh
#*************************************************
# Automatic Build Script - Setting file          *
#                                                *
#*************************************************

#********************************
# Include                       *
#********************************
pushd ../
. ./make_lib.sh


#********************************
# Build Setting                 *
#********************************
#build generator
build_debug_en="1"
build_release_en="0"

#ota generator
ota_debug_en="0"
ota_release_en="1"

#clean build
clean_en="1"

#zip enable
zip_en="1"

#dsp auto build 
dsp_auto_build="0"
dsp_ver_header="DSP/Joplin_L/ADAU1761_IC_1_PARAM.h"
dsp_ver_param="MOD_SWVERSION_DCINPALG2_VALUE"


#********************************
# Project Environment           *
#********************************
project=Joplin
model=Joplin_L
hw_ver=EVT

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

#overwrite model_config.h in project folder
cp -f ../model_config/${model}.h ../model_config.h

#********************************
# Main Flow                     *
#********************************
build_start

