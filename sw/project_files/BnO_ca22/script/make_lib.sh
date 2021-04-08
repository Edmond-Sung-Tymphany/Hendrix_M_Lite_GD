#!/bin/sh
#*************************************************
# Automatic Build Script                         *
#                                                *
#*************************************************


#********************************
# Common Environment            *
#********************************
#paths
iar_path="D:/Programs/IAR Systems/Embedded Workbench 7.2/common/bin/IarBuild.exe"
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
	mkdir "${out_folder}/${folder_target}"
	#rm -f iar/${tp_target}/exe/* iar/${tp_target}/list/*


	#########################
	# Build PIU
	#########################
	if [ "" == "$piu_prebuilt_hex" ]; then
		if [ "1" == $clean_en ]; then
			"${iar_path}" iar/piu.ewp -clean ${tp_target}
		fi

		if [ "1" == "${disable_log}" ]; then
			"${iar_path}" iar/piu.ewp -build ${tp_target} -parallel ${paralle_cpu} | grep -E '(Error|Total number of)' 
		else
			"${iar_path}" iar/piu.ewp -build ${tp_target} -parallel ${paralle_cpu}
		fi

		piu_tgt_hex=${project}_piu_swv${piu_version}_${tp_target}.hex
		cp iar/${tp_target}/exe/${piu_build_hex} ${out_folder}/${tmp_folder}/${tp_target}/${piu_tgt_hex}
	else
		piu_tgt_hex=${piu_prebuilt_hex}
		cp ${path_prebuild}/${piu_prebuilt_hex}  ${out_folder}/${tmp_folder}/${tp_target}/${piu_tgt_hex}
	fi


	#########################
	# Build UBL
	#########################
	if [ "" == "$ubl_prebuilt_hex" ]; then
		if [ "1" == $clean_en ]; then
			"${iar_path}" iar/ubl.ewp -clean ${tp_target}
		fi
		
		if [ "1" == "${disable_log}" ]; then
			"${iar_path}" iar/ubl.ewp -build ${tp_target} -parallel ${paralle_cpu} | grep -E '(Error|Total number of)' 
		else
			"${iar_path}" iar/ubl.ewp -build ${tp_target} -parallel ${paralle_cpu}
		fi

		ubl_tgt_hex=${project}_ubl_swv${ubl_version}_${tp_target}.hex
		cp iar/${tp_target}/exe/${ubl_build_hex} ${out_folder}/${tmp_folder}/${tp_target}/${ubl_tgt_hex}
	else
		ubl_tgt_hex=${ubl_prebuilt_hex}
		cp ${path_prebuild}/${ubl_prebuilt_hex}  ${out_folder}/${tmp_folder}/${tp_target}/${ubl_tgt_hex}
	fi



	#########################
	# Build FW
	#########################
	if [ "1" == $clean_en ]; then
		"${iar_path}" iar/app.ewp -clean ${tp_target}
	fi

	
	if [ "1" == "${disable_log}" ]; then
		"${iar_path}" iar/app.ewp -build ${tp_target} -parallel ${paralle_cpu} | grep -E '(Error|Total number of)' 
	else
		"${iar_path}" iar/app.ewp -build ${tp_target} -parallel ${paralle_cpu}
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


	cp ${path_hexmate}/${bl_status_hex} ./
	"${path_hexmate}/hexmate.exe" "${piu_tgt_hex}" "${ubl_tgt_hex}" "${fw_tgt_hex}" --edf=${path_hexmate}/en_msgs.txt -o${tmp_hex} -LOGFILE=${tmp_log}
	"${path_hexmate}/hexmate.exe" "${tmp_hex}" "${bl_status_hex}" --edf=${path_hexmate}/en_msgs.txt -o../../${folder_target}/${package_name}.hex -LOGFILE=${out_log}
	
	popd
	

	#########################
	# Prepare ota package
	#########################
	pushd "${out_folder}"
	if [ "1" == $ota_gen ]; then
		mkdir "${folder_target}/${ota_folder}/"

		# copy files
		cp "${tmp_folder}/${tp_target}/${ubl_tgt_hex}" "${folder_target}/${ota_folder}/${ubl_tgt_hex}"
		cp "${tmp_folder}/${tp_target}/${fw_tgt_hex}"  "${folder_target}/${ota_folder}/${fw_tgt_hex}"
		cp "${fepaddr_file}" "${folder_target}/${ota_folder}"

		# generate upgrade command
		fw_chm=$(sed -n "s/:04F80800\(\)/\1/p" "${folder_target}/${ota_folder}/${fw_tgt_hex}")
		fw_chm2="${fw_chm:6:2}${fw_chm:4:2}${fw_chm:2:2}${fw_chm:0:2}"
		ubl_chm=$(sed -n "s/:04F80400\(\)/\1/p" "${folder_target}/${ota_folder}/${ubl_tgt_hex}")
		ubl_chm2="${ubl_chm:6:2}${ubl_chm:4:2}${ubl_chm:2:2}${ubl_chm:0:2}"
		echo "#!/bin/sh" > "${folder_target}/${ota_folder}/${upgrade_cmd_file}"
		echo "./fwupdate.sh '${fw_tgt_hex}' '${ubl_tgt_hex}' '0x${fw_chm2}' '0x${ubl_chm2}'" >> "${folder_target}/${ota_folder}/${upgrade_cmd_file}"
		echo "UBL Checksum:0x${ubl_chm2}, FW Checksum:0x${fw_chm2}"

		# generate md5
		md5sum ${folder_target}/*/*.hex >> md5sum.txt
	fi

	
	#########################
	# finish
	#########################
	if [ "release" == ${tp_target} ]; then
		mkdir ${folder_target}/production_test
		pwd
		cp "${sdf_file}" "${folder_target}/production_test/sdf_${project}_v${sw_ver}.json"
	fi

	md5sum ${folder_target}/*.hex >> md5sum.txt
	popd
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
	

	#UBL Version
	if [ "" == "$ubl_prebuilt_hex" ]; then
		uver0=$(sed -n 's/#define *BL_MAJOR_VERSION *\(\)/\1/p' product.config)
		uver1=$(sed -n 's/#define *BL_MINOR_VERSION1 *\(\)/\1/p' product.config)
		uver2=$(sed -n 's/#define *BL_MINOR_VERSION2 *\(\)/\1/p' product.config)
		uver3=$(sed -n 's/#define *BL_MINOR_VERSION3 *\(\)/\1/p' product.config)

		#UBL Version may define to FW version
		[ "${uver0}" == "SW_MAJOR_VERSION" ]  && uver0=${ver0}
		[ "${uver1}" == "SW_MINOR_VERSION1" ] && uver1=${ver1}
		[ "${uver2}" == "SW_MINOR_VERSION2" ] && uver2=${ver2}
		[ "${uver3}" == "SW_MINOR_VERSION3" ] && uver3=${ver3}
		if [ "${uver3}" == "0" ]; then
			ubl_version=${uver0}.${uver1}.${uver2}
		else
			ubl_version=${uver0}.${uver1}.${uver2}.${uver3}	
		fi
		echo "UBL Version: ${ubl_version}"
	fi

	
	#PIU Version
	if [ "" == "$piu_prebuilt_hex" ]; then
		pver0=$(sed -n 's/#define *PIU_MAJOR_VERSION *\(\)/\1/p' product.config)
		pver1=$(sed -n 's/#define *PIU_MINOR_VERSION1 *\(\)/\1/p' product.config)
		pver2=$(sed -n 's/#define *PIU_MINOR_VERSION2 *\(\)/\1/p' product.config)
		pver3=$(sed -n 's/#define *PIU_MINOR_VERSION3 *\(\)/\1/p' product.config)

		#PIU Version may define to FW version
		[ "${pver0}" == "SW_MAJOR_VERSION" ]  && pver0=${ver0}
		[ "${pver1}" == "SW_MINOR_VERSION1" ] && pver1=${ver1}
		[ "${pver2}" == "SW_MINOR_VERSION2" ] && pver2=${ver2}
		[ "${pver3}" == "SW_MINOR_VERSION3" ] && pver3=${ver3}
		if [ "${pver3}" == "0" ]; then
			piu_version=${pver0}.${pver1}.${pver2}
		else
			piu_version=${pver0}.${pver1}.${pver2}.${pver3}	
		fi
		echo "PIU Version: ${piu_version}"
	fi


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
	pushd ${out_folder}
	dsp_ver=$(sed -n "s/.*${dsp_ver_param}.*_CONVERT *(\(\)/\1/p" "../${dsp_ver_header}" | sed 's/)//g')
	dsp_ver2=$(printf "%.2f" "${dsp_ver}")
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


