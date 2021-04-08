@echo off

::************************************
:: User Parameters
::************************************
::Disable log will speed up print
Set LOG_ENABLE=1

::Could be a sub-string for device name, for example
::Type "BeoSound 1" can find "BeoSound 1_26296561" and "BeoSound 1_00000000"
set Name=BeoSound 1



::************************************
:: Parameters
::************************************
:: For developer, execute asetk_finder.py directly
:: After release, user do not have .py, thus execute asetk_finder.py
if exist "..\asetk_finder.py" (
	set FINDER=..\asetk_finder.py
) else (
	set FINDER=..\asetk_finder.exe
)
echo FINDER: %FINDER%



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
echo NAME:   %Name%
echo.
echo Searching ASE-TK...
echo.

:START

if "%LOG_ENABLE%"=="1" (
	"%FINDER%" "%Name%" 2>&1 | "%TEE%" -a "%LOG%"
) else (
	"%FINDER%" "%Name%" 
)

goto START



pause