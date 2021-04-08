@echo off
::=================================
:: Parameter
::=================================
::Copy object files to out folder
set PARAM_CP_OBJ=0

::Generate archive file
set PARAM_GEN_ARCHIVE=0


::======================================
:: Get project and tool information
::======================================
if not exist parm_proj_info.bat echo *** [gen_tym_bundle.bat] parm_proj_info.bat is not exist *** & goto FINISH
call "parm_proj_info.bat"
call "%~dp0\parm_tools.bat"
if not "%RET_PARM_TOOLS%"=="1" echo *** [gen_tym_bundle.bat] parm_tools.bat is fail *** & goto FINISH

::Tool check
if not exist "%PATH_PYTHON%\python.exe" echo *** [gen_tym_bundle.bat] Please install python v2.7 to %PATH_PYTHON%\ *** & goto FINISH
if not exist "%PATH_BASH%\bash.exe"     echo *** [gen_tym_bundle.bat] Please install cygwin to %PATH_BASH%\ *** & goto FINISH

::gen_tym_bundle.bat need these variables:
if "%PATH_BASE%"==""     echo *** Do not call gen_tym_bundle.bat directly (PATH_BASE is empty) ***    & goto FINISH
if "%PRJ_NAME%"==""      echo *** Do not call gen_tym_bundle.bat directly (PRJ_NAME is empty) ***     & goto FINISH
if "%MCU_VER%"==""       echo *** Do not call gen_tym_bundle.bat directly (MCU_VER is empty) ***      & goto FINISH
if "%SAM_VER%"==""       echo *** Do not call gen_tym_bundle.bat directly (SAM_VER is empty) ***      & goto FINISH
if "%SAM_THEME%"==""     echo *** Do not call gen_tym_bundle.bat directly (SAM_THEME is empty) ***    & goto FINISH
if "%HW_VER%"==""        echo *** Do not call gen_tym_bundle.bat directly (HW_VER is empty) ***       & goto FINISH
if "%PATH_DTB%"==""      echo *** Do not call gen_tym_bundle.bat directly (PATH_DTB is empty) ***     & goto FINISH
if "%PATH_PRJ_AP%"==""   echo *** Do not call gen_tym_bundle.bat directly (PATH_PRJ_AP is empty) ***  & goto FINISH
if "%PATH_PRJ_BL%"==""   echo *** Do not call gen_tym_bundle.bat directly (PATH_PRJ_BL is empty) ***  & goto FINISH
if "%DTB_MANUFA%"==""    echo *** Do not call gen_tym_bundle.bat directly (DTB_MANUFA is empty) ***   & goto FINISH
if "%DTB_DEVICE%"==""    echo *** Do not call gen_tym_bundle.bat directly (DTB_DEVICE is empty) ***   & goto FINISH
if "%PATH_CERT%"==""     echo *** Do not call gen_tym_bundle.bat directly (PATH_CERT is empty) ***    & goto FINISH
if "%NAME_BL%"==""       echo *** Do not call gen_tym_bundle.bat directly (NAME_BL is empty) ***      & goto FINISH

if "%ENA_BUNDLE_DEBUG_INCLUDE_SAM%"=="" echo *** Do not call gen_tym_bundle.bat directly (ENA_BUNDLE_DEBUG_INCLUDE_SAM is empty) ***  & goto FINISH
if "%ENA_BUNDLE_DEBUG_INCLUDE_MCU%"=="" echo *** Do not call gen_tym_bundle.bat directly (ENA_BUNDLE_DEBUG_INCLUDE_MCU is empty) ***  & goto FINISH
if "%ENA_BUNDLE_DEBUG_INCLUDE_DTB%"=="" echo *** Do not call gen_tym_bundle.bat directly (ENA_BUNDLE_DEBUG_INCLUDE_DTB is empty) ***  & goto FINISH
if "%ENA_BUNDLE_DBG_R_INCLUDE_SAM%"=="" echo *** Do not call gen_tym_bundle.bat directly (ENA_BUNDLE_DBG_R_INCLUDE_SAM is empty) ***  & goto FINISH
if "%ENA_BUNDLE_DBG_R_INCLUDE_MCU%"=="" echo *** Do not call gen_tym_bundle.bat directly (ENA_BUNDLE_DBG_R_INCLUDE_MCU is empty) ***  & goto FINISH
if "%ENA_BUNDLE_DBG_R_INCLUDE_DTB%"=="" echo *** Do not call gen_tym_bundle.bat directly (ENA_BUNDLE_DBG_R_INCLUDE_DTB is empty) ***  & goto FINISH
if "%ENA_BUNDLE_RELEA_INCLUDE_SAM%"=="" echo *** Do not call gen_tym_bundle.bat directly (ENA_BUNDLE_RELEA_INCLUDE_SAM is empty) ***  & goto FINISH
if "%ENA_BUNDLE_RELEA_INCLUDE_MCU%"=="" echo *** Do not call gen_tym_bundle.bat directly (ENA_BUNDLE_RELEA_INCLUDE_MCU is empty) ***  & goto FINISH
if "%ENA_BUNDLE_RELEA_INCLUDE_DTB%"=="" echo *** Do not call gen_tym_bundle.bat directly (ENA_BUNDLE_RELEA_INCLUDE_DTB is empty) ***  & goto FINISH
if "%ENA_GEN_BUNDLE_DEBUG%"==""   echo *** Do not call gen_tym_bundle.bat directly (ENA_GEN_BUNDLE_DEBUG is empty) ***  & goto FINISH
if "%ENA_GEN_BUNDLE_DBG_R%"==""   echo *** Do not call gen_tym_bundle.bat directly (ENA_GEN_BUNDLE_DBG_R is empty) ***  & goto FINISH
if "%ENA_GEN_BUNDLE_RELEA%"==""   echo *** Do not call gen_tym_bundle.bat directly (ENA_GEN_BUNDLE_RELEA is empty) ***  & goto FINISH
if "%ENA_GEN_DTB_DEBUG%"==""      echo *** Do not call gen_tym_bundle.bat directly (ENA_GEN_DTB_DEBUG is empty) ***  & goto FINISH
if "%ENA_GEN_DTB_DBG_R%"==""      echo *** Do not call gen_tym_bundle.bat directly (ENA_GEN_DTB_DBG_R is empty) ***  & goto FINISH
if "%ENA_GEN_DTB_RELEA%"==""      echo *** Do not call gen_tym_bundle.bat directly (ENA_GEN_DTB_RELEA is empty) ***  & goto FINISH
if "%ENA_GEN_DTB_DEBUG%"=="1" if "%DTB_XML_FN_DEBUG%"==""  echo *** Do not call gen_tym_bundle.bat directly (DTB_XML_FN_DEBUG is empty) ***  & goto FINISH
if "%ENA_GEN_DTB_DBG_R%"=="1" if "%DTB_XML_FN_DBG_R%"==""  echo *** Do not call gen_tym_bundle.bat directly (DTB_XML_FN_DBG_R is empty) ***  & goto FINISH
if "%ENA_GEN_DTB_RELEA%"=="1" if "%DTB_XML_FN_RELEA%"==""  echo *** Do not call gen_tym_bundle.bat directly (DTB_XML_FN_RELEA is empty) ***  & goto FINISH
if "%ENA_GEN_DTB_DEBUG%"=="1" if "%UPDATE_IMG_URL_BASE_DEBUG%"=="" echo *** Do not call gen_tym_bundle.bat directly (UPDATE_IMG_URL_BASE_DEBUG is empty) *** & goto FINISH
if "%ENA_GEN_DTB_DBG_R%"=="1" if "%UPDATE_IMG_URL_BASE_DBG_R%"=="" echo *** Do not call gen_tym_bundle.bat directly (UPDATE_IMG_URL_BASE_DBG_R is empty) *** & goto FINISH
if "%ENA_GEN_DTB_RELEA%"=="1" if "%UPDATE_IMG_URL_BASE_RELEA%"=="" echo *** Do not call gen_tym_bundle.bat directly (UPDATE_IMG_URL_BASE_RELEA is empty) *** & goto FINISH
if "%ENA_GEN_DTB_DEBUG%"=="1" if "%DTB_VER_DEBUG%"==""  echo *** Do not call gen_tym_bundle.bat directly (DTB_VER_DEBUG is empty) ***   & goto FINISH
if "%ENA_GEN_DTB_DBG_R%"=="1" if "%DTB_VER_DBG_R%"==""  echo *** Do not call gen_tym_bundle.bat directly (DTB_VER_DBG_R is empty) ***   & goto FINISH
if "%ENA_GEN_DTB_RELEA%"=="1" if "%DTB_VER_RELEA%"==""  echo *** Do not call gen_tym_bundle.bat directly (DTB_VER_RELEA is empty) ***   & goto FINISH
if "%ENA_GEN_DTB_DEBUG%"=="1" if "%MAIN_FN_DTB_DEBUG_TGT%"==""  echo *** Do not call gen_tym_bundle.bat directly (MAIN_FN_DTB_DEBUG_TGT is empty) ***   & goto FINISH
if "%ENA_GEN_DTB_DBG_R%"=="1" if "%MAIN_FN_DTB_DBG_R_TGT%"==""  echo *** Do not call gen_tym_bundle.bat directly (MAIN_FN_DTB_DBG_R_TGT is empty) ***   & goto FINISH
if "%ENA_GEN_DTB_RELEA%"=="1" if "%MAIN_FN_DTB_RELEA_TGT%"==""  echo *** Do not call gen_tym_bundle.bat directly (MAIN_FN_DTB_RELEA_TGT is empty) ***   & goto FINISH



::=================================
:: Global Variable
::=================================
::Build Result
set RET_GEN_BUNDLE=0

:Date String, ex.20141128
%PATH_BASH%\date +%%Y%%m%%d > tmp.txt
set DATE_STR=
FOR /F %%i in (tmp.txt) do set DATE_STR=%%i
rm -f tmp.txt

::Path
set TGT_NAME=%PRJ_NAME%_mcu_hwv%HW_VER%_swv%MCU_VER%_%DATE_STR%
set OUT_FOLDER=out_Bundle
set DIR_OUT=%PATH_BASE%\%OUT_FOLDER%
set DIR_OUT_DEBUG=%DIR_OUT%\%TGT_NAME%\DEBUG
set DIR_OUT_DBG_R=%DIR_OUT%\%TGT_NAME%\DEBUG\Repeated_Upgrade_Test
set DIR_OUT_RELEA=%DIR_OUT%\%TGT_NAME%\RELEASE
set DIR_OUT_INFO=%DIR_OUT%\dbg_info
set DIR_RELEA_BL=%PATH_PRJ_BL%\dist\release\production
set DIR_DEBUG_BL=%PATH_PRJ_BL%\dist\debug\production
set DIR_RELEA_AP=%PATH_PRJ_AP%\dist\release\production
set DIR_DEBUG_AP=%PATH_PRJ_AP%\dist\debug\production

::Output files
set PATH_TMP_BINHEX_DEBUG=%DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.binhex
set PATH_TMP_BINHEX_DBG_R=%PATH_TMP_BINHEX_DEBUG%
set PATH_TMP_BINHEX_RELEA=%DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.binhex

::DTB
set PATH_DTB_DEBUG_TMP=%DIR_OUT_INFO%\dtb\%MAIN_FN_DTB_DEBUG_TGT%
set PATH_DTB_DBG_R_TMP=%DIR_OUT_INFO%\dtb\%MAIN_FN_DTB_DBG_R_TGT%
set PATH_DTB_RELEA_TMP=%DIR_OUT_INFO%\dtb\%MAIN_FN_DTB_RELEA_TGT%
set PATH_DTB_DEBUG_TGT=%DIR_OUT_DEBUG%\%MAIN_FN_DTB_DEBUG_TGT%
set PATH_DTB_DBG_R_TGT=%DIR_OUT_DBG_R%\%MAIN_FN_DTB_DBG_R_TGT%
set PATH_DTB_RELEA_TGT=%DIR_OUT_RELEA%\%MAIN_FN_DTB_RELEA_TGT%

::Bootloader image
set PATH_HEX_BL_RELEA=%DIR_RELEA_BL%\%NAME_BL%.production.hex
set PATH_HEX_BL_DEBUG=%DIR_DEBUG_BL%\%NAME_BL%.production.hex
set PATH_HEX_BL_RELEA_TMP=%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_release.hex
set PATH_HEX_BL_DEBUG_TMP=%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_debug.hex

::Application image
set PATH_HEX_AP_DEBUG=%DIR_DEBUG_AP%\mplab.production.hex
set PATH_HEX_AP_DBG_R=%PATH_HEX_AP_DEBUG%
set PATH_HEX_AP_RELEA=%DIR_RELEA_AP%\mplab.production.hex

::Final image
set PATH_HEX_DEBUG=%DIR_DEBUG_AP%\mplab.production.unified.hex
set PATH_HEX_RELEA=%DIR_RELEA_AP%\mplab.production.unified.hex
set PATH_HEX_DEBUG_TGT=%DIR_OUT_DEBUG%\%TGT_NAME%_debug.hex
set PATH_HEX_RELEA_TGT=%DIR_OUT_RELEA%\%TGT_NAME%_release.hex
set PATH_HEX_AP_DEBUG_TMP=%DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.hex
set PATH_HEX_AP_DBG_R_TMP=%PATH_HEX_AP_DEBUG_TMP%
set PATH_HEX_AP_RELEA_TMP=%DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.hex

:SAM
set SAM=%~dp0/sam_v%SAM_VER%/openwrt-ar71xx-generic-cus227-firmware-v%SAM_VER%-%SAM_THEME%

:Output
set TGZ_OUT_PATH=%PATH_BASE%
set TGZ_TGT_NAME=%OUT_FOLDER%.tgz


::=================================
:: Environment Check
::=================================
if not exist "%PATH_HEX_BL_DEBUG%"  echo *** File not found: %PATH_HEX_BL_DEBUG% *** & goto FINISH_RM_TMP
if not exist "%PATH_HEX_AP_DEBUG%"  echo *** File not found: %PATH_HEX_AP_DEBUG% *** & goto FINISH_RM_TMP
if not exist "%PATH_HEX_BL_RELEA%"  echo *** File not found: %PATH_HEX_BL_RELEA% *** & goto FINISH_RM_TMP
if not exist "%PATH_HEX_AP_RELEA%"  echo *** File not found: %PATH_HEX_AP_RELEA% *** & goto FINISH_RM_TMP
if not exist "%PATH_HEX_DEBUG%"     echo *** File not found: %PATH_HEX_DEBUG%    *** & goto FINISH_RM_TMP
if not exist "%PATH_HEX_RELEA%"     echo *** File not found: %PATH_HEX_RELEA%    *** & goto FINISH_RM_TMP




::=================================
:: Initialization
::=================================
rm -rf "%TGZ_OUT_PATH%\%TGZ_TGT_NAME%" "%DIR_OUT%" "%DIR_OUT_INFO%"
mkdir "%DIR_OUT%" "%DIR_OUT_INFO%" "%DIR_OUT_INFO%\bootloader" "%DIR_OUT_INFO%\app" "%DIR_OUT_INFO%\bundle" "%DIR_OUT_INFO%\dtb" "%DIR_OUT_RELEA%" "%DIR_OUT_DEBUG%" 
if "%ENA_GEN_BUNDLE_DBG_R%"=="1" mkdir "%DIR_OUT_DBG_R%"
if "%ENA_GEN_DTB_DBG_R%"=="1" if not exist "%DIR_OUT_DBG_R%"  mkdir "%DIR_OUT_DBG_R%"


pushd %~dp0

::=================================================
:: Generate dtb.sam
::=================================================
::debug, normal, get returned dtb version on DTB_VER_DEBUG
::Note DTB_VER_DEBUG have additional white space on tail, we need to remove it manually
call:FunGenDtb "%MAIN_FN_DTB_DEBUG_TGT%" "%DTB_MAIN_FN_DEBUG%" "%PATH_DTB_DEBUG_TMP%" "%PATH_DTB_DEBUG_TGT%" "%ENA_GEN_DTB_DEBUG%" "%DTB_VER_DEBUG%"
if not "%RET%"=="1" goto FINISH_RM_TMP

::debug, repeated upgrade, get returned dtb version on DTB_VER_DBG_R
::Note DTB_VER_DBG_R have additional white space on tail, we need to remove it manually
call:FunGenDtb "%MAIN_FN_DTB_DBG_R_TGT%" "%DTB_MAIN_FN_DBG_R%" "%PATH_DTB_DBG_R_TMP%" "%PATH_DTB_DBG_R_TGT%" "%ENA_GEN_DTB_DBG_R%" "%DTB_VER_DBG_R%"
if not "%RET%"=="1" goto FINISH_RM_TMP

::release, get returned dtb version on DTB_VER_RELEA
::Note DTB_VER_RELEA have additional white space on tail, we need to remove it manually
call:FunGenDtb "%MAIN_FN_DTB_RELEA_TGT%" "%DTB_MAIN_FN_RELEA%" "%PATH_DTB_RELEA_TMP%" "%PATH_DTB_RELEA_TGT%" "%ENA_GEN_DTB_RELEA%" "%DTB_VER_RELEA%"
if not "%RET%"=="1" goto FINISH_RM_TMP



::=============================================================
:: Generate bundle file
::=============================================================
::debug, normal. Return value
 call:FunGenBundle "debug"         ""   "%ENA_GEN_BUNDLE_DEBUG%" "%DTB_XML_FN_DEBUG%" "%PATH_TMP_BINHEX_DEBUG%" "%PATH_HEX_AP_DEBUG%" "%DIR_OUT_DEBUG%" "%UPDATE_IMG_URL_BASE_DEBUG%" "%ENA_GEN_DTB_DEBUG%" "%PATH_DTB_DEBUG_TMP%" "%DTB_VER_DEBUG%" "%ENA_BUNDLE_DEBUG_INCLUDE_SAM%" "%ENA_BUNDLE_DEBUG_INCLUDE_MCU%" "%ENA_BUNDLE_DEBUG_INCLUDE_DTB%"
 if not "%RET%"=="1" goto FINISH_RM_TMP
 set FN_BUNDLE_DEBUG=%LOCAL_FN_BUNDLE%

::debug, repetaed upgrade
call:FunGenBundle "debug_repeated" "-R" "%ENA_GEN_BUNDLE_DBG_R%" "%DTB_XML_FN_DBG_R%" "%PATH_TMP_BINHEX_DBG_R%" "%PATH_HEX_AP_DBG_R%" "%DIR_OUT_DBG_R%" "%UPDATE_IMG_URL_BASE_DBG_R%" "%ENA_GEN_DTB_DBG_R%" "%PATH_DTB_DBG_R_TMP%" "%DTB_VER_DBG_R%" "%ENA_BUNDLE_DBG_R_INCLUDE_SAM%" "%ENA_BUNDLE_DBG_R_INCLUDE_MCU%" "%ENA_BUNDLE_DBG_R_INCLUDE_DTB%"
if not "%RET%"=="1" goto FINISH_RM_TMP
set FN_BUNDLE_DBG_R=%LOCAL_FN_BUNDLE%

::release 
call:FunGenBundle "release"        ""   "%ENA_GEN_BUNDLE_RELEA%" "%DTB_XML_FN_RELEA%" "%PATH_TMP_BINHEX_RELEA%" "%PATH_HEX_AP_RELEA%" "%DIR_OUT_RELEA%" "%UPDATE_IMG_URL_BASE_RELEA%" "%ENA_GEN_DTB_RELEA%" "%PATH_DTB_RELEA_TMP%" "%DTB_VER_RELEA%" "%ENA_BUNDLE_RELEA_INCLUDE_SAM%" "%ENA_BUNDLE_RELEA_INCLUDE_MCU%" "%ENA_BUNDLE_RELEA_INCLUDE_DTB%"
if not "%RET%"=="1" goto FINISH_RM_TMP
set FN_BUNDLE_RELEA=%LOCAL_FN_BUNDLE%

popd


::=================================================
:: Prepare Information files
::=================================================
::Bundle summary file
if exist "%DIR_OUT_DEBUG%\*.dtb.sam.summary" move "%DIR_OUT_DEBUG%\*.dtb.sam.summary" "%DIR_OUT_INFO%\dtb\"
if exist "%DIR_OUT_DBG_R%\*.dtb.sam.summary" move "%DIR_OUT_DBG_R%\*.dtb.sam.summary" "%DIR_OUT_INFO%\dtb\"
if exist "%DIR_OUT_RELEA%\*.dtb.sam.summary" move "%DIR_OUT_RELEA%\*.dtb.sam.summary" "%DIR_OUT_INFO%\dtb\"
if exist "%DIR_OUT_DEBUG%\*bundle*.summary" move "%DIR_OUT_DEBUG%\*bundle*.summary" "%DIR_OUT_INFO%\bundle\"
if exist "%DIR_OUT_DBG_R%\*bundle*.summary" move "%DIR_OUT_DBG_R%\*bundle*.summary" "%DIR_OUT_INFO%\bundle\"
if exist "%DIR_OUT_RELEA%\*bundle*.summary" move "%DIR_OUT_RELEA%\*bundle*.summary" "%DIR_OUT_INFO%\bundle\"

::Images
copy /Y "%PATH_HEX_BL_DEBUG%" "%PATH_HEX_BL_DEBUG_TMP%"
copy /Y "%PATH_HEX_AP_DEBUG%" "%PATH_HEX_AP_DEBUG_TMP%"
copy /Y "%PATH_HEX_BL_RELEA%" "%PATH_HEX_BL_RELEA_TMP%"
copy /Y "%PATH_HEX_AP_RELEA%" "%PATH_HEX_AP_RELEA_TMP%"
copy /Y "%PATH_HEX_DEBUG%"    "%PATH_HEX_DEBUG_TGT%"
copy /Y "%PATH_HEX_RELEA%"    "%PATH_HEX_RELEA_TGT%"

if not exist "%PATH_HEX_BL_DEBUG_TMP%" echo *** Fail to copy to: %PATH_HEX_BL_DEBUG_TMP% *** & goto FINISH_RM_TMP
if not exist "%PATH_HEX_AP_DEBUG_TMP%" echo *** Fail to copy to: %PATH_HEX_AP_DEBUG_TMP% *** & goto FINISH_RM_TMP
if not exist "%PATH_HEX_BL_RELEA_TMP%" echo *** Fail to copy to: %PATH_HEX_BL_RELEA_TMP% *** & goto FINISH_RM_TMP
if not exist "%PATH_HEX_AP_RELEA_TMP%" echo *** Fail to copy to: %PATH_HEX_AP_RELEA_TMP% *** & goto FINISH_RM_TMP
if not exist "%PATH_HEX_DEBUG_TGT%"    echo *** Fail to copy to: %PATH_HEX_DEBUG_TGT% ***    & goto FINISH_RM_TMP
if not exist "%PATH_HEX_RELEA_TGT%"    echo *** Fail to copy to: %PATH_HEX_RELEA_TGT% ***    & goto FINISH_RM_TMP


::Application files
copy "%DIR_RELEA_AP%\mplab.production.asm" "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.asm"
copy "%DIR_RELEA_AP%\mplab.production.elf" "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.elf"
copy "%DIR_RELEA_AP%\mplab.production.map" "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.map"
copy "%DIR_DEBUG_AP%\mplab.production.asm" "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.asm"
copy "%DIR_DEBUG_AP%\mplab.production.elf" "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.elf"
copy "%DIR_DEBUG_AP%\mplab.production.map" "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.map"

if not exist "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.asm" echo *** Fail to copy to: %DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.asm *** & goto FINISH_RM_TMP
if not exist "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.elf" echo *** Fail to copy to: %DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.elf *** & goto FINISH_RM_TMP
if not exist "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.map" echo *** Fail to copy to: %DIR_OUT_INFO%\app\swv%MCU_VER%_app_release.map *** & goto FINISH_RM_TMP
if not exist "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.asm"   echo *** Fail to copy to: %DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.asm *** & goto FINISH_RM_TMP
if not exist "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.elf"   echo *** Fail to copy to: %DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.elf *** & goto FINISH_RM_TMP
if not exist "%DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.map"   echo *** Fail to copy to: %DIR_OUT_INFO%\app\swv%MCU_VER%_app_debug.map *** & goto FINISH_RM_TMP


::Booloader files
copy "%DIR_RELEA_BL%\%NAME_BL%.production.asm" "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_release.asm" 
copy "%DIR_RELEA_BL%\%NAME_BL%.production.elf" "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_release.elf" 
copy "%DIR_RELEA_BL%\%NAME_BL%.production.map" "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_release.map"
copy "%DIR_DEBUG_BL%\%NAME_BL%.production.asm" "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_debug.asm"
copy "%DIR_DEBUG_BL%\%NAME_BL%.production.elf" "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_debug.elf" 
copy "%DIR_DEBUG_BL%\%NAME_BL%.production.map" "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_debug.map"

if not exist "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_release.asm" echo *** Fail to copy to: %DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_release.asm *** & goto FINISH_RM_TMP
if not exist "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_release.elf" echo *** Fail to copy to: %DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_release.elf *** & goto FINISH_RM_TMP
if not exist "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_release.map" echo *** Fail to copy to: %DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_release.map *** & goto FINISH_RM_TMP
if not exist "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_debug.asm"   echo *** Fail to copy to: %DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_debug.asm ***   & goto FINISH_RM_TMP
if not exist "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_debug.elf"   echo *** Fail to copy to: %DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_debug.elf ***   & goto FINISH_RM_TMP
if not exist "%DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_debug.map"   echo *** Fail to copy to: %DIR_OUT_INFO%\bootloader\swv%MCU_VER%_bl_debug.map ***   & goto FINISH_RM_TMP


::Copy object files
if "%PARAM_CP_OBJ%"=="1" (
	cp -rf  "%PATH_PRJ_BL%\build"  "%DIR_OUT_INFO%\bootloader\"
	cp -rf  "%PATH_PRJ_AP%\build"  "%DIR_OUT_INFO%\app\"
	if not exist "%DIR_OUT_INFO%\bootloader\build" echo [gen_tym_bundle.bat] Fail to copy object files to %DIR_OUT_INFO%\bootloader\build & goto FINISH_RM_TMP
	if not exist "%DIR_OUT_INFO%\app\build" echo [gen_tym_bundle.bat] Fail to copy object files to %DIR_OUT_INFO%\bootloader\app & goto FINISH_RM_TMP
)


::Generate archive file
if "%PARAM_GEN_ARCHIVE%"=="1" (
	if not "%DIR_OUT%"=="" echo del /Q /S "%DIR_OUT%\*.mft"
	if not "%DIR_OUT%"=="" del /Q /S "%DIR_OUT%\*.mft"
	pushd "%TGZ_OUT_PATH%"
	tar -zcvf "%TGZ_TGT_NAME%" "%OUT_FOLDER%/" > tmp.txt
	rm -f tmp.txt
	if not exist "%TGZ_TGT_NAME%" echo [gen_tym_bundle.bat] Fail to generate %TGZ_OUT_PATH%\%TGZ_TGT_NAME% & goto FINISH_RM_TMP
	popd
)


::=================================
:: Finish
::=================================
set RET_GEN_BUNDLE=1

:FINISH_RM_TMP
if not "%DIR_OUT%"=="" echo del /Q /S "%DIR_OUT%\*.mft"
if not "%DIR_OUT%"=="" del /Q /S "%DIR_OUT%\*.mft"

:FINISH
echo.
echo.
echo /-------------------------------------------------------------------------\
echo .  %date% %time:~0,8%  

set BUDNEL_DEBUG_INCLUDE=
if "%ENA_BUNDLE_DEBUG_INCLUDE_SAM%"=="1"  set BUDNEL_DEBUG_INCLUDE=%BUDNEL_DEBUG_INCLUDE% SAM,
if "%ENA_BUNDLE_DEBUG_INCLUDE_MCU%"=="1"  set BUDNEL_DEBUG_INCLUDE=%BUDNEL_DEBUG_INCLUDE% MCU
if "%ENA_BUNDLE_DEBUG_INCLUDE_DTB%"=="1"  set BUDNEL_DEBUG_INCLUDE=%BUDNEL_DEBUG_INCLUDE% DTB,

set BUDNEL_DBG_R_INCLUDE=
if "%ENA_BUNDLE_DBG_R_INCLUDE_DTB%"=="1"  set BUDNEL_DBG_R_INCLUDE=%BUDNEL_DBG_R_INCLUDE% DTB,
if "%ENA_BUNDLE_DBG_R_INCLUDE_SAM%"=="1"  set BUDNEL_DBG_R_INCLUDE=%BUDNEL_DBG_R_INCLUDE% SAM,
if "%ENA_BUNDLE_DBG_R_INCLUDE_MCU%"=="1"  set BUDNEL_DBG_R_INCLUDE=%BUDNEL_DBG_R_INCLUDE% MCU

set BUDNEL_RELEA_INCLUDE=
if "%ENA_BUNDLE_RELEA_INCLUDE_DTB%"=="1"  set BUDNEL_RELEA_INCLUDE=%BUDNEL_RELEA_INCLUDE% DTB,
if "%ENA_BUNDLE_RELEA_INCLUDE_SAM%"=="1"  set BUDNEL_RELEA_INCLUDE=%BUDNEL_RELEA_INCLUDE% SAM,
if "%ENA_BUNDLE_RELEA_INCLUDE_MCU%"=="1"  set BUDNEL_RELEA_INCLUDE=%BUDNEL_RELEA_INCLUDE% MCU


if "%RET_GEN_BUNDLE%"=="1"   echo .   [%TGT_NAME%] is generated
if "%RET_GEN_BUNDLE%"=="1"   echo .     DTB_MANUFA= %DTB_MANUFA%
if "%RET_GEN_BUNDLE%"=="1"   echo .     DTB_DEVICE= %DTB_DEVICE%
if "%RET_GEN_BUNDLE%"=="1"   echo .     BUNDLE_DEBUG_INCLUDE: %BUDNEL_DEBUG_INCLUDE%
if "%RET_GEN_BUNDLE%"=="1"   echo .     BUNDLE_DBG_R_INCLUDE: %BUDNEL_DBG_R_INCLUDE%
if "%RET_GEN_BUNDLE%"=="1"   echo .     BUNDLE_RELEA_INCLUDE: %BUDNEL_RELEA_INCLUDE%

if "%RET_GEN_BUNDLE%"=="1"   echo | set /p=".     "
if "%RET_GEN_BUNDLE%"=="1"   if "%ENA_GEN_BUNDLE_DEBUG%"=="1" echo | set /p="%FN_BUNDLE_DEBUG%.same "
if "%RET_GEN_BUNDLE%"=="1"   if "%ENA_GEN_DTB_DEBUG%"=="1"    echo | set /p=" <= %DTB_XML_FN_DEBUG% <= %MAIN_FN_DTB_DEBUG_TGT%.dtb.sam"
if "%RET_GEN_BUNDLE%"=="1"   echo.

if "%RET_GEN_BUNDLE%"=="1"   echo | set /p=".     "
if "%RET_GEN_BUNDLE%"=="1"   if "%ENA_GEN_BUNDLE_DBG_R%"=="1" echo | set /p="%FN_BUNDLE_DBG_R%.same "
if "%RET_GEN_BUNDLE%"=="1"   if "%ENA_GEN_DTB_DBG_R%"=="1"    echo | set /p=" <= %DTB_XML_FN_DBG_R% <= %MAIN_FN_DTB_DBG_R_TGT%.dtb.sam"
if "%RET_GEN_BUNDLE%"=="1"   echo.

if "%RET_GEN_BUNDLE%"=="1"   echo | set /p=".     "
if "%RET_GEN_BUNDLE%"=="1"   if "%ENA_GEN_BUNDLE_RELEA%"=="1" echo | set /p="%FN_BUNDLE_RELEA%.same "
if "%RET_GEN_BUNDLE%"=="1"   if "%ENA_GEN_DTB_RELEA%"=="1"    echo | set /p=" <= %DTB_XML_FN_RELEA% <= %MAIN_FN_DTB_RELEA_TGT%.dtb.sam"
if "%RET_GEN_BUNDLE%"=="1"   echo.

if not "%RET_GEN_BUNDLE%"=="1" (
	echo    *** [gen_tym_bundle.bat] %TGT_NAME% build FAIL ***
)
echo \-------------------------------------------------------------------------/
echo.

::Let jenkins know build fail
if not "%RET_GEN_BUNDLE%"=="1" set ERRORLEVEL=999
if "%RET_GEN_BUNDLE%"=="1" set ERRORLEVEL=0


::=================================================
:: Function Defination
::=================================================
goto EOF

:FunGenBundle
	echo.
	echo.
	echo FunGenBundle( %1, %2, %3, %4, %5, %6, %7, %8, %9, ... )
	set RET=0

	set PARM_BUNDLE_VER_POSTFIX=%~1&  shift
	set PARM_VER_POSTFIX=%~1&         shift 
	set PARM_ENA_GEN_BUNDLE=%~1&      shift
	set PARM_XML_FN=%~1&              shift             
	set PARM_PATH_TMP_BINHEX=%~1&     shift
	set PARM_PATH_HEX_AP=%~1&         shift
	set PARM_DIR_OUT=%~1&             shift
	set PARM_UPDATE_IMG_URL_BASE=%~1& shift
	set PARM_ENA_GEN_DTB=%~1&         shift
	set PARM_PATH_DTB_TMP=%~1&        shift
	set PARM_DTB_VER=%~1&             shift
	set PARM_BUNDLE_INCLUDE_SAM=%~1&  shift
	set PARM_BUNDLE_INCLUDE_MCU=%~1&  shift
	set PARM_BUNDLE_INCLUDE_DTB=%~1&  shift
	
	echo PARM_BUNDLE_VER_POSTFIX="%PARM_BUNDLE_VER_POSTFIX%"
	echo PARM_VER_POSTFIX="%PARM_VER_POSTFIX%"
	echo PARM_ENA_GEN_BUNDLE="%PARM_ENA_GEN_BUNDLE%"
	echo PARM_XML_FN="%PARM_XML_FN%"
	echo PARM_PATH_TMP_BINHEX="%PARM_PATH_TMP_BINHEX%"
	echo PARM_PATH_HEX_AP="%PARM_PATH_HEX_AP%"
	echo PARM_DIR_OUT="%PARM_DIR_OUT%"
	echo PARM_UPDATE_IMG_URL_BASE="%PARM_UPDATE_IMG_URL_BASE%"
	echo PARM_ENA_GEN_DTB="%PARM_ENA_GEN_DTB%"
	echo PARM_PATH_DTB_TMP="%PARM_PATH_DTB_TMP%"
	echo PARM_DTB_VER="%PARM_DTB_VER%"
	echo PARM_BUNDLE_INCLUDE_SAM="%PARM_BUNDLE_INCLUDE_SAM%"
	echo PARM_BUNDLE_INCLUDE_MCU="%PARM_BUNDLE_INCLUDE_MCU%"
	echo PARM_BUNDLE_INCLUDE_DTB="%PARM_BUNDLE_INCLUDE_DTB%"

	if not "%PARM_ENA_GEN_BUNDLE%"=="1" set RET=1& goto EOF

		set LOCAL_PATH_XML=%PARM_DIR_OUT%\%PARM_XML_FN%
		set LOCAL_MCU_VER=%MCU_VER%%PARM_VER_POSTFIX%
		set LOCAL_SAM_VER=%SAM_VER%%PARM_VER_POSTFIX%
		set LOCAL_DTB_VER=%PARM_DTB_VER%%PARM_VER_POSTFIX%

		if not "%LOCAL_MCU_VER:~31%"=="" set LOCAL_MCU_VER=%LOCAL_MCU_VER:~-31%& echo. & echo. *** WARNING: LOCAL_MCU_VER (%LOCAL_DTB_VER%) ^>32bytes, cut to "%LOCAL_MCU_VER:~-31%" & echo. ***
		if not "%LOCAL_SAM_VER:~31%"=="" set LOCAL_SAM_VER=%LOCAL_SAM_VER:~-31%& echo. & echo. *** WARNING: LOCAL_SAM_VER (%LOCAL_DTB_VER%) ^>32bytes, cut to "%LOCAL_SAM_VER:~-31%" & echo. ***
		if not "%LOCAL_DTB_VER:~31%"=="" set LOCAL_DTB_VER=%LOCAL_DTB_VER:~-31%& echo. & echo. *** WARNING: LOCAL_DTB_VER (%LOCAL_DTB_VER%) ^>32bytes, cut to "%LOCAL_DTB_VER:~-31%" & echo. ***

		set LOCAL_BUNDLE_VER=

		if "%PARM_BUNDLE_INCLUDE_DTB%"=="1" (
			set LOCAL_DTB_PARAM= -d "%LOCAL_DTB_VER%" "%PARM_PATH_DTB_TMP%.dtb"
		) else (
			set LOCAL_DTB_PARAM= 
		)
		
		if "%PARM_BUNDLE_INCLUDE_SAM%"=="1" (
			set LOCAL_SAM_PARAM= -s "%LOCAL_SAM_VER%" "%SAM%"
		) else (
			set LOCAL_SAM_PARAM= 
		)
		
		if "%PARM_BUNDLE_INCLUDE_MCU%"=="1" (
			set LOCAL_MCU_PARAM= -m "%LOCAL_MCU_VER%" "%PARM_PATH_HEX_AP%"
		) else (
			set LOCAL_MCU_PARAM= 
		)
		
		set LOCAL_BUNDLE_VER=%SAM_VER%.%MCU_VER%_%PARM_BUNDLE_VER_POSTFIX%
		set LOCAL_FN_BUNDLE=%PRJ_NAME%_bundle_hwv%HW_VER%_swv%LOCAL_BUNDLE_VER%
		set LOCAL_PATH_BUNDLE=%PARM_DIR_OUT%\%LOCAL_FN_BUNDLE%

		echo.
		echo python.exe gen_bundle.py "%PATH_CERT%" -e "%DTB_MANUFA%" "%DTB_DEVICE%" "%PARM_PATH_TMP_BINHEX%" "%LOCAL_BUNDLE_VER%" "%LOCAL_PATH_BUNDLE%" %LOCAL_SAM_PARAM% %LOCAL_MCU_PARAM% %LOCAL_DTB_PARAM%
		python.exe gen_bundle.py "%PATH_CERT%" -e "%DTB_MANUFA%" "%DTB_DEVICE%" "%PARM_PATH_TMP_BINHEX%" "%LOCAL_BUNDLE_VER%" "%LOCAL_PATH_BUNDLE%" %LOCAL_SAM_PARAM% %LOCAL_MCU_PARAM% %LOCAL_DTB_PARAM%

		if not "%ERRORLEVEL%"=="0" echo [gen_tym_bundle.bat] Fail to generate "%LOCAL_PATH_BUNDLE%.same" & goto EOF
		if "%PARM_ENA_GEN_DTB%"=="1"  sed "s/<url>/%PARM_UPDATE_IMG_URL_BASE%/g" "%LOCAL_PATH_BUNDLE%.same.mft" > "%LOCAL_PATH_XML%"
		if "%PARM_ENA_GEN_DTB%"=="1"  if not exist "%LOCAL_PATH_XML%" echo [gen_tym_bundle.bat] Fail to generate "%LOCAL_PATH_XML%" & goto EOF

	set RET=1
	goto EOF


:FunGenDtb 
	echo.
	echo.
	echo FunGenDtb( %1, %2, %3, %4, %5 )
	set RET=0

	set PARM_MAIN_FN_DTB_TGT=%~1&  shift
	set PARM_DTB_MAIN_FN=%~1&      shift 
	set PARM_PATH_DTB_TMP=%~1&     shift 
	set PARM_PATH_DTB_TGT=%~1&     shift 
	set PARM_ENA_GEN_DTB=%~1&      shift 
	set PARM_DBG_VER=%~1&          shift 


	if not "%PARM_ENA_GEN_DTB%"=="1" set RET=1& goto EOF
		
		cat "%PATH_DTB%\%PARM_DTB_MAIN_FN%.dts" | sed "s/device-tree-data-version = \".*\";/device-tree-data-version = \"%PARM_DBG_VER%\";/g" > "%PARM_PATH_DTB_TMP%.dts"
		"%~dp0\..\tools\dtc\dtc.exe" -o "%PARM_PATH_DTB_TMP%.dtb" -O dtb "%PARM_PATH_DTB_TMP%.dts"
		if not exist "%PARM_PATH_DTB_TMP%.dtb" echo [gen_tym_bundle.bat] Fail to generate "%PARM_PATH_DTB_TMP%.dtb" & goto EOF

		python.exe gen_bundle.py "%PATH_CERT%" "%DTB_MANUFA%" "%DTB_DEVICE%" "tmp.binhex" "%PARM_DBG_VER%" "%PARM_PATH_DTB_TGT%.dtb" -d "%PARM_DBG_VER%" "%PARM_PATH_DTB_TMP%.dtb"
		if not exist "%PARM_PATH_DTB_TGT%.dtb.sam" echo [gen_tym_bundle.bat] Fail to generate "%PARM_PATH_DTB_TGT%.dtb.sam" & goto EOF

	set RET=1
	goto EOF


:EOF
