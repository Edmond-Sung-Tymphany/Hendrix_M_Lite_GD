#!/bin/sh

###########################
# Tool                    #
###########################
export PATH=$PATH:$PWD/nanopb-0.3.7-windows-x86/generator-bin/:$PWD/protoc-gen-doc-v0.8-win32/

if [ ! -d ../ase_ng_proto ]; then
	printf >&2 "Error: Unable to find definitions directory.\nMake sure this script is run from root directory\n"
	exit 1
fi

if ! command -v protoc-gen-doc >/dev/null 2>&1; then
	echo >&2 "Error: Unable to find protoc-gen-doc command (documentation generator)"
	unzip protoc-gen-doc-v0.8-win32.zip
	#exit 1
fi

if ! command -v protoc >/dev/null 2>&1; then
	echo >&2 "Error: Unable to find protoc command (protobuf compiler)"
	unzip nanopb-0.3.7-windows-x86.zip 
	#exit 1
fi



###########################
# Prepare                 #
###########################
rm -f ../ase_ng_proto_compiled/*



###########################
# Compile                 #
###########################
echo 
echo [Compiler]
pushd ../ase_ng_proto

#Add syntax = 'proto2', otherwise protoc print warning
#find *.proto  -exec sed -i "1i syntax = 'proto2';" {} 2>/dev/null \; 

find *.proto  -exec echo {} \; \
              -exec protoc --nanopb_out=../ase_ng_proto_compiled -I./ {} \; 


#For python output
#find *.proto  -exec echo {} \; \
#              -exec protoc --python_out=../ase_ng_proto_compiled -I./ {} \; 




echo.
echo "Replace UENUM to ENUM..."
find ../ase_ng_proto_compiled/*.c -exec vi {} -c ":1,\$s/UENUM/ENUM/g" -c "wq" \;




###########################
# Doc                     #
###########################
echo 
echo [Generate doc]

protoc --doc_out=markdown,README-proto.md:"../ase_ng_proto_compiled" *.proto
if [ $? != 0 ]; then
	echo >&2 "Error unable to generate markdown documentation"
else
	echo "- ase_ng_proto_compiled/README-proto.md"
fi

protoc --doc_out=html,README-proto.html:"../ase_ng_proto_compiled" *.proto
if [ $? != 0 ]; then
	echo >&2 "Error unable to generate html documentation"
else
	echo "- ase_ng_proto_compiled/README-proto.html"
fi

#Remove syntax = 'proto2'
#find *.proto -exec sed -i 's/^syntax.*proto.*$//g' {} 2>/dev/null \;



###########################
# Finish                  #
###########################
popd
echo
read -n 1 -p "Press any key to continue..."

