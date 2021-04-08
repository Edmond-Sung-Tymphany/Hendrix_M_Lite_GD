@echo off
set IPADDR=192.168.1.4


echo ******* NOTE *************************************************************
echo   You are overwriting factory setting, it is a dangerous operation.
echo   If you are sure to overwrite gain, please remove "goto EOF"
echo **************************************************************************

goto EOF

..\thrift2.exe %IPADDR% 1 compensate centre fr -3.0
..\thrift2.exe %IPADDR% 1 compensate centre wf -6.0


:EOF
pause