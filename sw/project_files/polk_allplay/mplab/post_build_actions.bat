@echo off

set IMAGE_TYPE=%1
set RELEASE_TYPE=%2
set PATH_XC32_BIN=%3

set PATH=%PATH_XC32_BIN%;%PATH%

xc32-objdump -Sdr "dist\%RELEASE_TYPE%\%IMAGE_TYPE%\mplab.%IMAGE_TYPE%.elf" > "dist\%RELEASE_TYPE%\%IMAGE_TYPE%\mplab.%IMAGE_TYPE%.asm"
if "%ERRORLEVEL%"=="0" (
	echo dist\%RELEASE_TYPE%\%IMAGE_TYPE%\mplab.%IMAGE_TYPE%.asm is generated
) else (
	echo Fail to generate dist\%RELEASE_TYPE%\%IMAGE_TYPE%\mplab.%IMAGE_TYPE%.asm
)
echo.


echo Please generate bundle file manually:
echo    cd "%cd%\..\"
echo    make_bundle.bat


::pushd ..\
::make_bundle.bat
::popd



