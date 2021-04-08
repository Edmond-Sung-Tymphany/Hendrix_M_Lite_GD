#!/usr/bin/env python
#*********************************************************************************
#
# Bonjour Test tool for ASE-TK
#
# Install Python and Bonjour package:
#   1. Install Windows Bonjour Service
#		  bonjoursdksetup.exe	
#
#   2. Install 
#		  Python v2.7
#
#   3. Install pybonjour
#		 wget https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/pybonjour/pybonjour-1.1.1.zip
#		 unzip pybonjour-1.1.1.zip
#		 cd pybonjour-1.1.1
#		 python setup.py install
#
#   4. Install Crypto
#		 http://www.voidspace.org.uk/python/pycrypto-2.6.1/pycrypto-2.6.1.win32-py2.7.msi
#
#   5. Install paramiko
#		 wget https://pypi.python.org/packages/source/p/paramiko/paramiko-1.7.7.1.zip
#		 unzip paramiko-1.7.7.1.zip
#		 cd paramiko-1.7.7.1/
#		 python setup.py build && python setup.py install
#
# Usage:
#   python asetk_bonjour_test.py
#      => No parameter will print all ASE-TK
#   python asetk_bonjour_test.py "BeoSound 1"
#      => One parameter will print all ASE-TK which include this string
#
#
# Touble Shooting:
#   pybonjour.BonjourError: (-65563, 'unknown')
#   ==> Bonjour servcie may not start
#
# Author: Gavin Lee 
# Date: 2016/4/1
#
#*********************************************************************************

import select
import socket
import sys
import traceback
import time
import os

#External module

try:
	import pybonjour
except Exception as e:
	print e
	print "ERROR: Please install bonjoursdksetup.exe"
	sys.exit(-1)

#import Crypto
#import paramiko
#import pxssh

#==============================================
# Parameters
#==============================================
#Normal parameters
#QUERY_TYPE = "_airplay._tcp"
QUERY_TYPE = "_beo_settings._tcp"
DNSSD_QUERY_TIMEOUT  = 5
#SSH_PORT = 22
#UDP_RECV_TIMEOUT	 = 5
#SSH_PATH= "C:\Program Files (x86)\Git\bin\ssh.exe"



#==============================================
# Global Variables
#==============================================
queried  = []
resolved = []
asetkNameReq= ""




#==============================================
# Common
#==============================================
def print_usage():
	print ""
	print "asetk_finder.exe [name]"
	print "  [name] is optional, to query the ASE-TK whose name include it"
	print ""
	print "ex."
	print "   asetk_finder.exe"
	print "   asetk_finder.exe \"BeoSound 1\""
	print ""



#==============================================
# Discovery Function
#==============================================
def query_record_callback(sdRef, flags, interfaceIndex, errorCode, fullname,
						  rrtype, rrclass, rdata, ttl):
	if errorCode == pybonjour.kDNSServiceErr_NoError:
		ipaddr= socket.inet_ntoa(rdata)
		print '/ %s ]'%ipaddr
		#print 'echo "nsdk_cli invoke \"beo_ASETK:requests/getVersionInfo\"" | "C:/Program Files (x86)/Git/bin/ssh.exe" -p 22 root@%s'%ipaddr
		#exec('cd "C:/Program Files (x86)/Git/bin/ssh.exe" && echo "nsdk_cli invoke \"beo_ASETK:requests/getVersionInfo\"" | ssh -p 22 root@%s'%ipaddr)

		if os.path.isdir(sys.path[0]):
			#asetk_finder.py ==> sys.path[0]=xxx\
			curr_path= sys.path[0] 
		else:
			#asetk_finder.exe ==> sys.path[0]=xxx\asetk_finder.exe
			curr_path= os.path.dirname(sys.path[0])
		#print "sys.path[0]: %s"%sys.path[0]
		#print "curr_path: %s"%curr_path

		commend= "%s\\thrift2.exe %s 1 readinfo"%(curr_path,ipaddr)
		#commend= "%s\\thrift2.exe %s 1 echo 5 1 2 3 4 5"%(curr_path,ipaddr)
		output = os.popen(commend)
		print output.read()
		#os.system(commend)

		queried.append(True)


def resolve_callback(sdRef, flags, interfaceIndex, errorCode, fullname,
					 hosttarget, port, txtRecord):
	global asetkNameReq
	if errorCode != pybonjour.kDNSServiceErr_NoError:
		return

	#print '  hosttarget =', hosttarget
	#print '  fullname   =', fullname
	record= txtRecord.split("=")
	if record[0].find("DEVICE_TYPE")>=0:
		devName= record[1]
	else:
		devName= "Unknown"
	
	friendlyName= fullname.split(".")[0].replace("\\032"," ")

	if asetkNameReq=="" or friendlyName.find(asetkNameReq)>=0:
		print '\n-----------------------------------------------------------------',
		print '\n[%s / %s'%(devName,friendlyName),

		query_sdRef = \
			pybonjour.DNSServiceQueryRecord(interfaceIndex = interfaceIndex,
											fullname = hosttarget,
											rrtype = pybonjour.kDNSServiceType_A,
											callBack = query_record_callback)

		try:
			while not queried:
				ready = select.select([query_sdRef], [], [], DNSSD_QUERY_TIMEOUT)
				if query_sdRef not in ready[0]:
					print 'Query record timed out'
					break
				pybonjour.DNSServiceProcessResult(query_sdRef)
			else:
				queried.pop()
		except Exception as e:
			print e
			#sys.exit(-1)
		finally:
			query_sdRef.close()

		resolved.append(True)


def browse_callback(sdRef, flags, interfaceIndex, errorCode, serviceName,
					regtype, replyDomain):
	if errorCode != pybonjour.kDNSServiceErr_NoError:
		return

	if not (flags & pybonjour.kDNSServiceFlagsAdd):
		print 'Service removed'
		return

	#print '\nService added; resolving'

	resolve_sdRef = pybonjour.DNSServiceResolve(0,
												interfaceIndex,
												serviceName,
												regtype,
												replyDomain,
												resolve_callback)

	try:
		while not resolved:
			ready = select.select([resolve_sdRef], [], [], DNSSD_QUERY_TIMEOUT)
			if resolve_sdRef not in ready[0]:
				#print 'Resolve timed out'
				break
			pybonjour.DNSServiceProcessResult(resolve_sdRef)
		else:
			resolved.pop()
	except Exception as e:
		print e
		#sys.exit(-1)
	finally:
		resolve_sdRef.close()




#==============================================
# Main
#==============================================
#print debug message
#print "ASE-TK Tool"
#print "  service type: %s"%QUERY_TYPE
#print "  port: %d"%SSH_PORT
#print ""

if len(sys.argv)>2:
	print_usage()
	sys.exit(-1)

if len(sys.argv)==2:
	asetkNameReq= sys.argv[1]
#print "asetkNameReq= %s"%asetkNameReq

try:
	browse_sdRef = pybonjour.DNSServiceBrowse(regtype  = QUERY_TYPE,
											  callBack = browse_callback)
except pybonjour.BonjourError as e:
	print "ERROR: Bonjour service is not exist"
	#print e
	sys.exit(-1)


try:
	#while 1:
		#print "\n\n\n!!!! Start new Discovery !!!! \n\n\n"
		time.sleep(3) #Wait bonjour response
		ready = select.select([browse_sdRef], [], [])
		if browse_sdRef in ready[0]:
			pybonjour.DNSServiceProcessResult(browse_sdRef)
except Exception as e:
	print e
	sys.exit(-1)
finally:
	browse_sdRef.close()
