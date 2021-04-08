import sys
import getopt
import argparse
import os
import subprocess

"""
Takes a list of scripts and runs them with output

@author Chris Alexander
@date 07-Aug-2014
"""

#def validateArgs(args):

#def getMakefiles(file)
    

def main(argv):                         
    parser = argparse.ArgumentParser(description="Runs a batch of unit test files and outputs the results in <nameoftests>_tr.xml")
    parser.add_argument('-i', '--input', help="Input File")
    parser.add_argument('-o', '--out', help="Output folder")
    
    tpswtop = os.environ['WORKSPACE'] + "/sw"
    proc = subprocess.Popen(['cygpath', tpswtop],stdout=subprocess.PIPE, 
                           stderr=subprocess.PIPE)
    for line in proc.stdout:
        os.environ['TP_SW_TOP'] = line.rstrip()

    args = parser.parse_args()
	
    if args.input is not None:
        print args.input
        f = open(args.input, "r")
        content = f.readlines()
        for s in content:
            s = s.rstrip()
            subprocess.call(["make","-C",os.path.dirname(s),"-f",os.path.basename(s),"clean"]) 
            subprocess.call(["make","-C",os.path.dirname(s),"-f",os.path.basename(s),"all"])
            
            outfile = "--gtest_output=xml:./" + os.path.splitext(os.path.basename(s))[0] +".xml"
            exeName = s.replace(".mk",".exe").strip()
            if os.path.isfile(exeName):
                subprocess.call([exeName,outfile])
        f.close()

if __name__ == "__main__":
    main(sys.argv[1:])
