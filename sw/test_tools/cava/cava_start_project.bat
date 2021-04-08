rem Uart test ---------------------
cd ../../project_files/CaVa/%1/iar

"C:/Program Files (x86)/IAR Systems/Embedded Workbench 7.2/common/bin/IarBuild.exe" %1.ewp -clean debug
"C:/Program Files (x86)/IAR Systems/Embedded Workbench 7.2/common/bin/IarBuild.exe" %1.ewp -build debug

"C:/Program Files (x86)/STMicroelectronics/STM32 ST-LINK Utility/ST-LINK Utility/ST-LINK_CLI.exe" -ME                           
"C:/Program Files (x86)/STMicroelectronics/STM32 ST-LINK Utility/ST-LINK Utility/ST-LINK_CLI.exe" -P Debug/Exe/%1.hex -Run

cd ../../CaVaTools
rem start_%1.bat %2


echo Start %1 test %TIME%
CaVaPc.exe %2 %1
echo End   %1 test %TIME%

mv *.xml ../../../test_tools/cava/reports

rem -------------------------------