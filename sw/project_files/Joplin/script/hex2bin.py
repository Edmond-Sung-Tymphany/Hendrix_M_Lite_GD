#!/usr/bin/python
###########################################################
#
# @file      hex2bin.py
# @brief     Convert hex file to binary file
# @author    Daniel Qin
# @date      27-Mar-2018
# @copyright Tymphany Ltd.
#
# NOTE: Python version shoule be >= 2.7
#
# MCU hex-bin Image header (64bytes)
#   typedef struct {
#    /*  4bytes, 0x00~0x03 */ char   magic_str[4]; //should be "IMG."
#    /*  1bytes, 0x04      */ uint8  header_fmt_ver; //header format version
#    /*  1bytes, 0x05      */ uint8  md5_check_ena; //0:no checksum, 1:md5
#    /*  2bytes, 0x06~0x07 */ uint8  reserve1[2];
#    /*  4bytes, 0x08~0x0b */ uint32 fw_ver; // firmware version 0x01 00 01 00: 1.0.1.0
#    /*  4bytes, 0x0c~0x0f */ uint32 data_len;
#    /*  4bytes, 0x10~0x13 */ uint32 data_md5;
#    /* 44bytes, 0x14~0x3f */ uint32 reserve2[11];
#    /* N bytes, 0x40~     */ //data
#   }ImgHeader;
#
###########################################################

import sys,os
import binascii
import platform
import struct
import hashlib
import time
from datetime import datetime,timedelta

##############################################
# Constants
##############################################
PARAM_NUM=5
HEX_TYPE_OFFSET=7
HEX_DATA_OFFSET=9
HEX_CHECKSUM_LEN=2
CHECKSUM_ADDR_STR=":04F808"

##############################################
# Function
##############################################
def err_exit(msg):
    print( msg )
    exit(1)

# version number should be in the format as: 1.0.1.0 or 1.0.1
def usage_exit():
    print("hex2bin.py <input hex file> <output binary-hex file> <version number> <fw checksum>")
    exit(1)


##############################################
# MAIN FUNCTION
##############################################
#Check parameter
if len(sys.argv)!=PARAM_NUM:
    usage_exit()


#Parameter Analyze
path_hex= sys.argv[1]
path_bin= sys.argv[2]
fw_ver= sys.argv[3]
checksum= sys.argv[4]

#Convert fw version string to binary
fw_ver=fw_ver.replace(".","")
if len(fw_ver) ==3:
	fw_ver +="0"

if not os.path.exists(path_hex):
    err_exit("%s is not exist"%path_hex)

#Check Python version
py_ver= platform.python_version()
if int(py_ver.replace(".","")) < 227:
    err_exit("Python version shoule be at least 2.2.7 (current version is %s)"%py_ver)


#File open
fr= open(path_hex, 'r')
fw= open(path_bin, "wb")


#hex to buf, and calculate md5
hash = hashlib.md5()
i=1
buf= []
for line in fr.readlines():
	if 0==len(line):
		i+= 1
		continue
	line= line.replace("/n","")
	#print (line)
	if line[0]!=":":
		err_exit("ERROR: line %d does not start with ':'"%i)
	if line[HEX_TYPE_OFFSET]!="0" or line[HEX_TYPE_OFFSET+1]!="0" or  CHECKSUM_ADDR_STR in line:
		#print("ERROR:line %d is not data"%i)
		#print (line)
		i+= 1
		continue

	for j in range(HEX_DATA_OFFSET, len(line)-HEX_CHECKSUM_LEN-1, 2):
		hex_str= "0x" + line[j] + line[j+1]
		hex_bin= struct.pack('B',int(hex_str,0))
		hash.update( hex_bin )
		buf.append( hex_bin )
	i+= 1

#Write header:
now_gmt8= time.time() + 8*60*60
fw.write( struct.pack('4s', b"IMG.") )  # 4bytes string        => magic_str[4]
fw.write( struct.pack('B',   0    ) )  # 1bytes unsigned char => header_fmt_ver
fw.write( struct.pack('B',   1    ) )  # 1bytes unsigned char => md5_check_ena
fw.write( struct.pack('2B',  0,0  ) )  # 2bytes unsigned char => reserve1[2]
fw.write( struct.pack('4s', fw_ver.encode('utf-8')) )  # 4bytes string  => firmware version
fw.write( struct.pack('L',len(buf)) )  # 4bytes unsigned int  => data_len
fw.write( struct.pack('L',int(checksum, 16)) )  #16bytes unsigned char => data_md5[16]
fw.write( struct.pack('11L', 0,0,0,0,0,0,0,0,0,0,0))  #16bytes unsigned int  => reserve2[11]

#Write data
for byte in buf:
    fw.write(byte)


#The following line only support Python3.3
#L= list()
#fw.write( bytes(int(x,0) for x in L) )

fr.close()
fw.close()

print ("%s is created (%d bytes)\n"%(path_bin, len(buf)))
