#!/bin/sh
#*************************************************
# Automatic Build Script - Setting file          *
#                                                *
# Usage                                          *
#   make_ota_fw.sh [--disable-log]               *
#                                                *
#*************************************************

#********************************
# Include                       *
#********************************
pushd ../


#********************************
# Project Environment           *
#********************************
project=Joplin
model=Joplin_L
hw_ver=EVT

#folder
ota_folder="ota"
tmp_folder="tmp_files"

#hex name
fw_build_hex="${project}_fw.hex"

#intermediate files
tmp_hex="tmp.hex"

#log files
tmp_log="hexmerger_t.log"
out_log="hexmerger.log"

#overwrite model_config.h in project folder
cp -f ../model_config/${model}.h ../model_config.h

#********************************
# Common Environment            *
#********************************
#paths
iar_path="C:/Program Files (x86)/IAR Systems/Embedded Workbench 7.2/common/bin/IarBuild.exe"
zip_path="../../../tool/7z.exe"

#files
fepaddr_file="../stm32/include/fep_addr.h"
bl_status_hex="BnO_ca_info.hex"

#auto parameters
date=$(date "+%Y%m%d")

#other
paralle_cpu=4

#********************************
# Build Setting                 *
#********************************
#build generator
build_debug_en="0"
build_release_en="1"

#ota generator
ota_debug_en="0"
ota_release_en="1"

#clean build
clean_en="1"

#disable log for Jenkins build
if [ "$1" == "--disable-log" ]; then
	disable_log="1"
fi

function print_elapsed_time()
{
	timeStart=$1
	timeEnd=$2

	timeElapsed=$((timeEnd-timeStart))
	ss=$((timeElapsed%60))
	mm=$(((timeElapsed/60)%60))
	hh=$((timeElapsed/3600))
	printf "     Time: %02i:%02i:%02i\r\n" $hh $mm $ss
}

#********************************
# Function                      *
#********************************
function build_target()
{
	#########################
	# arguments
	#########################
	sw_ver=$1
	tp_target=$2
	#folder_target=$3
	build_en=$4
	ota_gen=$5

	#File name
	package_name=${model}_hw${hw_ver}_swv${sw_ver}_${date}_${tp_target}
	fw_tgt_hex=${model}_fw_swv${sw_ver}_${tp_target}.hex

	echo "=============================================="
	echo ${package_name}
	echo "=============================================="

	#########################
	# prepare hex
	#########################
	# need to build?
	if [ "1" != $build_en ]; then
		return
	fi

	mkdir "${out_folder}/${tmp_folder}/${tp_target}"
	#rm -f iar/${tp_target}/exe/* iar/${tp_target}/list/*

	#########################
	# Build FW
	#########################
	if [ "1" == $clean_en ]; then
		"${iar_path}" iar/${project}.ewp -clean ${tp_target}
	fi

	if [ "1" == "${disable_log}" ]; then
		"${iar_path}" iar/${project}.ewp -build ${tp_target} -parallel ${paralle_cpu} | grep -E '(Error|Total number of)' 
	else
		"${iar_path}" iar/${project}.ewp -build ${tp_target} -parallel ${paralle_cpu}
	fi

	cp iar/${tp_target}/exe/${fw_build_hex} ${out_folder}/${tmp_folder}/${tp_target}/${fw_tgt_hex}

	#########################
	# Pre-process hex
	#########################
	cp iar/${tp_target}/list/*.map ${out_folder}/${tmp_folder}/${tp_target}/

	pushd ${out_folder}/${tmp_folder}/${tp_target}/

	# remove unused line containing "Start Linear Address"
	sed -i.org "/:04000005/d" *.hex

	# remove unused section
	sed -i "/00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/d" *.hex

	popd

	#########################
	# Prepare ota package
	#########################
	pushd "${out_folder}"
	if [ "1" == $ota_gen ]; then
		mkdir "${ota_folder}/"

		# copy files
		cp "${fepaddr_file}" "${ota_folder}"

		# generate upgrade bin file
		fw_chm=$(sed -n "s/:04F80800\(\)/\1/p" "${tmp_folder}/${tp_target}/${fw_tgt_hex}")
		fw_chm2="${fw_chm:6:2}${fw_chm:4:2}${fw_chm:2:2}${fw_chm:0:2}"
		python ../script/hex2bin.py "${tmp_folder}/${tp_target}/${fw_tgt_hex}" "${ota_folder}/Joplin.bin" "${sw_ver}" "0x${fw_chm2}"
	fi

	popd
}

function build_start()
{
	#stop when error
	set e
	date
	timeStart=$(date +%s)

	pushd ../
	#FW Version
	echo "Parsing software version ..."
	echo ""
	ver0=$(sed -n 's/#define *SW_MAJOR_VERSION *\(\)/\1/p'  model_config.h)
	ver1=$(sed -n 's/#define *SW_MINOR_VERSION1 *\(\)/\1/p' model_config.h)
	ver2=$(sed -n 's/#define *SW_MINOR_VERSION2 *\(\)/\1/p' model_config.h)
	ver3=$(sed -n 's/#define *SW_MINOR_VERSION3 *\(\)/\1/p' model_config.h)
	if [ "${ver3}" == "0" ]; then
		sw_version=${ver0}.${ver1}.${ver2}
	else
		sw_version=${ver0}.${ver1}.${ver2}.${ver3}
	fi
	echo "FW Version: ${sw_version}"

	#Folder name
	out_folder="Zound_${model}_hw${hw_ver}_swv${sw_version}_${date}"

	#Create folder
	rm -f package.zip
	rm -f "${out_folder}*.zip"
	rm -rf "${out_folder}"
	mkdir "${out_folder}"
	mkdir "${out_folder}/${tmp_folder}"

	#Build target                                      build enable        ota enable
	build_target ${sw_version} "debug"   "MCU"  ${build_debug_en}    ${ota_debug_en}
	build_target ${sw_version} "release" "production"   ${build_release_en}  ${ota_release_en}

	popd

	#Print time
	date
	timeEnd=$(date +%s)
	printf "\r\n"
	printf "\r\n"
	printf "*******************************************************\r\n"
	printf "  Build successful         \r\n"
	printf "     Name: ${out_folder}   \r\n"
	if [ ${ota_debug_en} == "1" ] || [ ${ota_release_en} == "1" ]; then
		printf "     Checksum: fw[${fw_chm2}]  \r\n"
	fi
	print_elapsed_time $timeStart $timeEnd
	printf "*******************************************************\r\n"
}

#********************************
# Main Flow                     *
#********************************
build_start
