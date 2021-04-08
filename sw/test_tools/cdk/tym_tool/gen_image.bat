@echo off

::======================================
:: Get project and tool information
::======================================
if not exist parm_proj_info.bat echo *** [gen_image.bat] parm_proj_info.bat is not exist *** & goto FINISH
call "parm_proj_info.bat"
call "%~dp0\parm_tools.bat"
if not "%RET_PARM_TOOLS%"=="1" echo *** [gen_image.bat] parm_tools.bat is fail *** & goto FINISH

if "%PRJ_NAME%"==""   echo *** Do not call gen_image.bat directly (PRJ_NAME is empty) ***   & goto FINISH
if "%PATH_PRJ_AP%"=="" echo *** Do not call gen_image.bat directly (PATH_PRJ_AP is empty) *** & goto FINISH
if "%PATH_PRJ_BL%"=="" echo *** Do not call gen_image.bat directly (PATH_PRJ_BL is empty) *** & goto FINISH
if "%PATH_MPLAB%"==""  echo *** Do not call gen_image.bat directly (PATH_MPLAB is empty) ***  & goto FINISH
if "%PATH_JAVA%"==""   echo *** Do not call gen_image.bat directly (PATH_JAVA is empty) ***   & goto FINISH


::Java path
%PATH_JAVA%\java -version 2> java_test.txt
if not "%ERRORLEVEL%"=="0" echo *** gen_image.bat: java.exe is not found *** & goto FINISH 
rm -f java_test.txt


::=================================
:: Parameter
::=================================
set MAKE_TARGET=.build-conf

::Enable for clean build
set MAKE_TARGET_CLEAN=.clean-conf
::set MAKE_TARGET_CLEAN=

:Thread number for make
set MAKE_PARM=-j 8

:QP
set PATH_QP=%~dp0\..\..\..\common\qp\ports\pic32\vanilla\xc32\MX450F256L



::=================================
:: Clean
::=================================
echo  %date% %time:~0,8%  Clean
if not "%MAKE_TARGET_CLEAN%" == "" rm -rf "%PATH_PRJ_BL%\build\" "%PATH_PRJ_BL%\dist\" "%PATH_PRJ_BL%\debug\"
if not "%MAKE_TARGET_CLEAN%" == "" rm -rf "%PATH_PRJ_AP%\build\" "%PATH_PRJ_AP%\dist\" "%PATH_PRJ_AP%\debug\"




::=================================
:: Build Release
::=================================

set TP_TARGET=release

echo.
echo.
echo ----------------------------------------------------------------------------
echo  %date% %time:~0,8%  // Build %PRJ_NAME% (%TP_TARGET% mode) //
echo ----------------------------------------------------------------------------
echo  %date% %time:~0,8%  Generate makefiles for QP
pushd "%PATH_QP%\"
%PATH_JAVA%\java -jar "%PATH_MPLAB%\mplab_ide\lib\PrjMakefilesGenerator.jar" .\@%TP_TARGET%
popd

echo.
echo  %date% %time:~0,8%  Generate makefiles and clean bootloader
pushd "%PATH_PRJ_BL%"
%PATH_JAVA%\java -jar "%PATH_MPLAB%\mplab_ide\lib\PrjMakefilesGenerator.jar" .\@%TP_TARGET%
if not "%ERRORLEVEL%"=="0" echo [gen_image.bat] Fail to generate bootloader makefile (%TP_TARGET% mode) & goto FINISH
if not "%MAKE_TARGET_CLEAN%" == "" %PATH_MPLAB%\gnuBins\GnuWin32\bin\make %MAKE_PARM% -f nbproject\Makefile-%TP_TARGET%.mk SUBPROJECTS= %MAKE_TARGET_CLEAN%
if not "%ERRORLEVEL%"=="0" echo [gen_image.bat] Fail to build bootloader (%TP_TARGET% mode) & goto FINISH
popd

echo.
echo  %date% %time:~0,8%  Generate makefiles for application
pushd "%PATH_PRJ_AP%"
%PATH_JAVA%\java -jar "%PATH_MPLAB%\mplab_ide\lib\PrjMakefilesGenerator.jar" .\@%TP_TARGET%
if not "%ERRORLEVEL%"=="0" echo [gen_image.bat] Fail to generate application makefile (%TP_TARGET% mode) & goto FINISH
echo.
echo  %date% %time:~0,8%  Build project
%PATH_MPLAB%\gnuBins\GnuWin32\bin\make %MAKE_PARM% -f nbproject\Makefile-%TP_TARGET%.mk SUBPROJECTS= %MAKE_TARGET_CLEAN% %MAKE_TARGET%
if not "%ERRORLEVEL%"=="0" echo [gen_image.bat] Fail to build bootloader (%TP_TARGET% mode) & goto FINISH
popd


echo --------------------------------------------
echo.
echo.
echo /------------------------------------------------\
echo    %date% %time:~0,8%   
echo    %PRJ_NAME% %MCU_VER% (%TP_TARGET% mode) build pass
echo \------------------------------------------------/
echo.
echo.
echo.




::=================================
:: Build Debug
::=================================
set TP_TARGET=debug

echo.
echo.
echo ----------------------------------------------------------------------------
echo  %date% %time:~0,8%  // Build %PRJ_NAME% (%TP_TARGET% mode) //
echo ----------------------------------------------------------------------------
echo  %date% %time:~0,8%  Generate makefiles for QP 
pushd "%PATH_QP%\"
%PATH_JAVA%\java -jar "%PATH_MPLAB%\mplab_ide\lib\PrjMakefilesGenerator.jar" .\@%TP_TARGET%
popd

echo.
echo  %date% %time:~0,8%  Generate makefiles and clean bootloader
pushd "%PATH_PRJ_BL%"
%PATH_JAVA%\java -jar "%PATH_MPLAB%\mplab_ide\lib\PrjMakefilesGenerator.jar" .\@%TP_TARGET%
if not "%ERRORLEVEL%"=="0" echo [gen_image.bat] Fail to generate bootloader makefile (%TP_TARGET% mode) & goto FINISH
if not "%MAKE_TARGET_CLEAN%" == "" %PATH_MPLAB%\gnuBins\GnuWin32\bin\make %MAKE_PARM% -f nbproject\Makefile-%TP_TARGET%.mk SUBPROJECTS= %MAKE_TARGET_CLEAN%
if not "%ERRORLEVEL%"=="0" echo [gen_image.bat] Fail to build application (%TP_TARGET% mode) & goto FINISH
popd

echo.
echo  %date% %time:~0,8%  Generate makefiles for application
pushd "%PATH_PRJ_AP%"
%PATH_JAVA%\java -jar "%PATH_MPLAB%\mplab_ide\lib\PrjMakefilesGenerator.jar" .\@%TP_TARGET%
if not "%ERRORLEVEL%"=="0" echo [gen_image.bat] Fail to generate application makefile (%TP_TARGET% mode) & goto FINISH
echo.
echo  %date% %time:~0,8%  Build project
%PATH_MPLAB%\gnuBins\GnuWin32\bin\make %MAKE_PARM% -f nbproject\Makefile-%TP_TARGET%.mk SUBPROJECTS= %MAKE_TARGET% %MAKE_TARGET_CLEAN%
if not "%ERRORLEVEL%"=="0" echo [gen_image.bat] Fail to build application (%TP_TARGET% mode) & goto FINISH
popd


echo --------------------------------------------
echo.
echo.
echo /-------------------------------------------------\
echo    %date% %time:~0,8%  
echo    %PRJ_NAME% %MCU_VER% (%TP_TARGET% mode) build pass
echo \-------------------------------------------------/
echo.
echo.
echo.



::=================================
:: Finish
::=================================
:FINISH
echo.
echo.



