#!/bin/sh
#*************************************************
# Automatic Build Script                         *
#                                                *
#*************************************************


#********************************
# Common Environment            *
#********************************
#paths
iar_path="C:/Program Files (x86)/IAR Systems/Embedded Workbench 7.2/common/bin/IarBuild.exe"
zip_path="../../../tool/7z.exe"
path_hexmate="../../../../tool/"
path_prebuild="stm32/bootloader/prebuilt/"

#files
bl_status_hex="BnO_ca_info.hex"

#log files
#tmp_log="hexmerger_t.log"
#out_log="hexmerger.log"

#auto parameters
date=$(date "+%Y%m%d")

#other
paralle_cpu=4


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
	folder_target=$3
	build_en=$4

	#File name
	if [ "1" == ${dsp_auto_build} ]; then
		sw_ver=${sw_ver}_dsp_test
	fi

	package_name=${project}_hw${hw_ver}_swv${sw_ver}_${date}
	fw_tgt_hex=${project}_app_swv${sw_ver}.hex

	echo "=============================================="
	echo ${package_name}
	echo "=============================================="

	# need to build?
	if [ "1" != $build_en ]; then
		return
	fi



	#########################
	# prepare hex
	#########################

	#########################
	# Build PIU
	#########################
	if [ "" == "$piu_prebuilt_hex" ]; then
		if [ "1" == $clean_en ]; then
			"${iar_path}" iar/BnO_SW_BTL.ewp -clean ${tp_target}
		fi

		if [ "1" == "${disable_log}" ]; then
			"${iar_path}" iar/BnO_SW_BTL.ewp -build ${tp_target} -parallel ${paralle_cpu} | grep -E '(Error|Total number of)'
		else
			"${iar_path}" iar/BnO_SW_BTL.ewp -build ${tp_target} -parallel ${paralle_cpu}
		fi

		piu_tgt_hex=${project}_btl_swv${piu_version}.hex
		cp iar/${tp_target}/exe/${piu_build_hex} ${out_folder}/${hex_folder}/${piu_tgt_hex}
	else
		piu_tgt_hex=${piu_prebuilt_hex}
		cp ${path_prebuild}/${piu_prebuilt_hex}  ${out_folder}/${hex_folder}/${piu_tgt_hex}
	fi

	#########################
	# Build FW
	#########################
	if [ "1" == $clean_en ]; then
		"${iar_path}" iar/BnO_SW_APP.ewp -clean ${tp_target}
	fi

	
	if [ "1" == ${disable_log} ]; then
		"${iar_path}" iar/BnO_SW_APP.ewp -build ${tp_target} -parallel ${paralle_cpu} >1
	else
		"${iar_path}" iar/BnO_SW_APP.ewp -build ${tp_target} -parallel ${paralle_cpu}
	fi

	cp iar/${tp_target}/exe/${fw_build_hex} ${out_folder}/${hex_folder}/${fw_tgt_hex}


	#########################
	# Pre-process hex
	#########################
	cp iar/${tp_target}/list/*.map ${out_folder}/${msg_folder}

	pushd ${out_folder}/${hex_folder}/

	# remove unused line containing "Start Linear Address"
	sed -i "/:04000005/d" *.hex

	# remove unused section
#	sed -i "/00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/d" *.hex


#	cp ${path_hexmate}/${bl_status_hex} ./
#	"${path_hexmate}/hexmate.exe" "${piu_tgt_hex}" "${fw_tgt_hex}" --edf=${path_hexmate}/en_msgs.txt -o${tmp_hex} -LOGFILE=${tmp_log}
#	"${path_hexmate}/hexmate.exe" "${tmp_hex}" "${bl_status_hex}" --edf=${path_hexmate}/en_msgs.txt -o../../${folder_target}/${package_name}.hex -LOGFILE=${out_log}

	"${path_hexmate}/hexmate.exe" "${piu_tgt_hex}" "${fw_tgt_hex}" --edf=${path_hexmate}/en_msgs.txt -o${project}_hw${hw_ver}_swv${sw_ver}.hex
	

	popd
	
	#########################
	# finish
	#########################
}


function store_git_sha()
{
	file_sha=$1
	git_sha=$(git log | grep commit | awk 'NR==1;')
	echo "GIT-SHA: ${git_sha}" >  "${file_sha}"
}


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


function build_start()
{
	#stop when error
	set e
	date
	timeStart=$(date +%s)

	pushd ../
	
	echo "Parsing software version ..."
	echo ""

	#PIU Version
	if [ "" == "$piu_prebuilt_hex" ]; then
		pver1=$(sed -n 's/#define BOOTLOADER_VERSION1 *\(\)/\1/p' product.config)
		pver2=$(sed -n 's/#define BOOTLOADER_VERSION2 *\(\)/\1/p' product.config)
		pver3=$(sed -n 's/#define BOOTLOADER_VERSION3 *\(\)/\1/p' product.config)

		piu_version=${pver1}.${pver2}.${pver3}
		echo "BootLoader Version: ${piu_version}"
	fi


	#FW Version
	ver1=$(sed -n 's/#define APPLICATION_VERSION1 *\(\)/\1/p' product.config)
	ver2=$(sed -n 's/#define APPLICATION_VERSION2 *\(\)/\1/p' product.config)
	ver3=$(sed -n 's/#define APPLICATION_VERSION3 *\(\)/\1/p' product.config)
	sw_version=${ver1}.${ver2}.${ver3}	
	echo "APP Version: ${sw_version}"


	#Folder name
	out_folder="${project}_hw${hw_ver}_swv${sw_version}_${date}"


	#Create folder
	rm -f package.zip
	rm -f "${out_folder}*.zip"
	rm -rf "${out_folder}"
	mkdir "${out_folder}"
	mkdir "${out_folder}/${hex_folder}"
	mkdir "${out_folder}/${msg_folder}"


	#Build target                                      build enable      
	build_target ${sw_version} "debug"   "development"  ${build_debug_en}    
	store_git_sha "${out_folder}/${msg_folder}/git_sha.txt"


	#dsp version
	pushd ${out_folder}
	dsp_ver=$(sed -n "s/.*${dsp_ver_param}.*_CONVERT *(\(\)/\1/p" "../${dsp_ver_header}" | sed 's/)//g')
	dsp_ver2=$(printf "%d" "${dsp_ver}")
	echo DSP Version: ${dsp_ver2}
	echo DSP Version: v${dsp_ver2} > "${msg_folder}/dsp_version.txt"


	#zip files	
	if [ ${zip_en} == "1" ] && [ ${dsp_auto_build} != "1" ]; then
		"${zip_path}" a "../${project}_hw${hw_ver}_swv${sw_version}_${date}.zip" $(ls -I $hex_folder)
		"${zip_path}" a "../${project}_hw${hw_ver}_swv${sw_version}_${date}_backup.zip" "./*"
	fi


	#dsp auto build
	if [ "1" == ${dsp_auto_build} ]; then
		echo
		echo " *** dsp auto build actions **********************************************************"
		
		#Prepare DSP files
		cp ../portingLayer/Adau1452_drv/BnO_SoundWall_IC_1*.h "${msg_folder}"

		"${zip_path}" a "../package.zip" $(ls -I $out_folder)
		cd ..
		rm -rf "$out_folder"
	fi
	popd

	#Print time
	date
	timeEnd=$(date +%s)
	printf "\r\n"
	printf "\r\n"
	printf "*******************************************************\r\n"
	printf "  Build successful         \r\n"
	printf "     Name: ${out_folder}   \r\n"
##### bootloader version
if [ "" == "$piu_prebuilt_hex" ]; then
	printf "     BTL: v${piu_version}  \r\n"
else
	printf "     BTL: ${piu_prebuilt_hex}  \r\n"
fi
##### application version
	printf "     APP: v${sw_version}  \r\n"
##### DSP version
	printf "     DSP: v${dsp_ver2} \r\n"

	print_elapsed_time $timeStart $timeEnd
	printf "*******************************************************\r\n"
}


