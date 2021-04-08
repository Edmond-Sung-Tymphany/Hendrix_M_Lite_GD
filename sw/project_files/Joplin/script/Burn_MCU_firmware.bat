
copy *.hex Joplin.hex

ST-LINK_CLI -c SWD UR
ST-LINK_CLI -ME
ST-LINK_CLI -p Joplin.hex -v "while_programming"
ST-LINK_CLI -OB nBOOT0_SW_Cfg=0
ST-LINK_CLI -Rst

del Joplin.hex
pause
