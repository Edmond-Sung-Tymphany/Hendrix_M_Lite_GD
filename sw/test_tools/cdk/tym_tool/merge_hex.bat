@echo off


::=================================
:: Parameter
::=================================
set TOOL_HEXMATE=C:/Microchip/MPLABX/mplab_ide/bin/hexmate
set TOOL_EDF=C:/Microchip/MPLABX/mplab_ide/dat/en_msgs.txt

::set HEX_BL=..\..\..\bootloaders\bl_pic32\mplab\bl_PolkAllplay\dist\debug\production\bl_PolkAllplay.production.hex
set HEX_BL=..\..\..\bootloaders\bl_pic32\mplab\bl_mofa\dist\debug\production\bl_mofa.production.hex
set HEX_AP=..\..\..\project_files\polk_allplay\mplab\dist\debug\production\mplab.production.hex
set HEX_MERGE=out\merge.hex


::=================================
:: Initialization
::=================================
rm -rf out
mkdir out


::=================================
:: START
::=================================
copy "%HEX_BL%" "out\hex_bl.hex"
copy "%HEX_AP%" "out\hex_ap.hex"
@"%TOOL_HEXMATE%" --edf="%TOOL_EDF%" "%HEX_AP%" "%HEX_BL%" -o%HEX_MERGE%"



:FINISH
pause