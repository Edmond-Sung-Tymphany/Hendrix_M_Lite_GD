#!/usr/bin/env python

import argparse
import os
import inspect

"""Changes DSP settings.
@author Dimitris Grimanis
Tymphany Ltd., April 2014
"""

def findFileInDirectory(the_directory_to_look_in, the_file_to_search_for):
    for file in os.listdir(the_directory_to_look_in):
        if file.endswith(the_file_to_search_for):
            return file

def lineno():
    #Returns the current line number in our program.
    return inspect.currentframe().f_back.f_lineno

def createAfileOfName( file_name ):
    if file_name == '':
        print 'the name field cannot be empty.'
        print lineno()
    else:
        try:
            file = open( file_name, 'w+')
            print 'a file named "' + file_name + '" was created.'
        except:
            print 'Error: Could not create file'
            print lineno()
        finally:
            file.close()

def replaceLineInFile(file_to_replace_line_in, line_to_replace, line_to_replace_with ):
    #The whole lines should be introduced.
    with open(file_to_replace_line_in, "r") as file:
        lines = file.readlines()
    with open(file_to_replace_line_in, "w+") as file:
        for line in lines:
            file.write(line.replace(line_to_replace, line_to_replace_with))

def replaceStringInFile(file_to_replace_string_in, string_to_replace, string_to_replace_with ):
    with open(file_to_replace_string_in, "r") as file:
        readFile = file.read()
        readFile = readFile.replace(string_to_replace, string_to_replace_with)
    with open(file_to_replace_string_in, "w+") as file:
        file.write(readFile)

def writeContentAtTheBeginningOfTheFile( file_to_write_to, content_to_write ):
    try:
        file = open( file_to_write_to, 'w+')
        file.seek( 0,0 )
        file.write( content_to_write + '\n' )
    except:
        print 'Error: Could not write to file: ' + file_to_write_to
        print lineno()
    finally:
        file.close()

def appendInTheEndOfAFile( file_to_write_to, content_to_write ):
    with open(file_to_write_to, "a") as myfile:
        myfile.write(content_to_write)

def findSpecificLineInFile( file_to_look_in, string_to_look_for ):
    #the whole line should be introduced.
    the_whole_line = '' #initialise this cause it needs to return in the end.
    try:
        with open( file_to_look_in, 'r' ) as file:
            filelines = file.readlines()
            for line in filelines[:]:
                if string_to_look_for in line:
                    the_whole_line = line.strip()
                    string_found = 1
                    break
                else:
                    string_found = 0
            if string_found == 0:
                print 'Error: the string: ' + string_to_look_for + ' was not found.'
                print lineno()
    except:
        print 'Error: Could not open file: ' + file_to_look_in
        print lineno()
    return the_whole_line

def removeLinesFromFileThatStartWith( file_to_edit, string_to_start_with ):
    try:
        with open( file_to_edit, 'r' ) as file:
            lines = file.readlines()
    except:
        print 'Error: Could not open file: ' + file_to_edit
        print lineno()
    with open( file_to_edit, 'w') as file:
        for line in lines:
            if line.startswith(string_to_start_with):
                pass # well, do nothing
            else:
                file.write(line)

def removeLinesFromFileThatIncludes( file_to_edit, string_includes ):
    try:
        with open( file_to_edit, 'r' ) as file:
            lines = file.readlines()
    except:
        print 'Error: Could not open file: ' + file_to_edit
        print lineno()
    with open( file_to_edit, 'w') as file:
        for line in lines:
            if string_includes in line:
                pass # well, do nothing
            else:
                file.write(line)

def removeLinesFromFileThatEquates( file_to_edit, string_equates ):
    try:
        with open( file_to_edit, 'r' ) as file:
            lines = file.readlines()
    except:
        print 'Error: Could not open file: ' + file_to_edit
        print lineno()
    with open( file_to_edit, 'w') as file:
        for line in lines:
            if string_equates is line:
                line = line.replace( string_equates, '')
            else:
                file.write(line)

def deleteUnwantedTextFromLine( line_to_edit, text_to_get_rid_of ):
    #deletes unwanted text from a line.
    #returns the line after removing the unwanted text.
    cleared_line = ''
    if text_to_get_rid_of in line_to_edit:
        cleared_line = line_to_edit.replace(text_to_get_rid_of,"")
    else:
        print 'could not find ' +text_to_get_rid_of+ ' in the line you specified.'
        print lineno()
    return cleared_line

def findAndReturnNumberOfLineInFile(pathOfReadFile, offset, start_string):
    line_counter = 0
    with open(pathOfReadFile, 'rb') as f:
        input_data = f.readlines()
    for line in input_data[offset:]:
        if line.strip().startswith(start_string):
            break
        line_counter += 1
    return line_counter + offset

def readDataFromFileAndWriteToAnotherFile(pathOfReadFile, start_string, end_string, pathOfWriteFile):
    #part of the line to start with can be introduced. Not necessarily the whole line. the same for end line.
    start_line = findAndReturnNumberOfLineInFile(pathOfReadFile, 0, start_string)
    end_line = findAndReturnNumberOfLineInFile(pathOfReadFile, start_line, end_string)
    with open(pathOfReadFile, 'rb') as f:
        input_data = f.readlines()
    with open(pathOfWriteFile, "ab") as f:
        for line in input_data[start_line:end_line+1]:
            f.write(line)

def deleteLine( file_to_edit, line_to_remove ):
    #only removes the first ocurrence of line_to_remove in file_to_edit.
    with open(file_to_edit, 'rb') as f:
        input_data = f.readlines()
        line_number = findAndReturnNumberOfLineInFile(file_to_edit, 0, line_to_remove)
        ##print input_data[line_number]
    with open(file_to_edit, "wb") as f:
        for line in input_data[0:line_number]:
            f.write(line)
        for line in input_data[line_number+1:]:
            f.write(line)

# add indents at the head of each line between the two strings
def addIndentToLines(file_to_add, start_string, end_string):
    start_line = findAndReturnNumberOfLineInFile(file_to_add, 0, start_string)
    end_line = findAndReturnNumberOfLineInFile(file_to_add, start_line, end_string)

    with open(file_to_add, 'rb') as f:
        input_data = f.readlines()
    with open(file_to_add, "ab") as f:
        for line in input_data[start_line:end_line]:
            line = ' ' * 4 + line

def returnNumberOfLineInFile(pathOfReadFile, offset, target_string):
    line_counter = 0
    with open(pathOfReadFile, 'rb') as f:
        input_data = f.readlines()
    for line in input_data[offset:]:
        if target_string in line:
            break
        line_counter += 1
    return line_counter + offset




