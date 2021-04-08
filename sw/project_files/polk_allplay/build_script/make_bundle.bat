@echo off
call ..\..\..\test_tools\cdk\tym_tool\gen_tym_bundle.bat 2>&1 | tee make_bundle.log
pause
