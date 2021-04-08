@echo off


::Note it is a old script. It does not fill "file name" as version to dtb file
echo It is a old script and no longer support
exit



::======================================
:: Get project and tool information
::======================================
if not exist parm_proj_info.bat echo *** [gen_tym_bundle_dtb.bat] parm_proj_info.bat is not exist *** & goto FINISH
call "parm_proj_info.bat"
call "%~dp0\parm_tools.bat"
if not "%RET_PARM_TOOLS%"=="1" echo *** [gen_tym_bundle_dtb.bat] parm_tools.bat is fail *** & goto FINISH

if not exist "%PATH_PYTHON%\python.exe" echo *** Please install python v2.7 to %PATH_PYTHON%\ *** & goto FINISH
if not exist "%PATH_BASH%\bash.exe"     echo *** Please install cygwin to %PATH_BASH%\ ***        & goto FINISH

if "%PATH_DTB%"==""   echo *** Do not call gen_tym_bundle_dtb.bat directly (PATH_DTB is empty)***    & goto FINISH
if "%DTB_MANUFA%"=="" echo *** Do not call gen_tym_bundle_dtb.bat directly (DTB_MANUFA is empty) *** & goto FINISH
if "%DTB_DEVICE%"=="" echo *** Do not call gen_tym_bundle_dtb.bat directly (DTB_DEVICE is empty) *** & goto FINISH
if "%PATH_CERT%"==""  echo *** Do not call gen_tym_bundle_dtb.bat directly (PATH_CERT is empty) ***  & goto FINISH


::=================================
:: Global Variable
::=================================
::Generete result
set RET_GEN_DTB=0

::Note we apply a "fake dtb version" instead of real DTB version (DTB_VER, query from dtb's model-number field).
::So DTB bundle must upgrade forcely (ignore version checking)
set DTB_VER=v1.0_fake
set BUNDLE_VER=%DTB_VER%
set PATH_OUT_TMP=.\out_tmp_dtb


::=================================
:: Translate DTB to SAM
::=================================
pushd "%PATH_DTB%"

rm -f *.dtb.sam
forfiles /m *.dtb /c "cmd /c python.exe \"%~dp0\gen_bundle.py\" "%PATH_CERT%" \"%DTB_MANUFA%\" \"%DTB_DEVICE%\" \".\tmp.binhex\" \"%BUNDLE_VER%\" \"@file\" -d \"%DTB_VER%\" \"@file\""
bash -c "[ $(ls *.dtb -l | wc -l) == $(ls -l *.dtb.sam|wc -l) ]"
if not "%ERRORLEVEL%"=="0" echo *** [gen_tym_bundle_dtb.bat] Fail to translate some dtb files *** && goto FINISH

popd


::=================================
:: Finish
::=================================
set RET_GEN_DTB=1

:FINISH
del /Q /S "%PATH_DTB%\*.mft" "%PATH_DTB%\*.tmp" > tmp.txt
del /Q  tmp.txt
echo.
echo.
echo /------------------------------------------\
echo    %date% %time:~0,8%  
if "%RET_GEN_DTB%"=="1" (
	echo    DTBs of %PRJ_NAME% are generated
	echo      DTB_MANUFA= %DTB_MANUFA%
	echo      DTB_DEVICE= %DTB_DEVICE%
)else (
	echo    Fail to generate DTBs
)
echo \------------------------------------------/
echo.

