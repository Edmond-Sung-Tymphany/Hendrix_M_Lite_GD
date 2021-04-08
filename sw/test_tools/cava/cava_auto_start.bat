rem Uart test ---------------------
echo "\r\n<<<*** CBuffer Test start:"
cava_start_project.bat CBuffer %1
echo "\r\nCBuffer Test end***>>>"
echo "\r\n<<<*** Uart Test start:"
cava_start_project.bat Uart %1
echo "\r\nUart Test end***>>>"
echo "\r\n<<<*** Timer Test start:"
cava_start_project.bat Timer %1
echo "\r\nUart Timer end***>>>"
rem -------------------------------