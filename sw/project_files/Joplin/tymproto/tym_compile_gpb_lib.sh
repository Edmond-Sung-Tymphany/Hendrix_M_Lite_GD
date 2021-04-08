#!/bin/sh

###########################
# Tool                    #
###########################
export PATH=$PATH:$PWD/nanopb-0.3.8-windows-x86/generator-bin/:$PWD/protoc-gen-doc-v0.8-win32/

if [ ! -d $PROTO_DIR ]; then
	printf >&2 "Error: Unable to find definitions directory.\nMake sure this script is run from root directory\n"
	exit 1
fi

if ! command -v protoc-gen-doc >/dev/null 2>&1; then
	echo >&2 "Error: Unable to find protoc-gen-doc command (documentation generator)"
	unzip ../../../tool/pb_compiler/protoc-gen-doc-v0.8-win32.zip
	#exit 1
fi

if ! command -v protoc >/dev/null 2>&1; then
	echo >&2 "Error: Unable to find protoc command (protobuf compiler)"
	unzip ../../../tool/pb_compiler/nanopb-0.3.8-windows-x86.zip 
	#exit 1
fi



###########################
# Prepare                 #
###########################
rm -f $OUTPUT_DIR/*.*



###########################
# Compile                 #
###########################
echo 
echo [Compiler]
pushd $PROTO_DIR

#Add syntax = 'proto2', otherwise protoc print warning
#find *.proto  -exec sed -i "1i syntax = 'proto2';" {} 2>/dev/null \; 

find *.proto  -exec echo {} \; \
              -exec protoc --nanopb_out=$OUTPUT_DIR -I./ {} \; 

echo.
echo "Replace UENUM to ENUM..."
find $OUTPUT_DIR/*.c -exec vi {} -c ":1,\$s/UENUM/ENUM/g" -c "wq" \;




###########################
# Doc                     #
###########################
echo 
echo [Generate doc]

protoc --doc_out=markdown,README-proto.md:"$OUTPUT_DIR" *.proto
if [ $? != 0 ]; then
	echo >&2 "Error unable to generate markdown documentation"
else
	echo "- $OUTPUT_NAME/README-proto.md"
fi

protoc --doc_out=html,README-proto.html:"$OUTPUT_DIR" *.proto
if [ $? != 0 ]; then
	echo >&2 "Error unable to generate html documentation"
else
	echo "- $OUTPUT_NAME/README-proto.html"
fi

#Remove syntax = 'proto2'
#find *.proto -exec sed -i 's/^syntax.*proto.*$//g' {} 2>/dev/null \;



###########################
# Finish                  #
###########################
popd
echo 
echo -e "\033[32m tym_proto is compiled successfully!!! \033[0m"
#read -n 1 -p "Press any key to continue..."

