@echo off
call ..\..\..\test_tools\cdk\tym_tool\gen_image.bat 2>&1 | tee make_image.log
pause
