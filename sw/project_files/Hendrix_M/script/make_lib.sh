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
path_hexmate="../../../../../tool/"
path_prebuild="stm32/bootloader/prebuilt/"

#files
fepaddr_file="../stm32/include/fep_addr.h"
sdf_file="../conf/sdf.json"
upgrade_cmd_file="upgrade.sh"
bl_status_hex="BnO_ca_info.hex"

#log files
tmp_log="hexmerger_t.log"
out_log="hexmerger.log"

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
	ota_gen=$5

	#File name
	if [ "1" == ${dsp_auto_build} ]; then
		sw_ver=${sw_ver}_dsp_test
	fi

	package_name=${project}_hw${hw_ver}_swv${sw_ver}_${date}_${tp_target}
	fw_tgt_hex=${project}_fw_swv${sw_ver}_${tp_target}.hex

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
	mkdir "${out_folder}/${tmp_folder}/${tp_target}"
	#mkdir "${out_folder}/${folder_target}"
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
	# finish
	#########################
	if [ "release" == ${tp_target} ]; then
		mkdir ${folder_target}/production_test
		pwd
		cp "${sdf_file}" "${folder_target}/production_test/sdf_${project}_v${sw_ver}.json"
	fi

	if [ "release" == ${tp_target} ]; then
    	md5sum ${folder_target}/*.hex >> md5sum.txt
    fi
	
	#popd
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

	cd ../
	#pushd ../
	
	#FW Version
	echo "Parsing software version ..."
	echo ""
	ver0=$(sed -n 's/#define *SW_MAJOR_VERSION *\(\)/\1/p'  product.config)
	ver1=$(sed -n 's/#define *SW_MINOR_VERSION1 *\(\)/\1/p' product.config)
	ver2=$(sed -n 's/#define *SW_MINOR_VERSION2 *\(\)/\1/p' product.config)
	ver3=$(sed -n 's/#define *SW_MINOR_VERSION3 *\(\)/\1/p' product.config)
	if [ "${ver3}" == "0" ]; then
		sw_version=${ver0}.${ver1}.${ver2}
	else
		sw_version=${ver0}.${ver1}.${ver2}.${ver3}	
	fi
	echo "FW Version: ${sw_version}"
	


	#Folder name
	out_folder="${project}_hw${hw_ver}_swv${sw_version}_${date}"


	#Create folder
	rm -f package.zip
	rm -f "${out_folder}*.zip"
	rm -rf "${out_folder}"
	mkdir "${out_folder}"
	mkdir "${out_folder}/${tmp_folder}"


	#Build target                                      build enable        ota enable
	build_target ${sw_version} "debug"   "development"  ${build_debug_en}    ${ota_debug_en}
	build_target ${sw_version} "release" "production"   ${build_release_en}  ${ota_release_en}
	store_git_sha "${out_folder}/${tmp_folder}/git_sha.txt"


	#dsp version
	pushd ${out_folder}/
	
	#ToDo Add DSP version
	#dsp_ver=    #$(sed -n "s/.*${dsp_ver_param}.*_CONVERT *(\(\)/\1/p" "../${dsp_ver_header}" | sed 's/)//g')
	dsp_ver2=$dsp_version    #$(printf "%.2f" "${dsp_ver}")
	echo DSP Version: ${dsp_ver2}
	echo DSP Version: ${dsp_ver2} > "${tmp_folder}/dsp_version.txt"


	#zip files	
	if [ ${zip_en} == "1" ] && [ ${dsp_auto_build} != "1" ]; then
		"${zip_path}" a "../${project}_hw${hw_ver}_swv${sw_version}_${date}.zip" $(ls -I $tmp_folder)
		"${zip_path}" a "../${project}_hw${hw_ver}_swv${sw_version}_${date}_backup.zip" "./*"
	fi


	#dsp auto build
	if [ "1" == ${dsp_auto_build} ]; then
		echo
		echo " *** dsp auto build actions **********************************************************"
		
		#Prepare DSP files
		dsp_path="dsp_v${dsp_ver2}"
		mkdir "${dsp_path}"
		cp ../DSP/ADAU*_1*.h "${dsp_path}"
		
		cp $(pwd)/${tmp_folder}/debug/*.hex "${dsp_path}"

		store_git_sha "${dsp_path}/sha.txt"
		"${zip_path}" a "../package.zip" $(ls -I $tmp_folder)
		
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
	printf "     DSP: v${dsp_ver2} \r\n"
if [ "" == "$piu_prebuilt_hex" ]; then
	printf "     PIU: v${piu_version}  \r\n"
else
	printf "     PIU: ${piu_prebuilt_hex}  \r\n"
fi
if [ "" == "$ubl_prebuilt_hex" ]; then
	printf "     UBL: v${ubl_version}  \r\n"
else
	printf "     UBL: ${ubl_prebuilt_hex}  \r\n"
fi
	if [ ${ota_debug_en} == "1" ] || [ ${ota_release_en} == "1" ]; then
		printf "     Checksum: ubl[${ubl_chm2}], fw[${fw_chm2}]  \r\n"		
	fi
	print_elapsed_time $timeStart $timeEnd
	printf "*******************************************************\r\n"
}


