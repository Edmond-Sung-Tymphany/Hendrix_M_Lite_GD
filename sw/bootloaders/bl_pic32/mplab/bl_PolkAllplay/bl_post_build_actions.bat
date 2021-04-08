@echo off

set IMAGE_TYPE=%1
set RELEASE_TYPE=%2
set DIR_XC32_BIN=%3

set PATH=%DIR_XC32_BIN%;%PATH%

xc32-objdump -Sdr "dist\%RELEASE_TYPE%\%IMAGE_TYPE%\bl_PolkAllplay.%IMAGE_TYPE%.elf" > "dist\%RELEASE_TYPE%\%IMAGE_TYPE%\bl_PolkAllplay.%IMAGE_TYPE%.asm"
if "%ERRORLEVEL%"=="0" (
	echo dist\%RELEASE_TYPE%\%IMAGE_TYPE%\bl_PolkAllplay.%IMAGE_TYPE%.asm is generated
) else (
	echo Fail to generate dist\%RELEASE_TYPE%\%IMAGE_TYPE%\bl_PolkAllplay.%IMAGE_TYPE%.asm
)
echo.

REM execute a correct commend, then Jenkins will think the commend is OK
cd


