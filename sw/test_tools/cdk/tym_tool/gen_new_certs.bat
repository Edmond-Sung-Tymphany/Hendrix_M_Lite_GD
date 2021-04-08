@echo off


echo Press any key to generate and overwrite certificates. Close window to cancel.
pause


::=================================
:: Parameter
::=================================
set PARAM_COUNTRY=US
set PARAM_STATE=California
set PARAM_LOCALITY=San Francisco
set PARAM_ORGANIZATION=Qualcomm Connected Experiences, Inc.


::======================================
:: Get project and tool information
::======================================
if not exist parm_proj_info.bat echo *** [gen_tym_bundle.bat] parm_proj_info.bat is not exist *** & goto FINISH
call "parm_proj_info.bat"
call "%~dp0\parm_tools.bat"
if not "%RET_PARM_TOOLS%"=="1" echo *** [gen_tym_bundle.bat] parm_tools.bat is fail *** & goto FINISH

::Tool check
if not exist "%PATH_BASH%\bash.exe"     echo *** [gen_tym_bundle.bat] Please install cygwin to %PATH_BASH%\ *** & goto FINISH

::gen_tym_bundle.bat need these variables:
if "%PATH_CERT%"==""     echo *** Do not call gen_tym_bundle.bat directly (PATH_CERT is empty) ***    & goto FINISH


::=================================
:: Global Variable
::=================================
::Build Result
set RET_GEN_NEW_CERTS=0


::=================================
:: Initialization
::=================================
rm -rf "%PATH_CERT%\ca" "%PATH_CERT%\oem" "%PATH_CERT%\sam"
if not exist "%PATH_CERT%" mkdir "%PATH_CERT%" 


::=============================================================
:: Generate bundle file
::=============================================================
pushd "%~dp0\..\certs\"

rm -rf ca oem sam

cat "make_certificates" | dos2unix > "make_certificates_unix"

bash "make_certificates_unix" -c "%PARAM_COUNTRY%" -st "%PARAM_STATE%" -l "%PARAM_LOCALITY%" -org "%PARAM_ORGANIZATION%" -cn "AllPlay CA" ca
::  Generate ./ca folder with files
::    9B768ECA98152AE9.pem 
::    ca.crt
::    ca.csr
::    ca.key
::    index.txt
::    index.txt.attr
::    index.txt.old
::    serial

if not "%ERRORLEVEL%"=="0" echo *** [gen_new_certs.bat] Fail to generate CA certificates *** && goto FINISH

bash "make_certificates_unix" -c "%PARAM_COUNTRY%" -st "%PARAM_STATE%" -l "%PARAM_LOCALITY%" -org "%PARAM_ORGANIZATION%" -cn "AllPlay firmware oem" oem
::  Generate ./oem folder with files: 
::    oem.crt  oem.csr  oem.key

if not "%ERRORLEVEL%"=="0" echo *** [gen_new_certs.bat] Fail to generate OEM certificates *** && goto FINISH

bash "make_certificates_unix" -c "%PARAM_COUNTRY%" -st "%PARAM_STATE%" -l "%PARAM_LOCALITY%" -org "%PARAM_ORGANIZATION%" -cn "AllPlay firmware sam" sam
::  Generate ./sam folder with files:
::    sam.crt  sam.csr  sam.key

if not "%ERRORLEVEL%"=="0" echo *** [gen_new_certs.bat] Fail to generate SAM certificates *** && goto FINISH

move ca "%PATH_CERT%\"
if not "%ERRORLEVEL%"=="0" echo *** [gen_new_certs.bat] Fail to move ca *** && goto FINISH

move oem "%PATH_CERT%\"
if not "%ERRORLEVEL%"=="0" echo *** [gen_new_certs.bat] Fail to move oem *** && goto FINISH

move sam "%PATH_CERT%\"
if not "%ERRORLEVEL%"=="0" echo *** [gen_new_certs.bat] Fail to move sam *** && goto FINISH


popd


::=================================
:: Finish
::=================================
set RET_GEN_NEW_CERTS=1

:FINISH
echo.
echo.
echo /-------------------------------------------------------------------------\
echo .  %date% %time:~0,8%  

if "%RET_GEN_NEW_CERTS%"=="1"   echo .   Certificate is generated

if not "%RET_GEN_NEW_CERTS%"=="1" (
	echo    *** [gen_new_certs.bat] generate certificate FAIL ***
)
echo \-------------------------------------------------------------------------/
echo.

::Let jenkins know build fail
if not "%RET_GEN_NEW_CERTS%"=="1" set ERRORLEVEL=999
if "%RET_GEN_NEW_CERTS%"=="1" set ERRORLEVEL=0

