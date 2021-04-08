@echo off

::************************************
:: User Parameters
::************************************
::Disable log will speed up print
Set LOG_ENABLE=1

::IP Address
set IPADDR=192.168.1.138




::************************************
:: Log
::************************************
FOR /F "TOKENS=1-3 DELIMS=/ " %%A IN ("%DATE%") DO (SET DATE_STR=%%C%%A%%B)
FOR /F "TOKENS=1-3 DELIMS=: " %%A IN ("%TIME%") DO (SET TIME_STR=%%A%%B%%C)
set LOG=.\asetk_print_by_name_%Name%_%DATE_STR%_%TIME_STR:~0,6%.log
if "%LOG_ENABLE%"=="1" (
	echo LOG:    %LOG%
) else (
	echo LOG:    disable
)


::************************************
:: TEE
::************************************
:: For developer, execute ..\..\..\tool\tee.exe directly
:: After release, tee.exe will copy to asetk_thrift folder
if exist ..\tee.exe (
	set TEE=..\tee.exe
) else (
	set TEE=..\..\..\tool\tee.exe
)
echo TEE:    %TEE%



::************************************
:: Main
::************************************

echo IPADDR:  %IPADDR%
echo.
echo Connecting ASE-TK...
echo.

:START

if "%LOG_ENABLE%"=="1" (
	..\thrift2.exe %IPADDR% 1 readinfo 2>&1 | "%TEE%" -a "%LOG%"
) else (
	..\thrift2.exe %IPADDR% 1 readinfo 2>&1
)
echo.
echo -----------------------------------------------------------------

goto START



pause