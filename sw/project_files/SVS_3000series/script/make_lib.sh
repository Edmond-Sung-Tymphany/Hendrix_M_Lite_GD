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
    fw_tgt_name=${project}_fw_swv${sw_ver}_${tp_target}
	fw_tgt_hex=${fw_tgt_name}.hex
	
	# need to build?
	if [ "1" != $build_en ]; then
		return
	fi
	
	echo "=============================================="
	echo ${package_name}
	echo "=============================================="

	


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
		if [ "1" == ${clean_en} ]; then
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

	cp iar/${tp_target}/list/piu.map ${out_folder}/${tmp_folder}/${tp_target}/
	
	echo "PIU build finish"

	#########################
	# Build FW
	#########################
	if [ "1" == ${clean_en} ]; then
		"${iar_path}" iar/${project}.ewp -clean ${tp_target}
	fi

	
	if [ "1" == "${disable_log}" ]; then
		"${iar_path}" iar/${project}.ewp -build ${tp_target} -parallel ${paralle_cpu} | grep -E '(Error|Total number of)' 
	else
		"${iar_path}" iar/${project}.ewp -build ${tp_target} -parallel ${paralle_cpu}
	fi

	cp iar/${tp_target}/exe/${fw_build_hex} ${out_folder}/${tmp_folder}/${tp_target}/${fw_tgt_hex}
	cp iar/${tp_target}/list/${project}.map ${out_folder}/${tmp_folder}/${tp_target}/
	
	echo "FW build finish"

	#########################
	# Pre-process hex
	#########################

	pushd ${out_folder}/${tmp_folder}/${tp_target}/

	# remove unused line containing "Start Linear Address"
	sed -i.org "/:04000005/d" *.hex

	# remove unused section
	sed -i "/00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/d" *.hex

	"${path_hexmate}/hexmate.exe" "${piu_tgt_hex}" "${fw_tgt_hex}" --edf=${path_hexmate}/en_msgs.txt -o${package_name}.hex -LOGFILE=${out_log}

    # generate md5
	md5sum ${fw_tgt_name}.hex  >> ${fw_tgt_name}.hex.md5
    md5sum ${package_name}.hex >> ${package_name}.hex.md5

	popd
	

	#########################
	# Prepare ota package
	#########################
	pushd "${out_folder}"
	if [ "1" == $ota_gen ]; then
		mkdir "${folder_target}/${ota_folder}/"

		# copy files
        cp ./${tmp_folder}/${tp_target}/${fw_tgt_name}.hex      ${folder_target}/${ota_folder}/
        cp ./${tmp_folder}/${tp_target}/${fw_tgt_name}.hex.md5  ${folder_target}/${ota_folder}/

        "${zip_path}" a ${folder_target}/${ota_folder}/${fw_tgt_name}.zip   ./${tmp_folder}/${tp_target}/${fw_tgt_name}.hex
        "${zip_path}" a ${folder_target}/${ota_folder}/${fw_tgt_name}.zip   ./${tmp_folder}/${tp_target}/${fw_tgt_name}.hex.md5

		# generate upgrade command
		#fw_chm=$(sed -n "s/:04F80800\(\)/\1/p" "${folder_target}/${ota_folder}/${fw_tgt_hex}")
		#fw_chm2="${fw_chm:6:2}${fw_chm:4:2}${fw_chm:2:2}${fw_chm:0:2}"

		#echo "#!/bin/sh" > "${folder_target}/${ota_folder}/${upgrade_cmd_file}"
		#echo "./fwupdate.sh '${fw_tgt_hex}' '${ubl_tgt_hex}' '0x${fw_chm2}' '0x${ubl_chm2}'" >> "${folder_target}/${ota_folder}/${upgrade_cmd_file}"
		#echo "UBL Checksum:0x${ubl_chm2}, FW Checksum:0x${fw_chm2}"

	fi

	
	#########################
	# finish
	#########################
	if [ "release" == ${tp_target} ]; then
		mkdir ${folder_target}/production_test
		pwd
		cp "${sdf_file}" "${folder_target}/production_test/sdf_${project}_v${sw_ver}.json"
	fi
    
    cp ${tmp_folder}/${tp_target}/${package_name}.hex        ./${folder_target}/
    cp ${tmp_folder}/${tp_target}/${package_name}.hex.md5    ./${folder_target}/
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
	ver0=$(sed -n 's/#define *SW_MAJOR_VERSION *\(\)/\1/p'  ${config_path})
	ver1=$(sed -n 's/#define *SW_MINOR_VERSION1 *\(\)/\1/p' ${config_path})
	ver2=$(sed -n 's/#define *SW_MINOR_VERSION2 *\(\)/\1/p' ${config_path})
	sw_version=${ver0}.${ver1}.${ver2}
	echo "FW Version: ${sw_version}"
		
	#PIU Version
	if [ "" == "$piu_prebuilt_hex" ]; then
		pver0=$(sed -n 's/#define *PIU_MAJOR_VERSION *\(\)/\1/p'  ${config_path})
		pver1=$(sed -n 's/#define *PIU_MINOR_VERSION1 *\(\)/\1/p' ${config_path})
		pver2=$(sed -n 's/#define *PIU_MINOR_VERSION2 *\(\)/\1/p' ${config_path})
		piu_version=${pver0}.${pver1}.${pver2}
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

	#########################
	# Prepare ble package
	#########################
	if [ "1" == ${build_release_en} ]; then
		mkdir "${out_folder}/${ble_folder}"
		cp ${ble_path}/* "${out_folder}/${ble_folder}/"   
	fi
	
	#dsp version
	pushd ${out_folder}
	mkdir "${dsp_folder}"
	
	cp ../${dsp_path}/*.dspproj		"${dsp_folder}/"
	cp ../${dsp_path}/*.dspproj.md5	"${dsp_folder}/"
	
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
	printf "     PIU: v${piu_version}  \r\n"
	
	if [ ${ota_debug_en} == "1" ] || [ ${ota_release_en} == "1" ]; then
		printf "     Checksum: ubl[${ubl_chm2}], fw[${fw_chm2}]  \r\n"		
	fi
	print_elapsed_time $timeStart $timeEnd
	printf "*******************************************************\r\n"
	
	read -n 1
}


