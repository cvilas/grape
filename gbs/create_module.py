#!/usr/bin/env python3
#
# Copyright (C) 2018 GRAPE Contributors
#

import os
import shutil
import sys
import fileinput
import datetime

def create_module(module_name, project_name="grape"):
    """
    Generate a template module to later populate. The module is created in the current location
    :param module_name: Name of the module
    :param project_name: Name of the project (optional. default=grape)
    :return:
    """

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
        os.path.join(module_name, "include", "grape"),
        os.path.join(module_name, "include", project_name)
    )
    os.rename(
        os.path.join(module_name, "src", "@module@.cpp"),
        os.path.join(module_name, "src", module_name + ".cpp")
    )

    # Replace all instances of @tokens@ in files
    year = str(datetime.datetime.now().year)
    for subdir, _, files in os.walk(module_name):
         for f in files:
            file_path = os.path.join(subdir, f)
            with fileinput.FileInput(file_path, inplace=True) as file:
                 for line in file:
                    line = line.replace('@project@', project_name)
                    line = line.replace('@module@', module_name)
                    line = line.replace('@year@', year)
                    print(line, end='')
    return

if __name__ == '__main__':
    num_args = len(sys.argv)
    if num_args < 2 or num_args > 3:
        print("Usage: python3 create_module.py <module_name> [<project_name>]")
        sys.exit(1)
    elif num_args == 2:
        create_module(sys.argv[1])
    else:
        create_module(sys.argv[1], sys.argv[2])
