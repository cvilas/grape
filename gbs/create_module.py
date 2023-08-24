#!/usr/bin/env python3
#
# Copyright (C) 2018-2023 GRAPE Contributors
#

import os
import shutil
import sys
import fileinput

def create_module(module_name):
    """
    Generate a template module to later populate. The module is created in the current location
    :param module_name: Name of the module
    :return:
    """

    if len(sys.argv) != 2:
        print("Usage: python3 create_module.py <module_name>")
        sys.exit(1)

    # Copy the template over
    source_location = os.path.dirname(os.path.abspath(__file__))
    shutil.copytree(source_location+"/module_template", module_name)

    # rename newly created template files
    os.rename(
        os.path.join(module_name, "include", "grape", "@module@", "@module@.h"),
        os.path.join(module_name, "include", "grape", "@module@", module_name + ".h")
    )
    os.rename(
        os.path.join(module_name, "include", "grape", "@module@"),
        os.path.join(module_name, "include", "grape", module_name)
    )
    os.rename(
        os.path.join(module_name, "src", "@module@.cpp"),
        os.path.join(module_name, "src", module_name + ".cpp")
    )

    # Replace all instances of @module@ in files with module name
    for subdir, _, files in os.walk(module_name):
         for f in files:
            file_path = os.path.join(subdir, f)
            with fileinput.FileInput(file_path, inplace=True) as file:
                 for line in file:
                      print(line.replace("@module@", module_name), end='')
    return

if __name__ == '__main__':
    create_module(sys.argv[1])