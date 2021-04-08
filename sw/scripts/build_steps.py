#! /usr/bin/env python
# Ryan Ordille, Tymphany October 2013

from __future__ import print_function  # use python3 style prints

import xml.etree.ElementTree as ET
import os
import sys


def build_steps(product_xml_root):
    """Display choices for variant, debug mode, etc.
    Returns a dictionary of all environment variables to be sourced.
    """
    root = product_xml_root
    print("-----------------------")
    print("Options for product \"{}\":".format(root.get('value')))
    print("-----------------------")
    env_vars = {"TP_PRODUCT": root.get('value')}

    for var in root.findall('variable'):
        value = get_value(var)
        if value:
            env_vars[var.get('name')] = value

    return env_vars


def get_value(variable):
    """From a given 'variable' xml object, return a user's selection
    to be added to the environment variable dictionary.
    """
    # TODO: handle empty options (e.g. <option></options>)
    # these are currently ignored

    options = []
    for opt in variable.findall('option'):
        if opt is not None:
            options.append(opt.text)

    # TODO: human readable name (not just NAME)
    if options:
        selection = prompt_user(options, variable.get('name'))
        # return matching value
        return selection
    else:
        return None


def get_product_list(config_dir):
    """Get the names of products from a directory."""
    products = []
    for root, subdirs, files in os.walk(config_dir):
        # TODO: subdirectories and duplicates
        del subdirs[:]  # ignore subdirectores
        #print (root, subdirs, files)
        for f in files:
            filename, ext = os.path.splitext(f)
            if ext.lower() == ".xml":
                products.append(filename)
    return sorted(products)


def get_xml_root(product, config_dir):
    """Returns ElementTree root object for the given product."""
    xml_file = os.path.join(config_dir, '{}.xml'.format(product))
    return ET.parse(xml_file).getroot()


def prompt_user(options, prompt):
    """Display a user prompt for a set of options.
    Loops until user either selects a valid option, presses enter (selecting
    the 0th option), or gives an EOF (Ctrl+C, raising KeyboardInterrupt and
    exiting script prematurely).
    Returns string of selected option.
    """

    if len(options) == 1:
        print("{} = {}".format(prompt, options[0]))
        return options[0]

    # else prompt user for input
    while True:
        print("Enter {0}:".format(prompt))
        for num, opt in enumerate(options):
            print("{0}: {1}".format(num, opt))
        try:
            selection = input_()
            if not selection:
                # selection is empty (user presses enter)
                return options[0]
            elif 0 <= int(selection) < len(options):
                # selection within range
                return options[int(selection)]
            else:
                # negative number, too big a number, or non-int input given
                raise ValueError
        except ValueError:
            print("[ERROR] Invalid option.")
            # prints options again


def input_(prompt=">>>"):
    """Displays prompt and takes user input. Returns what user gave
    WITHOUT eval()'ing it.
    """
    # python handles user input differently between versions 2.x and 3.x
    if sys.version_info.major == 2:
        try:
            # python 2's input() == eval(raw_input())
            # which is hen bu hao to say the least
            out = raw_input(prompt)
        except EOFError:
            out = ""
        except KeyboardInterrupt:
            print()
            sys.exit(1)
    else:
        # python 3 got rid of raw_input
        try:
            out = input(prompt)
        except EOFError:
            out = ""
        except KeyboardInterrupt:
            print()
            sys.exit(1)
    return out


def get_sw_top():
    """Returns /path/to/tymphany_platform/sw"""
    return os.path.dirname(os.path.dirname(os.path.realpath(sys.argv[0])))


# ============================
# = Main Function for Script =
# ============================
def main():
    top = sys.argv[0]
    config_dir = "{0}/configs".format(
        os.path.dirname(os.path.realpath(top)))
    product_list = get_product_list(config_dir)
    product = prompt_user(product_list, "product")

    # misc exports applicable to all projects
    exports = {}

    if sys.argv:
        exports["TP_SW_TOP"] = get_sw_top()

    ex = build_steps(get_xml_root(product, config_dir=config_dir))
    # merge dictionaries
    exports = dict(exports, **ex)

    print("-----------------------")
    # write env vars to export to file
    with open("./exports", "wb") as f:
        # TODO: python 2 and python 3 support here (.iter())
        for env_var, value in exports.iteritems():
            print("{0}=\"{1}\"".format(env_var, value))
            f.write("export {0}=\"{1}\"\n".format(env_var, value))


if __name__ == "__main__":
    main()
