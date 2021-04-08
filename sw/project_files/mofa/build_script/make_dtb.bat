@echo off
call ..\..\..\test_tools\cdk\tym_tool\gen_tym_bundle_dtb.bat 2>&1 | tee make_dtb.log
pause

