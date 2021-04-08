#!/usr/bin/python 
###########################################################
#
# @file      gen_bundle.py
# @brief     Generate bundle firmware
# @author    Gavin Lee
# @date      23-Mar-2014
# @copyright Tymphany Ltd.
#
# @Reference:
#   http://docs.python.org/2/library/subprocess.html#subprocess.Popen.returncode
#
###########################################################

import sys,os  
import binascii
import subprocess


##############################################
# Parameter Setting
##############################################
#Path
CDK_DIR='%s/..'%sys.path[0]
SW_DIR='%s/../../..'%sys.path[0]
FILE_GEN_FW="%s/tools/bin/gen_firmware"%CDK_DIR
FILE_GEN_FW2="%s/tools/bin/gen_firmware_unix"%CDK_DIR

#Certification folder
#PATH_CERT_MCU='%s/oem/oem'%PATH_CERT
#PATH_CERT_SAM='%s/sam/sam'%PATH_CERT





##############################################
# Parameter Variable
##############################################
#Version
VER_BUNDLE=''
VER_FW_SAM=''
VER_FW_MCU=''
VER_FW_DTB=''

#Path
PATH_TMP_FW_MCU_BINHEX=''
PATH_OUT_FW_BUNDDLE=''
PATH_FW_SAM=''
PATH_FW_MCU=''
PATH_FW_DTB=''
PATH_CERT=''
PATH_CERT_MCU=''
PATH_CERT_SAM=''

#Enable 
ENCRYPT_ENA=0  
SAM_FW_ENA=0
MCU_FW_ENA=0
DTB_FW_ENA=0

#Parameter line
CMD_ENCRYPT=' '
CMD_MCU=' '
CMD_SAM=' '
CMD_DTB=' '

#Parameter for xml
DTB_MANUFA=''
DTB_DEVICE=''







##############################################
# Function
##############################################
def usage_exit():
    print ('''
Usage: python gen_bundle.py --help
       python gen_bundle.py [-e] <manufacturer> <device name> <tmp binhex path> <bundle ver> <out bundle path> [-m <mcu ver> <mcu path>] [-s <sam ver> <sam path>] [-d <dtb ver> <dtb path>]
       -e  encrypt bundl efile
       -m  include MCU firmware
       -s  include SAM firmware
       -d  include DTB firmware

       Example:           
       gen_bundle.py "polk_allplay/certs" -e "Qualcomm" "SamAudio" "out/mcu.binhex" "1.4.33.1.06" "out/bundle-all" -s "1.4.33" "rc2-firmware" -m "1.06" "mcu.hex" -d "1.06" "auto.dtb"
       gen_bundle.py "polk_allplay/certs"    "Qualcomm" "SamAudio" "out/mcu.binhex" "1.4.33.1.06" "out/bundle-sam" -s "1.4.33" 
       gen_bundle.py "polk_allplay/certs"    "Qualcomm" "SamAudio" "out/mcu.binhex" "1.06"        "out/bundle-dtb" -d "1.06" "auto.dtb" 

       Generate bundle file which include MCU, SAM, and DTB files.
''')
    exit(1)



def err_exit(msg):
    print ("[gen_bundle.py] " + msg)
    exit(1)



def exe_cmd(str_cmd, str_err):
    print (str_cmd)
    sp= subprocess.Popen(str_cmd, stderr=subprocess.STDOUT, shell=True)
    (stdout, stderr)= sp.communicate()  #Wait process terminate
    ret= sp.poll()
    if ret!=0:
        print (str_err)
        exit(1)





##############################################
# ANSLYZE PARAMETER
##############################################
#Analyze parameter
def parm_init():
    global ENCRYPT_ENA, VER_BUNDLE, PATH_OUT_FW_BUNDDLE, CMD_ENCRYPT, PATH_TMP_FW_MCU_BINHEX
    global SAM_FW_ENA, VER_FW_SAM, PATH_FW_SAM, CMD_SAM
    global MCU_FW_ENA, VER_FW_MCU, PATH_FW_MCU, CMD_MCU, PATH_CERT, PATH_CERT_MCU, PATH_CERT_SAM
    global DTB_FW_ENA, VER_FW_DTB, PATH_FW_DTB, CMD_DTB, DTB_MANUFA, DTB_DEVICE

    #Check parameter number
    if len(sys.argv)<10 or len(sys.argv)>17:
        usage_exit();

    iParam= 1

    if sys.argv[1]=='--help':
        usage_exit();

    #print 'sys.argv[%d]=%s'%(iParam, sys.argv[iParam])
    PATH_CERT=sys.argv[iParam]
    PATH_CERT_MCU='%s/oem/oem'%PATH_CERT
    PATH_CERT_SAM='%s/sam/sam'%PATH_CERT
    iParam+= 1

    #print 'sys.argv[%d]=%s'%(iParam, sys.argv[iParam])
    if sys.argv[iParam]=="-e":
        ENCRYPT_ENA=1
        iParam+= 1

    #print 'sys.argv[%d]=%s'%(iParam, sys.argv[iParam])
    DTB_MANUFA=sys.argv[iParam]
    iParam+= 1

    #print 'sys.argv[%d]=%s'%(iParam, sys.argv[iParam])
    DTB_DEVICE=sys.argv[iParam]
    iParam+= 1

    #print 'sys.argv[%d]=%s'%(iParam, sys.argv[iParam])
    PATH_TMP_FW_MCU_BINHEX=sys.argv[iParam]
    iParam+= 1
        
    #print 'sys.argv[%d]=%s'%(iParam, sys.argv[iParam])
    VER_BUNDLE=sys.argv[iParam]
    iParam+= 1

    #print 'sys.argv[%d]=%s'%(iParam, sys.argv[iParam])    
    if ENCRYPT_ENA==1:
        PATH_OUT_FW_BUNDDLE="%s.same"%sys.argv[iParam]
        CMD_ENCRYPT='--sam "%s"'%PATH_CERT_SAM
    else:    
        PATH_OUT_FW_BUNDDLE="%s.sam"%sys.argv[iParam]
    iParam+= 1
        
    #print 'sys.argv[%d]=%s'%(iParam, sys.argv[iParam])
    if sys.argv[iParam]=="-s":
        SAM_FW_ENA=1
        VER_FW_SAM=sys.argv[1+iParam]
        PATH_FW_SAM=sys.argv[2+iParam]
        CMD_SAM='sam "%s" "%s"'%(VER_FW_SAM, PATH_FW_SAM)
        if VER_FW_SAM=="":
            err_exit("*** ERROR: SAM version is empty ***")
        if not os.path.exists(PATH_FW_SAM):
            err_exit("*** ERROR: SAM file (%s) is not exist ***"%PATH_FW_SAM)
        iParam+= 3
    if iParam==len(sys.argv):
        return
    elif iParam>len(sys.argv):
        usage_exit();

    #print 'sys.argv[%d]=%s'%(iParam, sys.argv[iParam])
    if sys.argv[iParam]=="-m":
        MCU_FW_ENA=1
        VER_FW_MCU=sys.argv[1+iParam]
        PATH_FW_MCU=sys.argv[2+iParam]
        CMD_MCU='mcu "%s" "%s"'%(VER_FW_MCU, PATH_TMP_FW_MCU_BINHEX)
        if VER_FW_MCU=="":
            err_exit("*** ERROR: MCU version is empty ***")
        if not os.path.exists(PATH_FW_MCU):
            err_exit("*** ERROR: MCU firmware (%s) is not exist ***"%PATH_FW_MCU)
        iParam+= 3
    if iParam==len(sys.argv):
        return
    elif iParam>len(sys.argv):
        usage_exit();
        
    #print 'sys.argv[%d]=%s'%(iParam, sys.argv[iParam])
    if sys.argv[iParam]=="-d":
        DTB_FW_ENA=1
        VER_FW_DTB=sys.argv[1+iParam]
        PATH_FW_DTB=sys.argv[2+iParam]
        if VER_FW_DTB=="":
            err_exit("*** ERROR: DTB version is empty ***")
        if not os.path.exists(PATH_FW_DTB):
            err_exit("*** ERROR: DTB file (%s) is not exist ***"%PATH_FW_DTB)
        CMD_DTB='dtb "%s" "%s"'%(VER_FW_DTB, PATH_FW_DTB)
        iParam+= 3
    if iParam!=len(sys.argv):
        usage_exit();




##############################################
# MAIN
##############################################
#Parameter Analyze
parm_init()

#Create output directory
PATH_OUT_FW_BUNDDLE_BASEDIR=    os.path.split(PATH_OUT_FW_BUNDDLE)[0]
PATH_TMP_FW_MCU_BINHEX_BASEDIR= os.path.split(PATH_TMP_FW_MCU_BINHEX)[0]

if PATH_OUT_FW_BUNDDLE_BASEDIR!="" and not os.path.exists(PATH_OUT_FW_BUNDDLE_BASEDIR):
    os.mkdir(PATH_OUT_FW_BUNDDLE_BASEDIR)
if PATH_TMP_FW_MCU_BINHEX_BASEDIR!="" and not os.path.exists(PATH_TMP_FW_MCU_BINHEX_BASEDIR):
    os.mkdir(PATH_TMP_FW_MCU_BINHEX_BASEDIR)


#Transfer hex to binhex Firmware
if MCU_FW_ENA==1:
    exe_cmd('python "%s/tym_tool/hex2binhex.py" "%s" "%s"'%(CDK_DIR, PATH_FW_MCU,PATH_TMP_FW_MCU_BINHEX), 'Fail for hex2binhex.py')


#gen_firmware must transfer to unix format before execute
#gen_tym_bundle.bat MUST set cygwin's PATH before git path because git's dos2unix have different behavior
exe_cmd( 'test -f "%s" || cat "%s" | dos2unix > "%s"'%(FILE_GEN_FW2,FILE_GEN_FW,FILE_GEN_FW2)  , 'Fail for convert gen_firmware')
#exe_cmd( 'dos2unix "%s/tools/bin/gen_firmware"'%CDK_DIR  , 'Fail for convert gen_firmware')
#exe_cmd( 'sed -e "s/$$/\r/" "%s/tools/bin/gen_firmware" > "%s/tools/bin/gen_firmware_unix"'%(CDK_DIR, CDK_DIR)  , 'Fail for convert gen_firmware')

#Generate Bundle file
#Ex.
#../tools/bin/gen_firmware --out "out.sam" --oem "../certs/oem/oem" --manufacturer "Qualcomm" --id "SamAudio" --bundle-version "1.5.28.1.04" "1.5.26" sam sam_v1.5.26/openwrt-ar71xx-generic-cus227-firmware-v1.5.26-tym-theme mcu "1.04" "../../../project_files/polk_allplay/mplab/dist/debug/production/mplab.production.unified.hex" dtb "1.04" "..\..\..\project_files\polk_allplay\PolkAllPlay_dts_files\cus227_PolkAllPlay_AlwaysUpgrageTest_PolkServer.dtb"
exe_cmd( 'bash "%s" --out "%s" %s --oem "%s" --manufacturer "%s" --id "%s" --bundle-version "%s" %s %s %s'%(FILE_GEN_FW2, PATH_OUT_FW_BUNDDLE, CMD_ENCRYPT, PATH_CERT_MCU, DTB_MANUFA, DTB_DEVICE, VER_BUNDLE, CMD_SAM, CMD_MCU, CMD_DTB ) , 'Fail to execute gen_firmware_unix')


print ("\n[gen_bundle.py] %s is created\n"%PATH_OUT_FW_BUNDDLE)
exit(0)
