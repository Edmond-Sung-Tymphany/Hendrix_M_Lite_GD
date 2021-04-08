@echo off

echo Start uart test %TIME%
CaVaPc.exe %1 "uart"
echo End   uart test %TIME%

mv *.xml ../../../test_tools/cava/reports