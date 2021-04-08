#!/usr/bin/python 
###########################################################
#
# @file      hex2binhex.py
# @brief     Transfer hex file to hex binary file
# @author    Gavin Lee
# @date      16-Mar-2014
# @copyright Tymphany Ltd.
#
# NOTE: Python version shoule be >= 2.7
#
# MCU hex-bin Image header (64bytes), should the same as BootLoader.c
#   typedef struct {
#    /*  4bytes, 0x00~0x03 */ char   magic_str[4]; //should be "IMG."
#    /*  1bytes, 0x04      */ uint8  header_fmt_ver; //header format version
#    /*  1bytes, 0x05      */ uint8  md5_check_ena; //0:no checksum, 1:md5
#    /*  2bytes, 0x06~0x07 */ uint8  reserve1[2];
#    /*  4bytes, 0x08~0x0b */ uint32 build_time_sec;
#    /*  4bytes, 0x0c~0x0f */ uint32 data_len;
#    /* 16bytes, 0x10~0x1f */ char   data_md5[MD5_LEN];
#    /* 16bytes, 0x20~0x3f */ uint32 reserve2[8];
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
# Function
##############################################
def err_exit(msg):
    print( msg )
    exit(1)

def usage_exit():
    print("hex2binhex.py <input hex file> <output binary-hex file>")
    exit(1)





##############################################
# MAIN FUNCTION
##############################################
#Check parameter
if len(sys.argv)!=3:
    usage_exit()


#Parameter Analyze
path_hex= sys.argv[1]
path_binhex= sys.argv[2]
if not os.path.exists(path_hex):
    err_exit("%s is not exist"%path_hex)

#Check Python version
py_ver= platform.python_version()
if int(py_ver.replace(".","")) < 227:
    err_exit("Python version shoule be at least 2.2.7 (current version is %s)"%py_ver)


#File open
fr= open(path_hex, 'r')  
fw= open(path_binhex, "wb")


#hex to buf, and calculate md5
hash = hashlib.md5()
i=0
buf= []
for line in fr.readlines():
    if 0==len(line):
        continue
    line= line.replace("/n","")
    #print (line)
    if line[0]!=":":
        err_exit("ERROR: line %d does not start with ':'"%i)
    for j in range(1, len(line)-1, 2):
        hex_str= "0x" + line[j] + line[j+1]
        hex_bin= struct.pack('B',int(hex_str,0))
        hash.update( hex_bin )
        buf.append( hex_bin )
    i+= 1
    


#Write header:
now_gmt8= time.time() + 8*60*60
fw.write( struct.pack('4s', "IMG.") )  # 4bytes string        => magic_str[4]
fw.write( struct.pack('B',   0    ) )  # 1bytes unsigned char => header_fmt_ver
fw.write( struct.pack('B',   1    ) )  # 1bytes unsigned char => md5_check_ena
fw.write( struct.pack('2B',  0,0  ) )  # 2bytes unsigned char => reserve1[2]
fw.write( struct.pack('L',now_gmt8) )  # 4bytes unsigned int  => build_time_sec
fw.write( struct.pack('L',len(buf)) )  # 4bytes unsigned int  => data_len
fw.write( hash.digest()             )  #16bytes unsigned char => data_md5[16]
fw.write( struct.pack('8L', 0,0,0,0,0,0,0,0))  #16bytes unsigned int  => reserve2[8]

#Write data
for byte in buf:
    fw.write(byte)


#The following line only support Python3.3
#L= list()
#fw.write( bytes(int(x,0) for x in L) )

fr.close()
fw.close()

print ("%s is created (%d bytes)\n"%(path_binhex, len(buf)))
