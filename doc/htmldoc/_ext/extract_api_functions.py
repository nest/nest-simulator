# -*- coding: utf-8 -*-
#
# extract_api_functions.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.
import os
import ast
import json
import re


def find_all_variables(file_path):
    """
    This function gets the names of all functions listed in ``__all__``
    in each of the PyNEST API files, along with the Kernel Attributes
    found in ``__init__.py`` of ``pynest/nest/``.
    """
    all_variables = None

    if "pynest/nest/__init__" in file_path:
        # Read the __init__.py file
        with open(file_path, "r") as init_file:
            file_content = init_file.read()

        # Find the class definition
        match = re.search(r"class\s+NestModule\(.*?\):", file_content, re.DOTALL)
        if match:
            # Find the variable assignments within the class
            all_variables = re.findall(r"(\w+)\s*=\s*KernelAttribute", file_content)

    with open(file_path, "r") as file:
        try:
            tree = ast.parse(file.read())
        except SyntaxError:
            # Skip files with syntax errors
            return None

    for node in ast.iter_child_nodes(tree):
        if isinstance(node, ast.Assign) and len(node.targets) == 1:
            target = node.targets[0]
            if isinstance(target, ast.Name) and target.id == "__all__":
                value = node.value
                if isinstance(value, ast.List):
                    all_variables = [elem.s for elem in value.elts if isinstance(elem, ast.Str)]
                break

    return all_variables


def process_directory(directory):
    """
    Get the PyNEST API files and set the keys to the base name
    """
    result = {}
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".py"):
                file_path = os.path.join(root, file)
                # There are two hl_api_spatial files (one in ``lib``, the other in
                # ``spatial``) in PyNEST, so we need to distinguish them from each other
                if "lib/hl_api_spatial" in file_path:
                    file = "lib.hl_api_spatial.py"
                # We want the nestModule kernel attributes, which are in ``__init__``
                # Rename the key to nestModule
                if "pynest/nest/__init__" in file_path:
                    file = "nestModule.py"
                filename = os.path.splitext(file)[0]
                all_variables = find_all_variables(file_path)
                if all_variables:
                    result[filename] = all_variables
    return result


def ExtractPyNESTAPIS():
    directory = "../../pynest/nest/"
    all_variables_dict = process_directory(directory)

    with open("api_function_list.json", "w") as outfile:
        json.dump(all_variables_dict, outfile, indent=4)


if __name__ == "__main__":
    ExtractPyNESTAPIS()
