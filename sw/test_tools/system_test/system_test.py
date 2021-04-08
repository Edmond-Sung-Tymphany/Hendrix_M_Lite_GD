#! /usr/bin/env python

import unittest
import threading
import time
import sys
import serial
import crc16

SEQ = 0

def delay( x ):
    time.sleep(x)

def calcCrc( message ):
    crc = crc16.crc16xmodem( message )
    msb = ( int(crc)>>8 ) & 0xFF
    lsb = int(crc) & 0xFF
    return {'msb':msb, 'lsb':lsb}

# get a byte value from a hex string ie "01" -> 1
def get_byte( m, offset ):
    start=offset*3
    end=start+2
    try:
        byte = int(m[start:end],16)
    except:
        byte = 0
    return byte

# wait for a specific response discarding other indications, debug messages etc
def wait_for_command_response( s, command, timeout_s ):
    import time
    ret = 0
    start_s = time.time()
    now_s = time.time()
    while( now_s - start_s < timeout_s ):
        response = s.readline()
        rec_resp_cmd = get_byte( response, 2 )
        if( rec_resp_cmd == command ):
            ret = response
            break
        now_s = time.time()
    return ret

# adds crc16 bytes on to the end of the list
# ie: [1,2,3] -> [1,2,3,crc,crc]
def append_crc( data ):
    #calc crc
    crc_in = ""
    for byte in data:
        crc_in = crc_in + chr(byte)
    crc = calcCrc(crc_in)
    data.append(crc['msb'])
    data.append(crc['lsb'])
    return data

# constructs a pt message from a list
# ie. [1,2,3] -> "pt 01 02 03\r"
def pt_marshall( data ):
    message = "pt"
    for byte in data:
        message = message + " %02x" % byte
    message = message + "\r"
    return message

# send a pt command on the serial port in the form of a pt message
def send_pt_command( s, data ):
    data.insert(0, PT_TYPE_COMMAND)
    data.insert(0, SEQ)
    data = append_crc( data )
    message = pt_marshall( data )

    s.flushInput()
    s.write( message )
    s.flush()

#check the last two bytes (crc) of a message
def check_recd_crc( message ):
    num_bytes = len(message)/3
    rec_crc_lsb = get_byte( message, num_bytes-1 )
    rec_crc_msb = get_byte( message, num_bytes-2 )

    data = message[0:len(message)-8] #strip of \r\n and crc bytes
    crc_in = ""
    for i in range((len(data)/3)+1):
        crc_in = crc_in + chr(get_byte(message, i))
    calc_crc = calcCrc( crc_in )

    if( rec_crc_lsb != calc_crc['lsb'] ):
        return "Fail"
    if( rec_crc_msb != calc_crc['msb'] ):
        return "Fail"
    return "Pass"

#check the command response is ok (not in error)
def check_cmd_resp_ok( resp ):
    rec_seq = get_byte( resp, 0 )
    rec_type = get_byte( resp, 1 )
    rec_resp_cmd = get_byte( resp, 2 )

    if( rec_type != PT_TYPE_RESPONSE_OK ):
        return "Fail"

    return "Pass"

#  Protocol version command:
#  SEQ  | TYPE  | CMD  | CRC16_MSB | CRC16_LSB |
#  0x00 | 0x01  | 0x01 | 0x23      | 0x10      |
#
#  Protocol version response:
#  SEQ  | TYPE    | RESP |           ..DATA..            | CRC16_MSB | CRC16_LSB |
#       | RESP OK | CMD  | VERSION MAJOR | VERSION MINOR |           |           |
#  0x01 | 0x02    | 0x01 | 0x00          | 0x01          | 0x60      | 0x28      |
class Test1_ProtocolVersion(unittest.TestCase):
    """Verify protocol version"""
    def setUp(self):
        self.s = serial.Serial(port=COM, baudrate=115200, timeout=1)

    def tearDown(self):
        self.s.close()

    def test_ReadProtocolVersion(self):
        send_pt_command( self.s, [PT_COMMAND_PROTOCOL_VERSION] )
        response = wait_for_command_response( self.s, PT_COMMAND_PROTOCOL_VERSION, 1 )
        #sys.stdout.write("\r\nresponse: %r\n" % response)

        self.failIfEqual( response, 0, "no response")
        self.failIfEqual(check_recd_crc( response ), "Fail", "crc error")
        self.failIfEqual(check_cmd_resp_ok( response ), "Fail", "unexpected response")

        rec_data_1 = get_byte( response, 3 )
        rec_data_2 = get_byte( response, 4 )
        self.failUnlessEqual(PT_PROTOCOL_VERSION_MAJOR, rec_data_1, "unexpected protocol version")
        self.failUnlessEqual(PT_PROTOCOL_VERSION_MINOR, rec_data_2, "unexpected protocol version")

# Command: Read adc (raw value)
# Command data: 1 byte, product specific adc
# Response data: 2 bytes: adc msb | adc lsb
class Test2_ReadADCAC(unittest.TestCase):
    def setUp(self):
        self.s = serial.Serial(port=COM, baudrate=115200, timeout=1)

    def tearDown(self):
        self.s.close()

    def test_ReadADCAC(self):
        send_pt_command( self.s, [PT_COMMAND_ADC, PT_COMMAND_ADC_AC] )
        response = wait_for_command_response( self.s, PT_COMMAND_ADC, 1 )
        #sys.stdout.write("\r\nresponse: %r\n" % response)

        self.failIfEqual( response, 0, "no response")
        self.failIfEqual(check_recd_crc( response ), "Fail", "crc error")
        self.failIfEqual(check_cmd_resp_ok( response ), "Fail", "unexpected response")

        rec_data_1 = get_byte( response, 3 )
        rec_data_2 = get_byte( response, 4 )
        adc = (int(rec_data_1) << 8) + rec_data_2

        self.failIf( adc > 1023, "read adc ac, unexpected value")

# Command: Action MMI (press a button, illuminated an led etc.)
# Command data: up to 3 bytes, product specific instuctions
# Response data: 0 bytes
# Optional indication could follow
# Example: Power on to Allplay (send power key down, power key up then wait for allplay ok indicaion)
class Test3_PowerOnAllplay(unittest.TestCase):
    def setUp(self):
        self.s = serial.Serial(port=COM, baudrate=115200, timeout=1)

    def tearDown(self):
        self.s.close()

    def test_1_PowerKeyDown(self):
        send_pt_command( self.s, [PT_COMMAND_MMI, PT_COMMAND_MMI_BUTTON, PT_COMMAND_MMI_BUTTON_POWER, PT_COMMAND_MMI_BUTTON_DOWN ] )
        response = wait_for_command_response( self.s, PT_COMMAND_MMI, 1 )
        #sys.stdout.write("\r\nresponse: %r\n" % response)

        self.failIfEqual( response, 0, "no response")
        self.failIfEqual( check_recd_crc( response ), "Fail", "crc error" )
        self.failIfEqual( check_cmd_resp_ok( response ), "Fail", "unexpected response" )

    def test_2_Wait250mS(self):
        delay(0.25)

    def test_3_PowerKeyUp(self):
        send_pt_command( self.s, [PT_COMMAND_MMI, PT_COMMAND_MMI_BUTTON, PT_COMMAND_MMI_BUTTON_POWER, PT_COMMAND_MMI_BUTTON_UP ] )
        response = wait_for_command_response( self.s, PT_COMMAND_MMI, 1 )
        #sys.stdout.write("\r\nresponse: %r\n" % response)

        self.failIfEqual( response, 0, "no response")
        self.failIfEqual( check_recd_crc( response ), "Fail", "crc error" )
        self.failIfEqual( check_cmd_resp_ok( response ), "Fail", "unexpected response" )

#    def test_WaitForAllplayOnIndication(self):
#        response = wait_for_command_response( self.s, PT_COMMAND_ALLPLAY_PLAYER_STATE, 90 )
#        self.failIfEqual( response, 0, "power on allplay, no response")

if __name__ == '__main__':
    import sys
    import argparse

    # get external command definitions from the .h files
    from h2py import header_to_py
    header_to_py("..\..\include\pt_external.h")
    header_to_py("..\..\include\pt_polk_allplay_external.h")
    from pt_external import *
    from pt_polk_allplay_external import *

    # get the com port from the command line
    parser = argparse.ArgumentParser(description='System test over the serial port')
    parser.add_argument('port', metavar='COM_port', type=int, help='serial port number')
    args = parser.parse_args()
    COM = args.port-1

    # Check the COM port
    try:
        s = serial.Serial(port=COM, baudrate=115200, timeout=1)
        print "Testing with " + s.name +"\r\n"
        s.close()
    except serial.serialutil.SerialException as e:
        import ctypes
        print "Could not open COM" + str(COM+1) + " - " + str(ctypes.WinError())
        exit(1)

    sys.argv[1:] = ['-v'] # Override the command line args to run all tests, unittest takes args as well, but we won't use them
    # let 'er rip
    unittest.main()
