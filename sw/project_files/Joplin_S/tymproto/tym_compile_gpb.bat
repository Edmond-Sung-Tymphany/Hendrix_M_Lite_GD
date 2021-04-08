
::Set the paths and then compile

::set git tools path
set PATH=c:\Program Files (x86)\Git\bin\;%PATH%
::set the protocol definition directory
set PROTO_DIR=tym_proto
::set output folder name
set OUTPUT_NAME=tym_proto_compiled
::set output directory
set OUTPUT_DIR=../tym_proto_compiled
sh -e tym_compile_gpb_lib.sh
pause