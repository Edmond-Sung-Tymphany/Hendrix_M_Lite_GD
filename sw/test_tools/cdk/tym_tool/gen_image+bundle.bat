@echo off
::=================================
:: Build
::=================================
:Start Time
set strTimeStart=%time:~0,8%

::Build Image (Need 07:00 sec)
call "%~dp0\gen_image.bat"
if not exist "%PATH_PRJ_AP%\dist\release\production\mplab.production.unified.hex" echo *** Fail to build images *** & goto FINISH
if not exist "%PATH_PRJ_AP%\dist\debug\production\mplab.production.unified.hex"   echo *** Fail to build images *** & goto FINISH

::Build Bundle (Need 01:10 sec)
call "%~dp0\gen_tym_bundle.bat"


::=================================
:: Finish
::=================================
:FINISH
echo.
echo.
echo --------------------------------------------
echo   Start time: %strTimeStart:~0,8% 
echo   End time:   %time:~0,8% 
echo --------------------------------------------
echo.
echo.

