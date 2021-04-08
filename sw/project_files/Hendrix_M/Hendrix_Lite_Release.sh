#!/bin/sh

#********************************
# Include                       *
#********************************

. ./release_lib.sh


#********************************
# Build Setting                 *
#********************************
#build generator
build_debug_en="1"
build_release_en="0"
disable_log="1"

#clean build
clean_en="1"

#zip enable
zip_en="1"

#copy enable
bt_en="1"
dsp_en="1"
pte_en="1"

#dsp auto build 
dsp_auto_build="0"
dsp_ver_header="DSP/ADAU1761_IC_1_PARAM.h"
dsp_ver_param="MOD_SWVERSION_DCINPALG1_VALUE"


#********************************
# Project Environment           *
#********************************
project=Hendrix
model=Lite
hw_ver=PMP
ewp_target="iar/Hendrix_M.ewp"

#folder
release_folder="Release"
tmp_folder="tmp_files"


#intermediate files
tmp_hex="tmp.hex"

printmsg "Call switch Shell"

pushd script/${project}_${model}
#overwrite model_config.h in project folder
sh "switch_to_${project}_${model}.sh"
popd

printmsg "Build Start"
#********************************
# Main Flow                     *
#********************************
build_start
