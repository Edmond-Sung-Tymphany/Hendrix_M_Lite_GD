@echo off
call ..\..\..\test_tools\cdk\tym_tool\gen_tym_bundle_sam_fw.bat 2>&1 | tee make_sam_fw.log
pause
