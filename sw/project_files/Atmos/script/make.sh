#!/bin/sh
#*************************************************
# Automatic Build Script - Setting file          *
#                                                *
#*************************************************

#********************************
# Include                       *
#********************************
#####. ../../ca16/script/make_lib.sh
./make_lib.sh

#********************************
# Build Setting                 *
#********************************
#build generator
build_debug_en="1"
build_release_en="0"

#ota generator
ota_debug_en="1"
ota_release_en="0"

#clean build
clean_en="1"

#zip enable
zip_en="1"


#********************************
# Project Environment           *
#********************************
project=ca16
hw_ver=ES2

#folder
ota_folder="ota"
tmp_folder="tmp_files"

#hex name
piu_hex="${project}_piu.hex"
ubl_hex="${project}_ubl.hex"
fw_hex="${project}_fw.hex"
out_hex=${project}_all.hex

#intermediate files
tmp_hex="tmp.hex"

#log files
tmp_log="hexmerger_t.log"
out_log="hexmerger.log"




#********************************
# Main Flow                     *
#********************************
build_start

