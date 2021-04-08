#! /usr/bin/env python

# path to product.confg have to be pass as parameter
# parse product.config for SIZE_OF_LARGE_EVENTS value
# parse object_ids.h for SETTING_SRV_ID value
# parse SettingSrv.h eSettingId enum for SETID_DSP_INIT_DATA and SETID_DSP_TUNABLE_PART valuese
# parse signals.h for values of following signals:
#   SETTING_START_REQ_SIG, SETTING_DATA_REQ_SIG, SETTING_END_REQ_SIG, SETTING_START_RESP_SIG, SETTING_DATA_RESP_SIG, SETTING_END_RESP_SIG
# generate sdf.json file with filled with info above


import sys, re, getopt, os, unicodedata
from editingTextFile import *

setting_srv_h = '..\..\server\include\SettingSrv.h'
signals_h = '..\..\include' + '\\' + findFileInDirectory('..\..\include', 'signals.h')
object_ids = '..\..\include\object_ids.h'
sdf_sett = 'sdf_sett.json'


sdf_setting_conf_start = """
[
    {
        "setting conf" :
        {
"""
sdf_setting_conf_end_sig_def_start = """
        }
    },
    {
        "signal definition" :
        [
            {
"""
sfd_start_req_sig_static_part = """
                "sig_name" : "SETTING_START_REQ_SIG",
                "sig_type" : "REQ_SIG_TYPE",
"""
sfd_data_req_sig_static_part = """
                "sig_name" : "SETTING_DATA_REQ_SIG",
                "sig_type" : "REQ_SIG_TYPE",
"""
sfd_end_req_sig_static_part = """
                "sig_name" : "SETTING_END_REQ_SIG",
                "sig_type" : "REQ_SIG_TYPE",
"""
sfd_start_resp_sig_static_part = """
                "sig_name" : "SETTING_START_RESP_SIG",
                "sig_type" : "RESP_SIG_TYPE",
"""
sfd_data_resp_sig_static_part = """
                "sig_name" : "SETTING_DATA_RESP_SIG",
                "sig_type" : "RESP_SIG_TYPE",
"""
sfd_end_resp_sig_static_part = """
                "sig_name" : "SETTING_END_RESP_SIG",
                "sig_type" : "RESP_SIG_TYPE",
"""




sdf_sig_static_part = """
                "sig_display_mode" : 1,
                "sig_event_fields_num" : 0
            }"""

sdf_end = """
        ]
    }
]
"""

createAfileOfName(sdf_sett)



def extractTargetSettingSrvIdValue(input_header_file, start_str, end_str, init_val):
    global SETTING_SRV_ID
    start_line = findAndReturnNumberOfLineInFile(input_header_file, 0, start_str)
    end_line = findAndReturnNumberOfLineInFile(input_header_file, start_line, end_str)
    with open(input_header_file, 'r') as f_in:
        value = init_val
        # get the value of the first Macro
        lines = f_in.readlines()
        for line in lines[start_line: end_line]:
            line = line.rstrip()
            if (line == ''):
                continue
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
            if '*' in s:
                s = ''
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
                if (s == 'SETTING_SRV_ID'):
                    SETTING_SRV_ID = value
                value = value + 1




def extractSettingMacroAndValue(input_header_file, start_str, end_str, init_val):
    global SETID_DSP_INIT_DATA
    global SETID_DSP_TUNABLE_PART
    start_line = findAndReturnNumberOfLineInFile(input_header_file, 0, start_str)
    end_line = findAndReturnNumberOfLineInFile(input_header_file, start_line, end_str)
    with open(input_header_file, 'r') as f_in:
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
                if (s == 'SETID_DSP_INIT_DATA'):
                    SETID_DSP_INIT_DATA = value
                if (s == 'SETID_DSP_TUNABLE_PART'):
                    SETID_DSP_TUNABLE_PART = value
                value = value + 1



def extractSignalsValues(input_header_file, start_str, end_str, init_val):
    global SETTING_START_REQ_SIG
    global SETTING_DATA_REQ_SIG
    global SETTING_END_REQ_SIG
    global SETTING_START_RESP_SIG
    global SETTING_DATA_RESP_SIG
    global SETTING_END_RESP_SIG

    start_line = findAndReturnNumberOfLineInFile(input_header_file, 0, start_str)
    end_line = findAndReturnNumberOfLineInFile(input_header_file, start_line, end_str)
    with open(input_header_file, 'r') as f_in:
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
                if (s == 'SETTING_START_REQ_SIG'):
                    SETTING_START_REQ_SIG = value
                if (s == 'SETTING_DATA_REQ_SIG'):
                    SETTING_DATA_REQ_SIG = value
                if (s == 'SETTING_END_REQ_SIG'):
                    SETTING_END_REQ_SIG = value
                if (s == 'SETTING_START_RESP_SIG'):
                    SETTING_START_RESP_SIG = value
                if (s == 'SETTING_DATA_RESP_SIG'):
                    SETTING_DATA_RESP_SIG = value
                if (s == 'SETTING_END_RESP_SIG'):
                    SETTING_END_RESP_SIG = value
                value = value + 1



# extract signals and write them with values in a .py file
# f_out.close()

def findSettingChunkSizeValue(product_config):
    global SETTING_CHUNK_SIZE
    line_num = findAndReturnNumberOfLineInFile(product_config, 0, '#define SIZE_OF_LARGE_EVENTS')
    print (line_num)
    with open(product_config, 'r') as f_in:
        lines = f_in.readlines()
        # lines = f_in.readlines()
        line_p = lines[line_num]
        part = line_p.partition('SIZE_OF_LARGE_EVENTS')
        part = part[2]
        part = part.lstrip()
        part3 = part.partition(' ')
        val0 = part3[0]
        SETTING_CHUNK_SIZE = int(val0)
    f_in.close()            # for line in lines[start_line: end_line]:


if __name__ == "__main__":
    if (len(sys.argv) > 1):
        project_name = str(sys.argv[1])
    else:
        project_name = '..\..\project_files\SVS_14_Ultra\product.config'
    SETTING_START_REQ_SIG = 0
    SETTING_DATA_REQ_SIG = 0
    SETTING_END_REQ_SIG = 0
    SETTING_START_RESP_SIG = 0
    SETTING_DATA_RESP_SIG = 0
    SETTING_END_RESP_SIG = 0
    SETID_DSP_INIT_DATA = 0
    SETID_DSP_TUNABLE_PART = 0
    SETTING_CHUNK_SIZE = 0
    SETTING_SRV_ID = 0

    extractSignalsValues(signals_h, 'KEY_SIG', '}eSignal', 4)
    extractTargetSettingSrvIdValue(object_ids, 'FIRST_SRV_ID', '}ePersistantObjID', 0)
    extractSettingMacroAndValue(setting_srv_h, 'SETID_START', '}eSettingId', -1)
    findSettingChunkSizeValue(project_name)

    with open(sdf_sett,'wb') as f_out:
        f_out.write(sdf_setting_conf_start)
        stw = '            \"SETTING_CHUNK_SIZE\" : ' + str(SETTING_CHUNK_SIZE) + ', \n'
        f_out.write(stw)
        stw = '            \"SETID_DSP_INIT_DATA\" : ' + str(SETID_DSP_INIT_DATA) + ',\n'
        f_out.write(stw)
        stw = '            \"SETID_DSP_TUNABLE_PART\" : ' + str(SETID_DSP_TUNABLE_PART)
        f_out.write(stw)
        f_out.write(sdf_setting_conf_end_sig_def_start)
        stw = '                \"sig_id\" : ' + str(SETTING_START_REQ_SIG) + ','
        f_out.write(stw)
        f_out.write(sfd_start_req_sig_static_part)
        target_server_line = '                \"sig_target_srv\" : ' + str(SETTING_SRV_ID) + ','
        f_out.write(target_server_line)
        f_out.write(sdf_sig_static_part)
        stw = ',\n            {\n'
        f_out.write(stw)
        stw = '                \"sig_id\" : ' + str(SETTING_DATA_REQ_SIG) + ','
        f_out.write(stw)
        f_out.write(sfd_data_req_sig_static_part)
        f_out.write(target_server_line)
        f_out.write(sdf_sig_static_part)
        stw = ',\n            {\n'
        f_out.write(stw)
        stw = '                \"sig_id\" : ' + str(SETTING_END_REQ_SIG) + ','
        f_out.write(stw)
        f_out.write(sfd_end_req_sig_static_part)
        f_out.write(target_server_line)
        f_out.write(sdf_sig_static_part)
        stw = ',\n            {\n'
        f_out.write(stw)
        stw = '                \"sig_id\" : ' + str(SETTING_START_RESP_SIG) + ','
        f_out.write(stw)
        f_out.write(sfd_start_resp_sig_static_part)
        f_out.write(target_server_line)
        f_out.write(sdf_sig_static_part)
        stw = ',\n            {\n'
        f_out.write(stw)
        stw = '                \"sig_id\" : ' + str(SETTING_DATA_RESP_SIG) + ','
        f_out.write(stw)
        f_out.write(sfd_data_resp_sig_static_part)
        f_out.write(target_server_line)
        f_out.write(sdf_sig_static_part)
        stw = ',\n            {\n'
        f_out.write(stw)
        stw = '                \"sig_id\" : ' + str(SETTING_END_RESP_SIG) + ','
        f_out.write(stw)
        f_out.write(sfd_end_resp_sig_static_part)
        f_out.write(target_server_line)
        f_out.write(sdf_sig_static_part)
        f_out.write(sdf_end)
        f_out.close()



