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
. ../../BnO_SoundWall/script/make_soundshape.sh


#********************************
# Build Setting                 *
#********************************
#build generator
build_debug_en="1"
build_release_en="0"

#clean build
clean_en="1"

#zip enable
zip_en="0"

#dsp auto build
dsp_auto_build="1"
dsp_ver_header="portingLayer/Adau1452_drv/BnO_SoundWall_IC_1_PARAM.h"
dsp_ver_param="MOD_VERSION_DCINPALG145X1VALUE_VALUE"

#disable log for Jenkins build
if [ "$1" == "--disable-log" ]; then
	disable_log="1"
fi


#********************************
# Project Environment           *
#********************************
project=BnO_SoundWall
hw_ver=EVT

#folder
hex_folder="hex_files"
msg_folder="msg_files"

#pre-built bootloader
#  not define means re-build new piu
#  have define means use old image
piu_prebuilt_hex=""

#hex name
piu_build_hex="BnO_SW_BTL.hex"
fw_build_hex="BnO_SW_APP.hex"

#intermediate files
#tmp_hex="tmp.hex"

#log files
#tmp_log="hexmerger_t.log"
#out_log="hexmerger.log"



#********************************
# Main Flow                     *
#********************************
build_start

