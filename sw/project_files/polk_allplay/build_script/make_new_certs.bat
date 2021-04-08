@echo off
call ..\..\..\test_tools\cdk\tym_tool\gen_new_certs.bat 2>&1 | tee make_new_certs.log
pause
