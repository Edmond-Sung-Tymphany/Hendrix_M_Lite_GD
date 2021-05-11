#!/bin/sh
#*************************************************
# Automatic Build Script - Setting file          *
#                                                *
#*************************************************

#********************************
# Common Environment            *
#********************************
#paths
iar_path="C:/Program Files (x86)/IAR Systems/Embedded Workbench 8.4/common/bin/IarBuild.exe"
zip_path="../../../../tool/7z.exe"

#auto parameters
date=$(date "+%Y%m%d")

#other
paralle_cpu=4



#********************************
# Functions                     *
#********************************
function printmsg()
{
	echo "=============================================="
	echo $1
	echo "=============================================="
}

function build_target() 
{
	#########################
	# arguments
	#########################
	sw_ver=$1
	tp_target=$2
	build_en=$3

	if [ "1" != $build_en ]; then
	    return
	fi
	
	#File name
	if [ "1" == ${dsp_auto_build} ]; then
		sw_ver=${sw_ver}_dsp_test
	fi

	package_name=${project}_${model}_hw${hw_ver}_swv${sw_ver}_${date}
	fw_tgt_hex=${project}_${model}_fw_swv${sw_ver}_${date}.hex

	
	printmsg ${package_name}
	
	
	#########################
	# Build FW
	#########################
	if [ "1" == $clean_en ]; then
		"${iar_path}" "${ewp_target}" -clean ${tp_target}
	fi

	
	if [ "1" == "${disable_log}" ]; then
		"${iar_path}" "${ewp_target}" -build ${tp_target} -parallel ${paralle_cpu} | grep -E '(Error|Total number of)' 
	else
		"${iar_path}" "${ewp_target}" -build ${tp_target} -parallel ${paralle_cpu}
	fi

	printmsg "Copy Hex to Release Folder"
	
	cp ./iar/${tp_target}/Exe/*.hex ${release_folder}/${out_folder}/MCU/${fw_tgt_hex}

    printmsg "Checksum"
	
	#########################
	# finish
	#########################

    md5sum ${release_folder}/${out_folder}/MCU/${fw_tgt_hex} >> ${release_folder}/${out_folder}/MCU/${fw_tgt_hex}.MD5.txt

	

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

    printmsg "$date"
	
	#FW Version
	printmsg "Parsing software version ..."

	ver0=$(sed -n 's/#define *SW_MAJOR_VERSION *\(\)/\1/p'  model_config.h)
	ver1=$(sed -n 's/#define *SW_MINOR_VERSION1 *\(\)/\1/p' model_config.h)
	ver2=$(sed -n 's/#define *SW_MINOR_VERSION2 *\(\)/\1/p' model_config.h)
	ver3=$(sed -n 's/#define *SW_MINOR_VERSION3 *\(\)/\1/p' model_config.h)
	if [ "${ver3}" == "0" ]; then
		sw_version=${ver0}.${ver1}.${ver2}
	else
		sw_version=${ver0}.${ver1}.${ver2}.${ver3}	
	fi
	printmsg "FW Version: ${sw_version}"
	
    pushd ${release_folder}/

	#Folder name
	out_folder="${project}_${model}_hw${hw_ver}_swv${sw_version}_${date}"

    printmsg "Create Folder"
	#Create folder
	rm -f package.zip
	rm -f "${out_folder}*.zip"
	rm -rf "${out_folder}"
	mkdir "${out_folder}"

	if [ ${bt_en} == "1" ] ; then
    mkdir "${out_folder}/BT"
	cp -rf ../BT_PS_Key/ "${out_folder}/BT/"
	fi
	
	if [ ${dsp_en} == "1" ] ; then
    mkdir "${out_folder}/DSP"
	cp ../DSP/*.dspproj "${out_folder}/DSP/"
	fi
	if [ ${pte_en} == "1" ] ; then
    mkdir "${out_folder}/Document"
	cp *.docx "${out_folder}/Document/"
	cp ../conf/*.json "${out_folder}/Document/"
	#cp ../conf/CMD.tp "${out_folder}/Document/"
	fi
	
	mkdir "${out_folder}/MCU"
	cp ${project}_${model}_Notes/*.xlsx "${out_folder}/"

	
	popd
	
	printmsg "Build Debug Target"

	#Build target                          build enable
	build_target ${sw_version} "debug"    ${build_debug_en}
	
	printmsg "Build Release Target"
	
	build_target ${sw_version} "release"  ${build_release_en}

	printmsg "GIT-SHA"
	
	store_git_sha "${release_folder}/${out_folder}/git_sha_id.txt"


	#dsp version
	pushd ${release_folder}/${out_folder}/
	
    printmsg "Zip Package"
	
	#zip files	
	if [ ${zip_en} == "1" ] ; then
		"${zip_path}" a "../${project}_${model}_hw${hw_ver}_swv${sw_version}_${date}.zip" ./
		md5sum ../${project}_${model}_hw${hw_ver}_swv${sw_version}_${date}.zip >> ../${project}_${model}_hw${hw_ver}_swv${sw_version}_${date}.zip.MD5.txt
	fi

	#Print time
	date
	timeEnd=$(date +%s)
	printf "\r\n"
	printf "\r\n"
	printf "*******************************************************\r\n"
	printf "  Build successful         \r\n"
	printf "     Name: ${out_folder}   \r\n"
	printf "     DSP: v${dsp_ver2} \r\n"

	print_elapsed_time $timeStart $timeEnd
	printf "*******************************************************\r\n"
	
}



