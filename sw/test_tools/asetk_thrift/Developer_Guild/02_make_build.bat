@echo off

::*************************************
:: Parameters
::*************************************
set VISUAL_STUDIO=C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe
set FINDER_EXE=asetk_finder.exe
set THRIFT_EXE=thrift2.exe

FOR /F "TOKENS=1-3 DELIMS=/ " %%A IN ("%DATE%") DO (SET DATE_STR=%%C%%A%%B)
set RELEASE_ZIP=Developer_Guild\asetk_thrift_release_%DATE_STR%.zip

set VERSION=asetk_thrift_%DATE_STR%


::*************************************
:: Transfer asetk_finder.py to exe
::*************************************
pushd ..\
echo Version: %VERSION% > version.txt
del /F/S/Q  dist  build  obj  .vs  "%FINDER_EXE%" "%THRIFT_EXE%" Developer_Guild\*.zip
py_to_exe.py py2exe
move /Y dist\%FINDER_EXE% .\
if not exist "%FINDER_EXE%" echo *** Fail to generate %FINDER_EXE% *** && goto EOF
echo.
echo.


::*************************************
:: Compile thrift2.exe
:: Ref: http://stackoverflow.com/questions/3892065/building-c-sharp-solutions-from-command-line-with-visual-studio-2010
::*************************************
"%VISUAL_STUDIO%" thrift2.sln /Rebuild
if not exist "%THRIFT_EXE%" echo *** ERROR: %THRIFT_EXE% build fail, please open VC to build again *** && goto EOF



::*************************************
:: Prepare zip files
::*************************************
7z.exe a "%RELEASE_ZIP%" "%THRIFT_EXE%" "%FINDER_EXE%" *.crt *.dll *.exe User_Guild\*.bat User_Guild\*.txt User_Guild\*.exe
if not exist "%RELEASE_ZIP%" echo *** Fail to generate %RELEASE_ZIP% *** && goto EOF
rmdir /S/Q  dist  build  obj  .vs
rm -f thrift2.vshost.exe thrift2.pdb



::*************************************
:: Successful
::*************************************
echo.
echo.
echo *****************************************************************
echo   %RELEASE_ZIP%
echo   was generated!
echo *****************************************************************
echo.



::*************************************
:: Finish
::*************************************
:EOF
popd
pause