#! /usr/bin/env python

""" Defaults """
icd3cmd = "C:\\Program Files (x86)\\Microchip\\MPLABX\\mplab_ide\\bin\\icd3cmd.exe"
pic = "32MX450F256H"

def reset( command=icd3cmd, device=pic, verbose=False ):
    import subprocess
    # program
    p = subprocess.Popen([command, "-P"+device, "-H"], stdout=subprocess.PIPE, stderr=subprocess.PIPE )
    out, err = p.communicate()
    if verbose:
        print out
    p = subprocess.Popen([command, "-P"+device, "-L"], stdout=subprocess.PIPE, stderr=subprocess.PIPE )
    out, err = p.communicate()
    if verbose:
        print out

if __name__ == '__main__':
    import sys
    import argparse

    parser = argparse.ArgumentParser(description='Program and verify a .hex file using an ICD3')
    parser.add_argument('-i', metavar='--icd3', type=str, help='path to icd3cmd.exe', default=icd3cmd)
    parser.add_argument('-d', metavar='--device', type=str, help='pic mcu ex 32MX450F256H', default=pic)
    parser.add_argument('-v', action='store_true', default=False, dest='verbose', help='print verbose output')

    args = parser.parse_args()

    reset( args.i, args.d, args.verbose )
    exit(0)

