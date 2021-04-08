@echo off

::Note it is a old script and no longer support
echo It is a old script and no longer support
pause
exit


::======================================
:: Tool check
::======================================
call parm_tools.bat
if not exist "%PATH_PYTHON%\python.exe" echo *** Please install python v2.7 to %PATH_PYTHON%\ *** & goto FINISH
if not exist "%PATH_BASH%\bash.exe"     echo *** Please install cygwin to %PATH_BASH%\        *** & goto FINISH


::=================================
:: Parameter
::=================================
::Frequency Modify 
set SAM_VER=1.7.14-s

:: Other Parameter
set PATH_OUT=.\out_sam
set BUNDLE_NAME=tym-openwrt-ar71xx-generic-cus227-firmware-%SAM_VER%-tym-theme

::The information MUST the same as dtb/dts setting, or upgrading will fail
set DTB_MANUFACTURER=Tymphany
set DTB_DEV_NAME=TymphanySpeaker



::=================================
::Build SAM
::=================================
::Create Folder
rm -rf "%PATH_OUT%"
mkdir "%PATH_OUT%"

::Build SAM
set BUNDLE_VER=%SAM_VER%
set SAM_NAME=sam_v%SAM_VER%/openwrt-ar71xx-generic-cus227-firmware-v%SAM_VER%-tym-theme
python.exe gen_bundle.py "%PATH_CERT%" "%DTB_MANUFACTURER%" "%DTB_DEV_NAME%" %PATH_OUT% "%BUNDLE_VER%" "%BUNDLE_NAME%" -s "%SAM_VER%" "%SAM_NAME%"


::=================================
:: Finish
::=================================
:FINISH
pause
