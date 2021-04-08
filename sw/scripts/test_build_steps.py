#!/usr/bin/env python

import build_steps as bs

import os
import shutil
import unittest

# ==============
# = unit tests =
# ==============
class TestBuildSteps(unittest.TestCase):
    def setUp(self):
        self.test_config_dir = create_test_dir(os.getcwd())
        print self.test_config_dir
        d = {
            # normal
            ("var1_name", "var1_value") : [
                    ("var1opt1_name", "var1opt1_value"),
                    ("var1opt2_name", "var1opt2_value"),
                    ("var1opt3_name", "var1opt2_value")
                ],
            # no variables
            ("var2_name", "var2_value") : [],
            # no value
            tuple(["var3_name"]) : [("var3opt1_name"), ("var3opt2_name")]
        }
        with open(os.path.join(self.test_config_dir, "test_product.xml"), "wb") as f:
            f.write(make_xml("test_product", d))


    # def tearDown(self):
#         delete_test_dir(os.getcwd())


    def test_get_product_list(self):
        # TODO: duplicates?
        self.assertEqual(bs.get_product_list(self.test_config_dir),
            get_all_xml(self.test_config_dir))


    @unittest.skip("fn not yet written")
    def test_build_steps(self):
        pass

    # @unittest.skip("fn not yet written")
    def test_get_xml(self):
        # will fail if test_get_product_list fails
        #print bs.get_product_list(self.test_config_dir)
        for prod in bs.get_product_list(self.test_config_dir):
              # self.assertEqual(bs.get_xml(prod).tostring(), )
              pass


# ========================
# = extra test functions =
# ========================
def create_test_dir(root):
    """Create a directory with some test files/subdirectories."""
    top = os.path.join(root, "test_configs")
    try:
        os.mkdir(top)
    except Exception:
        pass

    for subdir in ["dir1", "dir2"]:
        try:
            subdir_path = os.path.join(top, subdir)
            os.mkdir(subdir_path)
        except Exception:
            pass
        for subsubdir in ["sdir1", "sdir2"]:
            try:
                subsubdir_path = os.path.join(subdir_path, subsubdir)
                os.mkdir(subsubdir_path)
            except Exception:
                pass
            with open(os.path.join(subsubdir_path, "inner_prod.xml"), "wb"):
                pass

    for realfile in ["prod1.XML", "prod2.xml", "prod3.xml", "test_product.xml"]:
        with open(os.path.join(top, realfile), "wb") as f:
            f.write("<product name=\"{}\" />".format(realfile))

    for f in ["fake_product.txt", "dir1/inner_fake"]:
        with open(os.path.join(top, f), "wb"):
            pass


    return top


def delete_test_dir(root):
    shutil.rmtree(os.path.join(root, "test_configs"))


def get_all(directory):
    files = []
    for root, subdirs, subfiles in os.walk(directory):
        del subdirs[:]
        for f in subfiles:
            files.append(os.path.abspath(os.path.join(root, f)))
    return files


def get_all_xml(directory):
    all_xml = []
    #print "get_all: ", get_all(directory)
    for f in get_all(directory):
        if os.path.splitext(f)[1].lower() == '.xml':
            all_xml.append(os.path.splitext(os.path.basename(f))[0])
    return sorted(all_xml)


def make_xml(product, d):
    # I could probably use ElementTree for this...
    xml = "<product name=\"{}\" value=\"TP_PRODUCT\">\n".format(product)
    # d = {(var1_name, var1_value) : [(var1opt1_name, var1opt1_value),
    #           (var1opt2_name, var1opt2_value), ...],
    #   (var2_name, var2_value) : [(var2opt1_name, var2opt1_value), ...
    # ]}
    for variable in d.keys():
        xml += "\t<variable "
        if len(variable) == 2:
            xml += "name=\"{}\" value=\"{}\">\n".format(*variable)
        elif len(variable) == 1:
            xml += "name=\"{}\">\n".format(variable[0])

        for option in d[variable]:
            opt = tuple(option)
            if len(opt) == 2:
                xml += "\t\t<option name=\"{}\" value=\"{}\" />\n".format(*opt)
            elif len(opt) == 1:
                xml += "\t\t<option name=\"{}\" />\n".format(opt[0])
        xml += "\t</variable>\n"
    xml += "</product>\n\n"
    return xml


if __name__ == '__main__':
    unittest.main()
