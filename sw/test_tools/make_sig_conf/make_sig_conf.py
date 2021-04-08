#! /usr/bin/env python

# read typedef enum of eSignal(from ), then comuputes each item's and fill in singals.conf as signals config file for TP_SNEAK.
# this script should be run from "tymphany_platform\sw\test_tools\"


import sys, re, getopt, os, unicodedata
from editingTextFile import *

# first find signals.h  under .\include
signals_h = '..\..\include' + '\\' + findFileInDirectory('..\..\include', 'signals.h')
signals_py = 'signals.conf'

# create signals.py 
createAfileOfName(signals_py)

def extractMacroAndValue(input_header_file, output_py_file, start_str, end_str, init_val):
    start_line = findAndReturnNumberOfLineInFile(input_header_file, 0, start_str)
    end_line = findAndReturnNumberOfLineInFile(input_header_file, start_line, end_str)
    with open(input_header_file, 'r') as f_in:
        with open(output_py_file,'ab') as f_out:
            value = init_val
            # get the value of the first Macro
            lines = f_in.readlines()

            for line in lines[start_line: end_line]:

                # separate the line from ','
                part = line.partition(',')
                s = part[0]
                # separate the line from '='
                part = s.partition('=')
                s = part[0]
                # separate the line from'/'
                part = s.partition('/')
                s = part[0]
                s = s.replace(' ', '')
                s = s.replace('\t', '')
                s = s.replace('\n', '')

                # delete all line which includes '/*' or'//'
                if '/*' in s:
                    s = ''
                elif '//' in s:
                    s = ''
                elif '\n' == s:
                    s = ''
                elif '#' in s:
                    s = ''
                elif '' == s:
                    pass
                else:
                    s = '[' + s + ':' + str(value) + ']\r\n'
                    value = value + 1
                f_out.write(unicode(s)) 


# extract signals and write them with values in a .py file
extractMacroAndValue(signals_h, signals_py, 'KEY_SIG', '}eSignal', 4)



