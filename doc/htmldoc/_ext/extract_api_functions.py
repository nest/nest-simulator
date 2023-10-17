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
import ast
import glob
import os
import re

from sphinx.application import Sphinx

"""
Generate a JSON dictionary that stores the module name as key and corresponding
functions as values, along with the ``NestModule`` and the kernel attributes.
Used in a Jinja template to generate the autosummary for each module in
the API documentation (``ref_material/pynest_api/``)
"""


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
    Get the PyNEST API filenames and set the keys to the base name
    """
    api_dict = {}
    api_exception_list = ["raster_plot", "visualization", "voltage_trace"]
    files = glob.glob(directory + "**/*.py", recursive=True)

    for file in files:
        # ignoring the low level api and connection_helpers and helper modules
        if "helper" in file or "ll_api" in file:
            continue

        # get the NestModule for the kernel attributes
        if "pynest/nest/__init__" in file:
            api_name = "nest.NestModule"

        parts = file.split(os.path.sep)
        nest_index = parts.index("nest")
        module_name = os.path.splitext(parts[-1])[0]
        # only get high level API modules
        if "hl_" in file:
            module_path = ".".join(parts[nest_index + 1 : -1])
            api_name = f"nest.{module_path}.{module_name}"
        for item in api_exception_list:
            if item in file:
                api_name = f"nest.{module_name}"

        all_variables = find_all_variables(file)
        if all_variables:
            api_dict[api_name] = all_variables

    return api_dict


def get_pynest_list(app, env, docname):
    directory = "../../pynest/nest/"

    if not hasattr(env, "pynest_dict"):
        env.pynest_dict = {}

    env.pynest_dict = process_directory(directory)


def api_customizer(app, docname, source):
    env = app.builder.env
    if docname == "ref_material/pynest_api/index":
        get_apis = env.pynest_dict
        html_context = {"api_dict": get_apis}
        api_source = source[0]
        rendered = app.builder.templates.render_string(api_source, html_context)
        source[0] = rendered


def setup(app):
    app.connect("env-before-read-docs", get_pynest_list)
    app.connect("source-read", api_customizer)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
