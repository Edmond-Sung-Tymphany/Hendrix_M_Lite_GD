#!/bin/sh
#*************************************************
# Automatic Build Script                         *
#                                                *
#*************************************************


#********************************
# Common Environment            *
#********************************
#path
iar_builder="C:/Program Files (x86)/IAR Systems/Embedded Workbench 7.2/common/bin/IarBuild.exe"
path_sed="sed"
#path_sed="/c/Program Files (x86)/Git/bin/sed.exe"
path_zip="../../../tool/7z.exe"
#path_zip="C:/Program Files (x86)/7-zip/7z.exe"
hexmate_path="../../../../../tool"
paralle_cpu=4

#files
fepaddr=../stm32/include/fep_addr.h
sdf=../conf/sdf.json
bootloader_status_hex="BnO_ca_info.hex"
upgrade_cmd_file="upgrade.sh"

#log files
tmp_log="hexmerger_t.log"
out_log="hexmerger.log"

#auto parameters
date=$(date "+%Y%m%d")




#********************************
# Function                      *
#********************************
function build_target() 
{
	#arguments
	sw_ver=$1
	ubl_ver=$2
	tp_target=$3
	folder_target=$4
	build_en=$5
	ota_gen=$6

	#File name
	package_name=${project}_hw${hw_ver}_swv${sw_ver}_${date}_${tp_target}
	ubl_name=${project}_ubl_swv${ubl_ver}_${tp_target}.hex
	fw_name=${project}_fw_swv${sw_ver}_${tp_target}.hex

	echo "=============================================="
	echo ${package_name}
	echo "=============================================="

	# need to build?
	if [ "1" != $build_en ]; then
		return
	fi

	# clean previous build
	if [ "1" == $clean_en ]; then
		"${iar_builder}" iar/piu.ewp -clean ${tp_target}
		"${iar_builder}" iar/ubl.ewp -clean ${tp_target}
		"${iar_builder}" iar/${project}.ewp -clean ${tp_target}
	fi

	# build
	"${iar_builder}" iar/piu.ewp -build ${tp_target} -parallel ${paralle_cpu}
	"${iar_builder}" iar/ubl.ewp -build ${tp_target} -parallel ${paralle_cpu}
	"${iar_builder}" iar/${project}.ewp -build ${tp_target} -parallel ${paralle_cpu}

	# prepare hex
	mkdir ${out_folder}/${tmp_folder}/${tp_target}
	cp iar/${tp_target}/exe/*.hex ${out_folder}/${tmp_folder}/${tp_target}/
	cp iar/${tp_target}/list/*.map ${out_folder}/${tmp_folder}/${tp_target}/


	pushd ${out_folder}/${tmp_folder}/${tp_target}/

	# remove unused line containing "Start Linear Address"
	"${path_sed}" -i "/:04000005/d" *.hex

	cp ${hexmate_path}/${bootloader_status_hex} ./
	"${hexmate_path}/hexmate.exe" "${piu_hex}" "${ubl_hex}" "${fw_hex}" --edf=${hexmate_path}/en_msgs.txt -o${tmp_hex} -LOGFILE=${tmp_log}
	"${hexmate_path}/hexmate.exe" "${tmp_hex}" "${bootloader_status_hex}" --edf=${hexmate_path}/en_msgs.txt -o${out_hex} -LOGFILE=${out_log}
	
	popd

	pushd "${out_folder}"
	mkdir "${folder_target}"
	
	# copy result
	cp "${tmp_folder}/${tp_target}/${out_hex}" "${folder_target}/${package_name}.hex"

	# copy ota
	if [ "1" == $ota_gen ]; then
		mkdir "${folder_target}/${ota_folder}/"
		
		# remove filled 0xFF
		"${path_sed}" -i.bak "/00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/d" "${tmp_folder}/${tp_target}/${ubl_hex}"
		"${path_sed}" -i.bak "/00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/d" "${tmp_folder}/${tp_target}/${fw_hex}"

		# copy files
		cp "${tmp_folder}/${tp_target}/${ubl_hex}" "${folder_target}/${ota_folder}/${ubl_name}"
		cp "${tmp_folder}/${tp_target}/${fw_hex}"  "${folder_target}/${ota_folder}/${fw_name}"
		cp "${fepaddr}" "${folder_target}/${ota_folder}"

		# generate upgrade command
		fw_chm=$(sed -n "s/:04F80800\(\)/\1/p" "${folder_target}/${ota_folder}/${fw_name}")
		fw_chm2="${fw_chm:6:2}${fw_chm:4:2}${fw_chm:2:2}${fw_chm:0:2}"
		ubl_chm=$(sed -n "s/:04F80400\(\)/\1/p" "${folder_target}/${ota_folder}/${ubl_name}")
		ubl_chm2="${ubl_chm:6:2}${ubl_chm:4:2}${ubl_chm:2:2}${ubl_chm:0:2}"
		echo "#!/bin/sh" > "${folder_target}/${ota_folder}/${upgrade_cmd_file}"
		echo "./fwupdate.sh '${fw_name}' '${ubl_name}' '0x${fw_chm2}' '0x${ubl_chm2}'" >> "${folder_target}/${ota_folder}/${upgrade_cmd_file}"
		echo "UBL Checksum:0x${ubl_chm2}, FW Checksum:0x${fw_chm2}"

		# generate md5
		md5sum ${folder_target}/*/*.hex >> md5sum.txt
	fi

	
	if [ "release" == ${tp_target} ]; then
		mkdir ${folder_target}/production_test
		cp "${sdf}" "${folder_target}/production_test/sdf_${project}_v${sw_ver}.json"
	fi

	# checksum
	md5sum ${folder_target}/*.hex >> md5sum.txt
	popd
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
	

	#Folder name
	out_folder="${project}_hw${hw_ver}_swv${sw_version}_${date}"


	#Create folder
	rm -f "${out_folder}*.zip"
	rm -rf "${out_folder}"
	mkdir "${out_folder}"
	mkdir "${out_folder}/${tmp_folder}"

	#Build target
	#                                                                   build enable        ota enable:
	build_target ${sw_version} ${ubl_version} "debug"   "development"  ${build_debug_en}    ${ota_debug_en}
	build_target ${sw_version} ${ubl_version} "release" "production"   ${build_release_en}  ${ota_release_en}

	#zip files	
	if [ ${zip_en} == "1" ]; then
		pushd ${out_folder}
		"${path_zip}" a "../${project}_hw${hw_ver}_swv${sw_version}_${date}.zip" $(ls -I $tmp_folder)
		"${path_zip}" a "../${project}_hw${hw_ver}_swv${sw_version}_${date}_backup.zip" "./*"
		popd
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
	printf "     UBL: v${ubl_version}  \r\n"
	if [ ${ota_debug_en} == "1" ] || [ ${ota_release_en} == "1" ]; then
		printf "     Checksum: ubl[${ubl_chm2}], fw[${fw_chm2}]  \r\n"		
	fi
	print_elapsed_time $timeStart $timeEnd
	printf "*******************************************************\r\n"
}


