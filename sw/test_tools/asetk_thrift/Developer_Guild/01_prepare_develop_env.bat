@echo off

::*************************************
:: Parameters
::*************************************
set VISUAL_STUDIO=C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe

FOR /F "TOKENS=1-3 DELIMS=/ " %%A IN ("%DATE%") DO (SET DATE_STR=%%C%%A%%B)
set DEV_ENV_ZIP=Developer_Guild\asetk_thrift_develop_env_%DATE_STR%.zip


::*************************************
:: Prepare config file
::*************************************
pushd ..\
call script_before_build.bat
echo.
echo.


::*************************************
:: Prepare zip files
::*************************************
del /f %DEV_ENV_ZIP%
7z.exe a "%DEV_ENV_ZIP%" *.* User_Guild\*.bat User_Guild\*.txt Developer_Guild\*.bat Developer_Guild\*.txt
if not exist "%DEV_ENV_ZIP%" echo *** Fail to generate %RDEV_ENV_ZIP% *** && goto EOF

popd


::*************************************
:: Successful
::*************************************
echo.
echo.
echo *****************************************************************
echo   %DEV_ENV_ZIP%
echo   was generated!
echo *****************************************************************
echo.



::*************************************
:: Finish
::*************************************
:EOF
popd
pause