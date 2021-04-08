@echo off

::************************************************
:: Parameter
::************************************************
set DIR_CMD=dir /N/S/B /A:D
set BASE_PATH=%~DP0..\..\..\..
set OUTPUT=source_insight_list.txt



::************************************************
:: Generate list
::************************************************
echo. > %OUTPUT%

::Driver
call:PrintDirRecursive "%BASE_PATH%\sw\driver\adc_driver\stm32"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\audio_driver\stm32"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\gpio_driver\stm32"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\i2c_driver\stm32f0"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\include"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\ioexpander_driver"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\key_driver"
call:PrintDir          "%BASE_PATH%\sw\driver\led_driver"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\led_driver\stm32"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\storage_driver\"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\storage_driver\stm32"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\uart_driver\stm32"

::Server
call:PrintDirRecursive "%BASE_PATH%\sw\server\ase_ng_server"
call:PrintDirRecursive "%BASE_PATH%\sw\server\audio_server"
call:PrintDirRecursive "%BASE_PATH%\sw\server\bt_server"
call:PrintDirRecursive "%BASE_PATH%\sw\server\debug_server"
call:PrintDirRecursive "%BASE_PATH%\sw\server\debug_snky_server"
call:PrintDirRecursive "%BASE_PATH%\sw\server\ds_server"
call:PrintDirRecursive "%BASE_PATH%\sw\server\dss_server"
call:PrintDirRecursive "%BASE_PATH%\sw\server\include"
call:PrintDirRecursive "%BASE_PATH%\sw\server\key_server"
call:PrintDirRecursive "%BASE_PATH%\sw\server\led_server"
call:PrintDirRecursive "%BASE_PATH%\sw\server\power_server"
call:PrintDirRecursive "%BASE_PATH%\sw\server\setting_server"

::Other
call:PrintDirRecursive "%BASE_PATH%\sw\include"
call:PrintDirRecursive "%BASE_PATH%\sw\ui_layer"
call:PrintDirRecursive "%BASE_PATH%\sw\bsp\stm32"
call:PrintDirRecursive "%BASE_PATH%\sw\common"
call:PrintDirRecursive "%BASE_PATH%\sw\hardware_management"
call:PrintDirRecursive "%BASE_PATH%\sw\bootloaders\bl_stm32

::CA17 specific
call:PrintDirRecursive "%BASE_PATH%\sw\project_files\ca17"
call:PrintDirRecursive "%BASE_PATH%\sw\driver\power_driver\stm32"



start notepad "%OUTPUT%"
echo %OUTPUT% is generated
echo.
pause



::************************************************
:: Function Defination
::************************************************
goto EOF
:PrintDirRecursive
	set DIR=%~1
	echo %DIR%
	echo %DIR%>> "%OUTPUT%"
	%DIR_CMD% %DIR%
	%DIR_CMD% %DIR%>> "%OUTPUT%"
	goto EOF
	

:PrintDir
	set DIR=%~1
	::echo %DIR%
	echo %DIR%>> "%OUTPUT%"
	goto EOF
	

:EOF
