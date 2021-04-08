Brief
-----
A python script to perform system testing over uart using the production test protocol


Installation
------------
1. Install python 2.7 32-bit (install 32-bit even on a 64-bit machine as some libraries may not find the install path)
2. Install install_this\crc16-0.1.1.win32-py2.7.exe (this is the crc library)
3. Install install_this\pyserial-2.7.win32.exe
4. Run system_test from the command line with the com port as an argument, ex: >system_test 1
5. Observe pass or fail

Contents
--------
program.py - program and execute a .hex file, run this with the target release
	ex: >program.py -f ..\..\project_files\polk_allplay\mplab\dist\release\production\mplab.production.hex -v

reset.py - reset the pic
    ex: >reset.py

system_test.py : the test script, run this once programmed with the com port as an argument
	ex: >system_test.py 1

h2py.py : generates pt_external.py and pt_polk_allplay_external.py from the projects .h files, do not run this