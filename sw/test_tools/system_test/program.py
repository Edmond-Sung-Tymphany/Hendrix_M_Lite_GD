#! /usr/bin/env python

""" Defaults """
icd3cmd = "C:\\Program Files (x86)\\Microchip\\MPLABX\\mplab_ide\\bin\\icd3cmd.exe"
pic = "32MX450F256H"
hex = "..\\..\\project_files\\polk_allplay\\mplab\\dist\\release\\production\\mplab.production.hex"

def parse_output( out ):
    if "Operation Succeeded" in out:
        return "Pass"
    else:
        return "Fail"

def program( file=hex, command=icd3cmd, device=pic, verbose=False ):
    import subprocess
    # program
    p = subprocess.Popen([command, "-P"+device, "-F"+file, "-M"], stdout=subprocess.PIPE, stderr=subprocess.PIPE )
    out, err = p.communicate()
    if verbose:
        print "Programming: " + file
        print out
    return parse_output( out )

if __name__ == '__main__':
    import sys
    import argparse

    parser = argparse.ArgumentParser(description='Program and verify a .hex file using an ICD3')
    parser.add_argument('-i', metavar='--icd3', type=str, help='path to icd3cmd.exe', default=icd3cmd)
    parser.add_argument('-d', metavar='--device', type=str, help='pic mcu ex 32MX450F256H', default=pic)
    parser.add_argument('-f', metavar='--hex', type=str, help='path to .hex', default=hex)
    parser.add_argument('-v', action='store_true', default=False, dest='verbose', help='print verbose output')

    args = parser.parse_args()

    result = program( args.f, args.i, args.d, args.verbose )
    print result
    if result == "Pass":
        exit(0)
    else:
        exit(1)
