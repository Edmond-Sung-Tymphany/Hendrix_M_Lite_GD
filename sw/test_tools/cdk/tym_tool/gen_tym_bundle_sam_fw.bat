@echo off

::======================================
:: Get project and tool information
::======================================
if not exist parm_proj_info.bat echo *** [gen_tym_bundle_sam_fw.bat] parm_proj_info.bat is not exist *** & goto FINISH
call "parm_proj_info.bat"
call "%~dp0\parm_tools.bat"
if not "%RET_PARM_TOOLS%"=="1" echo *** [gen_tym_bundle_sam_fw.bat] parm_tools.bat is fail *** & goto FINISH

if not exist "%PATH_PYTHON%\python.exe" echo *** Please install python v2.7 to %PATH_PYTHON%\ *** & goto FINISH
if not exist "%PATH_BASH%\bash.exe"     echo *** Please install cygwin to %PATH_BASH%\        *** & goto FINISH

if "%DTB_MANUFA%"=="" echo *** Do not call gen_tym_bundle_sam_fw.bat directly (DTB_MANUFA is empty) *** & goto FINISH
if "%DTB_DEVICE%"=="" echo *** Do not call gen_tym_bundle_sam_fw.bat directly (DTB_DEVICE is empty) *** & goto FINISH
if "%SAM_VER%"==""    echo *** Do not call gen_tym_bundle_sam_fw.bat directly (SAM_VER is empty) ***    & goto FINISH
if "%SAM_THEME%"==""  echo *** Do not call gen_tym_bundle.bat directly (SAM_THEME is empty) ***         & goto FINISH
if "%PATH_CERT%"==""  echo *** Do not call gen_tym_bundle_sam_fw.bat directly (PATH_CERT is empty) ***  & goto FINISH


::======================================
:: Parameter
::======================================
set PATH_OUT=%PATH_PRJ_AP%\..\out_SAM_FW
set BUNDLE_VER=%SAM_VER%
set PATH_SAM_RAW_FW=%~dp0/sam_v%SAM_VER%/openwrt-ar71xx-generic-cus227-firmware-v%SAM_VER%-%SAM_THEME%
set BUNDLE_FN_OUT=sam-fw-%SAM_VER%-%PRJ_NAME%-%SAM_THEME%


::=================================
::Build SAM
::=================================
::Create Folder
rm -rf "%PATH_OUT%"
mkdir "%PATH_OUT%"

::Build SAM
python.exe "%~dp0\gen_bundle.py" "%PATH_CERT%" "%DTB_MANUFA%" "%DTB_DEVICE%" "tmp.binhex" "%BUNDLE_VER%" "%PATH_OUT%\%BUNDLE_FN_OUT%" -s "%SAM_VER%" "%PATH_SAM_RAW_FW%"
set RET_GET_BUNDLE_PY=%ERRORLEVEL%

::=================================
:: Finish
::=================================
:FINISH
del /f "%PATH_OUT%\*.mft"
echo.
echo.
echo /------------------------------------------------------------------------------\
echo    %date% %time:~0,8%  
if "%RET_GET_BUNDLE_PY%"=="0" (
	echo    [%BUNDLE_FN_OUT%.sam] is generated
	echo      DTB_MANUFA= %DTB_MANUFA%
	echo      DTB_DEVICE= %DTB_DEVICE%
)else (
	echo    *** %BUNDLE_FN_OUT% generate FAIL ***
)
echo \------------------------------------------------------------------------------/
echo.



