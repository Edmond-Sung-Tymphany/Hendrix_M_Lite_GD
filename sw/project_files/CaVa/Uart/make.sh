PROJECT=$1
TP_TARGET=$2

#clean and compile the project   
echo --------------------------------------------
"C:/Program Files (x86)/IAR Systems/Embedded Workbench 7.2/common/bin/IarBuild.exe" $PROJECT.ewp -clean $TP_TARGET
"C:/Program Files (x86)/IAR Systems/Embedded Workbench 7.2/common/bin/IarBuild.exe" $PROJECT.ewp -build $TP_TARGET
echo --------------------------------------------
