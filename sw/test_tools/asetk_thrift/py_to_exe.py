#!/usr/bin/env python
#==========================================================
# This script is to transfer python to exe
#
# Usage:
#   python asetk_finder_toExe.py py2exe	
#
#=====================================================


from distutils.core import setup
import py2exe

py_name=  'asetk_finder.py'


setup(
    options = {'py2exe': {'bundle_files': 1}},
    console = [{'script': py_name}],
    zipfile = None,
)