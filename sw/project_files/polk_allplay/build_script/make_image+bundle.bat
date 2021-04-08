@echo off
call ..\..\..\test_tools\cdk\tym_tool\gen_image+bundle.bat 2>&1 | tee make_image+bundle.log
pause
