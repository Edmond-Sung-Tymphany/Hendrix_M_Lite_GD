#!/usr/bin/env python

from __future__ import print_function

import argparse
import datetime
import os
import string
import subprocess
import sys
import re

"""
Create a new driver, server, or application from a template.
Mimicks functionality of the devTools Netbeans/MPLabX plugin.

@author Ryan Ordille
@date 2013-12-05
"""
typestring = "\"App\", \"Dlg\", \"Srv\" or \"Drv\""
def main():
	
	type_ = ""
	classname = ""
	brief = ""
	user = ""
	# template location: /path/to/tymphany_platform/sw/scripts/templates
	templates_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "templates")
	# use git name as username
	user = raw_input("Enter your name (First Last): ")
	while user == "":
		user = raw_input("Enter your name (First Last): ")

	classname = raw_input("Enter a file name (Use CamelBackStyle): ")
	while classname == "":
		classname = raw_input("Enter a file name (Use CamelBackStyle): ")
	
	type_ = raw_input("Enter a type. Can be " + typestring + ": ")
	while invalidType(type_):
		type_ = raw_input("Enter a type. Can be " + typestring + ": ")

	brief = raw_input("Source code brief: ")
	
	template_c_file = "{}/{}C.c.template".format(templates_dir, type_)
	template_h_private_file = "{}/{}HPrivate.h.template".format(templates_dir, type_)
	template_h_public_file = "{}/{}HPublic.h.template".format(templates_dir, type_)
	args_to_template = {"user": user,
						"brief": brief,
						"name": classname,
						"date": str(datetime.date.today()),
						"file": classname.upper()}


	with open(template_c_file, "rb") as f:
		template_c = string.Template(f.read())

	with open(template_h_private_file, "rb") as f:
		template_h_private = string.Template(f.read())

	with open(template_h_public_file, "rb") as f:
		template_h_public = string.Template(f.read())

	output_basename = "{}/{}{}".format(os.getcwd(), classname, type_)
	output_c = "{}.c".format(output_basename)
	output_h_private = "{}_priv.h".format(output_basename)
	output_h_public = "{}.h".format(output_basename)

	with open(output_c, "wb") as f:
		args_to_template['nameAndExt'] = "{}{}.c".format(classname, type_)
		print(args_to_template)
		f.write(template_c.substitute(**args_to_template))

	with open(output_h_private, "wb") as f:
		args_to_template['nameAndExt'] = "{}{}_priv.h".format(classname, type_)
		f.write(template_h_private.substitute(**args_to_template))

	with open(output_h_public, "wb") as f:
		args_to_template['nameAndExt'] = "{}{}.h".format(classname, type_)
		f.write(template_h_public.substitute(**args_to_template))

	print("Generated files:")
	print(output_c)
	print(output_h_public + " <---- put me in the include dir e.g servers//include")
	print(output_h_private + " <---- put me in the same dir as the c file") 

def invalidType(type):
	if type == "":
		print ("Type can not be empty")
		return True
	if not type in ("App", "Dlg", "Srv", "Drv"):
		print("Type must be " + typestring);
		return True
	return False

if __name__ == '__main__':
    main()
