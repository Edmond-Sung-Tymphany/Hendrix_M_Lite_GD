@echo off

set FINDER_EXE=asetk_finder.exe
set THRIFT_EXE=thrift2.exe


pushd ..\
del /F/S/Q  dist  build  obj  .vs  "%FINDER_EXE%" "%THRIFT_EXE%" Developer_Guild\*.zip
::rmdir dist  build  obj
popd


echo.
echo.
echo *****************************************************************
echo   Clean successful
echo *****************************************************************
echo.

pause
