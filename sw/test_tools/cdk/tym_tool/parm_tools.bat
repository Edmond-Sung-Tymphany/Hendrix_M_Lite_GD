@echo off
::The output are show on FINISH section (end of this file)

set RET_PARM_TOOLS=0
::======================================
:: Caller should pass these parameters
::======================================
if "%PATH_MCU_HEADER%"==""   echo *** Please call "parm_proj_info.bat" first *** (PATH_MCU_HEADER is empty) ***   & goto FINISH
if "%DTB_MAIN_FN_DEBUG%"=="" echo *** Please call "parm_proj_info.bat" first *** (DTB_MAIN_FN_DEBUG is empty) *** & goto FINISH
if "%DTB_MAIN_FN_DBG_R%"=="" echo *** Please call "parm_proj_info.bat" first *** (DTB_MAIN_FN_DBG_R is empty) *** & goto FINISH
if "%DTB_MAIN_FN_RELEA%"=="" echo *** Please call "parm_proj_info.bat" first *** (DTB_MAIN_FN_RELEA is empty) *** & goto FINISH
if "%PATH_DTB%"==""          echo *** Please call "parm_proj_info.bat" first *** (PATH_DTB is empty) ***          & goto FINISH


::======================================
:: ENA_DTS_TRACE= ENA_CP_DTB || ENA_GEN_BUNDLE
::======================================
::Unuse currently
set ENA_DTS_TRACE_DEBUG=0
set ENA_DTS_TRACE_DBG_R=0
set ENA_DTS_TRACE_RELEA=0
if "%ENA_GEN_DTB_DEBUG%"=="1"     set ENA_DTS_TRACE_DEBUG=1
if "%ENA_GEN_BUNDLE_DEBUG%"=="1"  set ENA_DTS_TRACE_DEBUG=1
if "%ENA_GEN_DTB_DBG_R%"=="1"     set ENA_DTS_TRACE_DBG_R=1
if "%ENA_GEN_BUNDLE_DBG_R%"=="1"  set ENA_DTS_TRACE_DBG_R=1
if "%ENA_GEN_DTB_RELEA%"=="1"     set ENA_DTS_TRACE_RELEA=1
if "%ENA_GEN_BUNDLE_RELEA%"=="1"  set ENA_DTS_TRACE_RELEA=1


::===============================
:: Fixed tool path
::===============================
set PATH_PYTHON=C:\Python27

set PATH_BASH1=C:\cygwin\bin
set PATH_BASH2=C:\tools\cygwin\bin


::===============================
:: Automatic path search
::===============================
::MPLAB path
if exist "C:\Microchip\MPLABX\mplab_ide\bin" (
	set PATH_MPLAB=C:\Microchip\MPLABX
) else if exist "C:\Program Files (x86)\Microchip\MPLABX\mplab_ide\bin" (
	set PATH_MPLAB=C:\Program Files ^(x86^)\Microchip\MPLABX
) else if exist "C:\tools\MPLABX\mplab_ide\bin" (
	set PATH_MPLAB=C:\tools\MPLABX
) else if exist "D:\tools\MPLABX\mplab_ide\bin" (
	set PATH_MPLAB=D:\tools\MPLABX
) else (
	set PATH_MPLAB=
	echo *** MPLAB is not found ***
	goto FINISH
)
set PATH_JAVA=%PATH_MPLAB%\sys\java\jre1.7.0_25-windows-x64\java-windows\bin

::Cygwin path
if exist "%PATH_BASH1%\bash.exe" (
	set PATH_BASH=%PATH_BASH1%
) else if exist "%PATH_BASH2%\bash.exe" (
	set PATH_BASH=%PATH_BASH2%
) else (
	set PATH_BASH=
	echo *** Cygwin is not found ***
	goto FINISH
)

::Do not show set warning "MS-DOS style path detected"
set CYGWIN=nodosfilewarning

::Do not show set warning "MS-DOS style path detected"
if not exist "%PATH_BASH%\openssl.exe" echo *** Please install cygwin package openssl (%PATH_BASH%\openssl.exe) *** & goto FINISH

::gen_bundle.py MUST use cygwin's dos2unix, not git's dos2unix. Or "gen_firmware" format translation will have problem
set PATH=%PATH_BASH%;%PATH_PYTHON%;%PATH%




::================================================
:: Parameter Checking
::================================================
if not "%ENA_BUNDLE_DEBUG_INCLUDE_SAM%"=="1" if not "%ENA_BUNDLE_DEBUG_INCLUDE_MCU%"=="1" echo Please set ENA_BUNDLE_DEBUG_INCLUDE_SAM=1 or ENA_BUNDLE_DEBUG_INCLUDE_MCU=1 & goto FINISH



::================================================
:: MCU Version (attachedDevicesMcu.h)
::================================================
::PATH_BASH must add to PATH before
bash -c "cat '%PATH_MCU_HEADER%' | grep R_VER | sed ':a;N;s/\n/ /g;ta' | awk '{printf (\"%%s.%%s%%s\",$3,$6,$9)}'" > tmp.txt
set MCU_VER=
FOR /F "tokens=* delims= " %%i in (tmp.txt) do set MCU_VER=%%i
rm -f tmp.txt

::MCU_VER must have 4 characters
if "%MCU_VER:~3,1%"==""     set MCU_VER= & echo *** MCU_VER error (length^<4) *** & goto FINISH
if not "%MCU_VER:~4,1%"=="" set MCU_VER= & echo *** MCU_VER error (length^>4) *** & goto FINISH


::================================================
:: DTB Debug, Normal
::================================================
:TAG_DTB_DEBUG

if not "%ENA_GEN_DTB_DEBUG%"=="1" goto TAG_DTB_DBG_R
::Update XML file name
  bash -c "cat '%PATH_DTB%\%DTB_MAIN_FN_DEBUG%.dts' | grep 'update-url =' | sed 's/[\t ]*update-url = //g' | awk -F '/' '{print $NF}' | sed 's/[;\"]//g' > tmp.txt"
  set DTB_XML_FN_DEBUG=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set DTB_XML_FN_DEBUG=%%i
  rm -f tmp.txt
  ::if not "%DTB_XML_FN_DEBUG:~0,5%"=="http:" echo *** DTB_XML_FN_DEBUG should start with http: (=%DTB_XML_FN_DEBUG%) *** & set DTB_XML_FN_DEBUG= & goto FINISH
  if "%DTB_XML_FN_DEBUG%"=="" echo *** [parm_tools.bat] update-url in %DTB_MAIN_FN_DEBUG%.dts is empty *** & set DTB_XML_FN_DEBUG= & goto FINISH

::Manufacturer
  bash -c "cat '%PATH_DTB%\%DTB_MAIN_FN_DEBUG%.dts' | grep 'manufacturer =' | awk -F '=' '{print $2}' | sed 's/[;\"]//g' > tmp.txt"
  set TMP_DTB_MANUFA_DEBUG=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set TMP_DTB_MANUFA_DEBUG=%%i
  rm -f tmp.txt
  if "%TMP_DTB_MANUFA_DEBUG%"=="" echo *** manufacturer string is empty in %DTB_MAIN_FN_DEBUG%.dts *** & goto FINISH
  ::echo TMP_DTB_MANUFA_DEBUG=%TMP_DTB_MANUFA_DEBUG%

::Device
  bash -c "cat '%PATH_DTB%\%DTB_MAIN_FN_DEBUG%.dts' | grep 'device =' | sed 's/.*device = //g' | sed 's/[;\"]//g' > tmp.txt"
  set TMP_DTB_DEVICE_DEBUG=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set TMP_DTB_DEVICE_DEBUG=%%i
  rm -f tmp.txt
  if "%TMP_DTB_DEVICE_DEBUG%"=="" echo *** device string is empty in %DTB_MAIN_FN_DEBUG%.dts *** & set TMP_DTB_DEVICE_DEBUG= & goto FINISH
  ::echo TMP_DTB_DEVICE_DEBUG=%TMP_DTB_DEVICE_DEBUG%

::Version, must <= 32 bytes
  cat "%PATH_DTB%\%DTB_MAIN_FN_DEBUG%.dts" | grep -v "\/\*" | grep "device-tree-data" | sed "s/.*device-tree-data-version = \"\(.*\)\";/\1/g" > tmp.txt
  set DTB_VER_DEBUG=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set DTB_VER_DEBUG=%MCU_VER%%%i
  rm -f tmp.txt
  if not "%DTB_VER_DEBUG:~31%"=="" echo *** dtb version is too long (^>32) in %DTB_MAIN_FN_DEBUG%.dts *** & set DTB_VER_DEBUG= & goto FINISH
  if "%DTB_VER_DEBUG%"==""         echo *** dtb version is empty in %DTB_MAIN_FN_DEBUG%.dts ***  & goto FINISH

::File name
  set MAIN_FN_DTB_DEBUG_TGT=%DTB_MAIN_FN_DEBUG%_v%DTB_VER_DEBUG%


::================================================
:: DTB Debug, Repeated Upgrade
::================================================
:TAG_DTB_DBG_R

if not "%ENA_GEN_DTB_DBG_R%"=="1" goto TAG_DTB_RELEA
::Update XML file name
  bash -c "cat '%PATH_DTB%\%DTB_MAIN_FN_DBG_R%.dts' | grep 'update-url =' | sed 's/[\t ]*update-url = //g' | awk -F '/' '{print $NF}' | sed 's/[;\"]//g' > tmp.txt"
  set DTB_XML_FN_DBG_R=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set DTB_XML_FN_DBG_R=%%i
  rm -f tmp.txt
  ::if not "%DTB_XML_FN_DBG_R:~0,5%"=="http:" echo *** DTB_XML_FN_DBG_R should start with http: (=%DTB_XML_FN_DBG_R%) *** & set DTB_XML_FN_DBG_R= & goto FINISH
  if "%DTB_XML_FN_DBG_R%"=="" echo *** [parm_tools.bat] update-url in %DTB_MAIN_FN_DBG_R%.dts is empty *** & set DTB_XML_FN_DBG_R= & goto FINISH

::Manufacturer
  bash -c "cat '%PATH_DTB%\%DTB_MAIN_FN_DBG_R%.dts' | grep 'manufacturer =' | awk -F '=' '{print $2}' | sed 's/[;\"]//g' > tmp.txt"
  set TMP_DTB_MANUFA_DBG_R=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set TMP_DTB_MANUFA_DBG_R=%%i
  rm -f tmp.txt
  if "%TMP_DTB_MANUFA_DBG_R%"=="" echo *** manufacturer string is empty in %DTB_MAIN_FN_DBG_R%.dts *** & goto FINISH
  ::echo TMP_DTB_MANUFA_DBG_R=%TMP_DTB_MANUFA_DBG_R%

::Device
  bash -c "cat '%PATH_DTB%\%DTB_MAIN_FN_DBG_R%.dts' | grep 'device =' | sed 's/.*device = //g' | sed 's/[;\"]//g' > tmp.txt"
  set TMP_DTB_DEVICE_DBG_R=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set TMP_DTB_DEVICE_DBG_R=%%i
  rm -f tmp.txt
  if "%TMP_DTB_DEVICE_DBG_R%"=="" echo *** device string is empty in %DTB_MAIN_FN_DBG_R%.dts *** & set TMP_DTB_DEVICE_DBG_R= & goto FINISH
  ::echo TMP_DTB_DEVICE_DBG_R=%TMP_DTB_DEVICE_DBG_R%

::Version, must <= 32 bytes
  cat "%PATH_DTB%\%DTB_MAIN_FN_DBG_R%.dts" | grep -v "\/\*" | grep "device-tree-data" | sed "s/.*device-tree-data-version = \"\(.*\)\";/\1/g" > tmp.txt
  set DTB_VER_DBG_R=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set DTB_VER_DBG_R=%MCU_VER%%%i
  rm -f tmp.txt
  if not "%DTB_VER_DBG_R:~31%"=="" echo *** dtb version is too long (^>32) in %DTB_MAIN_FN_DBG_R%.dts *** & set DTB_VER_DBG_R= & goto FINISH
  if "%DTB_VER_DBG_R%"==""         echo *** dtb version is empty in %DTB_MAIN_FN_DBG_R%.dts *** & goto FINISH

::File name
  set MAIN_FN_DTB_DBG_R_TGT=%DTB_MAIN_FN_DBG_R%_v%DTB_VER_DBG_R%


::================================================
:: DTB Release
::================================================
:TAG_DTB_RELEA

if not "%ENA_GEN_DTB_RELEA%"=="1" goto TAG_DTB_CHECK
::Update XML file name
  bash -c "cat '%PATH_DTB%\%DTB_MAIN_FN_RELEA%.dts' | grep 'update-url =' | sed 's/[\t ]*update-url = //g' | awk -F '/' '{print $NF}' | sed 's/[;\"]//g' > tmp.txt"
  set DTB_XML_FN_RELEA=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set DTB_XML_FN_RELEA=%%i
  rm -f tmp.txt
  ::if not "%DTB_XML_FN_RELEA:~0,5%"=="http:" echo *** DTB_XML_FN_RELEA should start with http: (=%DTB_XML_FN_RELEA%) *** & set DTB_XML_FN_RELEA= & goto FINISH
  if "%DTB_XML_FN_RELEA%"=="" echo *** [parm_tools.bat] update-url in %DTB_MAIN_FN_RELEA%.dts is empty *** & set DTB_XML_FN_RELEA= & goto FINISH

::Manufacturer
  bash -c "cat '%PATH_DTB%\%DTB_MAIN_FN_RELEA%.dts' | grep 'manufacturer =' | awk -F '=' '{print $2}' | sed 's/[;\"]//g' > tmp.txt"
  set TMP_DTB_MANUFA_RELEA=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set TMP_DTB_MANUFA_RELEA=%%i
  rm -f tmp.txt
  if "%TMP_DTB_MANUFA_RELEA%"=="" echo *** manufacturer string is empty in %DTB_MAIN_FN_RELEA%.dts *** & goto FINISH
  ::echo TMP_DTB_MANUFA_RELEA=%TMP_DTB_MANUFA_RELEA%

:Device
  bash -c "cat '%PATH_DTB%\%DTB_MAIN_FN_RELEA%.dts' | grep 'device =' | sed 's/.*device = //g' | sed 's/[;\"]//g' > tmp.txt"
  set TMP_DTB_DEVICE_RELEA=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set TMP_DTB_DEVICE_RELEA=%%i
  rm -f tmp.txt
  if "%TMP_DTB_DEVICE_RELEA%"=="" echo *** device string is empty in %DTB_MAIN_FN_RELEA%.dts ***  & goto FINISH
  ::echo TMP_DTB_DEVICE_RELEA=%TMP_DTB_DEVICE_RELEA%

::Version, must <= 32 bytes
  cat "%PATH_DTB%\%DTB_MAIN_FN_RELEA%.dts" | grep -v "\/\*" | grep "device-tree-data" | sed "s/.*device-tree-data-version = \"\(.*\)\";/\1/g" > tmp.txt
  set DTB_VER_RELEA=
  FOR /F "tokens=* delims= " %%i in (tmp.txt) do set DTB_VER_RELEA=%MCU_VER%%%i
  rm -f tmp.txt
  if not "%DTB_VER_RELEA:~31%"=="" echo *** dtb version is too long (^>32) in %DTB_MAIN_FN_RELEA%.dts *** & set DTB_VER_RELEA= & goto FINISH
  if "%DTB_VER_RELEA%"==""         echo *** dtb version is empty in %DTB_MAIN_FN_RELEA%.dts *** & goto FINISH

::File name
  set MAIN_FN_DTB_RELEA_TGT=%DTB_MAIN_FN_RELEA%_v%DTB_VER_RELEA%

::================================================
:: DTB Check consistency
::================================================
:TAG_DTB_CHECK

if not "%ENA_GEN_DTB_DEBUG%"=="1" if not "%ENA_GEN_DTB_RELEA%"=="1" if not "%ENA_GEN_DTB_DBG_R%"=="1" echo *** Please enable at least one ENA_GEN_DTB_XXXXX in parm_proj_info.bat *** && goto FINISH

::Check Manufacturer consistency
if "%ENA_GEN_DTB_DEBUG%"=="1" if "%ENA_GEN_DTB_RELEA%"=="1" if not "%TMP_DTB_MANUFA_DEBUG%"=="%TMP_DTB_MANUFA_RELEA%" echo *** Manufacturer strings are mis-match on %DTB_MAIN_FN_DEBUG%.dts(=%TMP_DTB_MANUFA_DEBUG%) and %DTB_MAIN_FN_RELEA%.dts(=%TMP_DTB_MANUFA_RELEA%) & set TMP_DTB_MANUFA_DEBUG= & set TMP_DTB_MANUFA_RELEA= & goto FINISH
if "%ENA_GEN_DTB_DBG_R%"=="1" if "%ENA_GEN_DTB_RELEA%"=="1" if not "%TMP_DTB_MANUFA_DBG_R%"=="%TMP_DTB_MANUFA_RELEA%" echo *** Manufacturer strings are mis-match on %DTB_MAIN_FN_DBG_R%.dts(=%TMP_DTB_MANUFA_DBG_R%) and %DTB_MAIN_FN_RELEA%.dts(=%TMP_DTB_MANUFA_RELEA%) & set TMP_DTB_MANUFA_DBG_R= & set TMP_DTB_MANUFA_RELEA= & goto FINISH
if "%ENA_GEN_DTB_DBG_R%"=="1" if "%ENA_GEN_DTB_DEBUG%"=="1" if not "%TMP_DTB_MANUFA_DBG_R%"=="%TMP_DTB_MANUFA_DEBUG%" echo *** Manufacturer strings are mis-match on %DTB_MAIN_FN_DBG_R%.dts(=%TMP_DTB_MANUFA_DBG_R%) and %DTB_MAIN_FN_DEBUG%.dts(=%TMP_DTB_MANUFA_DEBUG%) & set TMP_DTB_MANUFA_DBG_R= & set TMP_DTB_MANUFA_DEBUG= & goto FINISH

if "%ENA_GEN_DTB_DEBUG%"=="1" set DTB_MANUFA=%TMP_DTB_MANUFA_DEBUG%
if "%ENA_GEN_DTB_DBG_R%"=="1" set DTB_MANUFA=%TMP_DTB_MANUFA_DBG_R%
if "%ENA_GEN_DTB_RELEA%"=="1" set DTB_MANUFA=%TMP_DTB_MANUFA_RELEA%

::Check Device consistency 
if "%ENA_GEN_DTB_DEBUG%"=="1" if "%ENA_GEN_DTB_RELEA%"=="1" if not "%TMP_DTB_DEVICE_DEBUG%"=="%TMP_DTB_DEVICE_RELEA%" echo *** Device strings are mis-match on %DTB_MAIN_FN_DEBUG%.dts(=%TMP_DTB_DEVICE_DEBUG%) and %DTB_MAIN_FN_RELEA%.dts(=%TMP_DTB_DEVICE_RELEA%) & set TMP_DTB_DEVICE_DEBUG= & set TMP_DTB_DEVICE_RELEA= & goto FINISH
if "%ENA_GEN_DTB_DBG_R%"=="1" if "%ENA_GEN_DTB_RELEA%"=="1" if not "%TMP_DTB_DEVICE_DBG_R%"=="%TMP_DTB_DEVICE_RELEA%" echo *** Device strings are mis-match on %DTB_MAIN_FN_DBG_R%.dts(=%TMP_DTB_DEVICE_DBG_R%) and %DTB_MAIN_FN_RELEA%.dts(=%TMP_DTB_DEVICE_RELEA%) & set TMP_DTB_DEVICE_DBG_R= & set TMP_DTB_DEVICE_RELEA= & goto FINISH
if "%ENA_GEN_DTB_DBG_R%"=="1" if "%ENA_GEN_DTB_DEBUG%"=="1" if not "%TMP_DTB_DEVICE_DBG_R%"=="%TMP_DTB_DEVICE_DEBUG%" echo *** Device strings are mis-match on %DTB_MAIN_FN_DBG_R%.dts(=%TMP_DTB_DEVICE_DBG_R%) and %DTB_MAIN_FN_DEBUG%.dts(=%TMP_DTB_DEVICE_DEBUG%) & set TMP_DTB_DEVICE_DBG_R= & set TMP_DTB_DEVICE_DEBUG= & goto FINISH

if "%ENA_GEN_DTB_DEBUG%"=="1" set DTB_DEVICE=%TMP_DTB_DEVICE_DEBUG%
if "%ENA_GEN_DTB_DBG_R%"=="1" set DTB_DEVICE=%TMP_DTB_DEVICE_DBG_R%
if "%ENA_GEN_DTB_RELEA%"=="1" set DTB_DEVICE=%TMP_DTB_DEVICE_RELEA%


::===============================
:: Print
::===============================
set RET_PARM_TOOLS=1
:: Here is the output of this file:
echo  [Project]
echo    PRJ_NAME:  %PRJ_NAME%
echo    MCU_VER:   %MCU_VER%
echo    SAM_VER:   %SAM_VER%
echo    SAM_THEME: %SAM_THEME%
echo    HW_VER:    %HW_VER%
echo  [Generate Bundle file]
echo    DEBUG: ena=%ENA_GEN_DTB_DEBUG%
echo    DBG_R: ena=%ENA_GEN_DTB_DBG_R%
echo    RELEA: ena=%ENA_GEN_DTB_RELEA%
echo  [DTB file]
echo    MANUFA: %DTB_MANUFA%
echo    DEVICE: %DTB_DEVICE%
if "%ENA_GEN_DTB_DEBUG%"=="1" echo    DEBUG: %DTB_XML_FN_DEBUG% ^<==	%MAIN_FN_DTB_RELEA_TGT%.dtb.sam
if "%ENA_GEN_DTB_DBG_R%"=="1" echo    DBG_R: %DTB_XML_FN_DBG_R% ^<==	%MAIN_FN_DTB_DBG_R_TGT%.dtb.sam
if "%ENA_GEN_DTB_RELEA%"=="1" echo    RELEA: %DTB_XML_FN_RELEA% ^<==	%MAIN_FN_DTB_RELEA_TGT%.dtb.sam
echo  [XML file, UPDATE_IMG_URL_BASE]
if "%ENA_GEN_DTB_DEBUG%"=="1" echo    DEBUG: %UPDATE_IMG_URL_BASE_DEBUG%
if "%ENA_GEN_DTB_DBG_R%"=="1" echo    DBG_R: %UPDATE_IMG_URL_BASE_DBG_R%
if "%ENA_GEN_DTB_RELEA%"=="1" echo    RELEA: %UPDATE_IMG_URL_BASE_RELEA%
echo  [Tool Path]
echo    PATH_PYTHON: %PATH_PYTHON%
echo    PATH_BASH:   %PATH_BASH%
echo    PATH_MPLAB:  %PATH_MPLAB%
echo **********************************************************************
echo.
echo.

:FINISH
echo.
