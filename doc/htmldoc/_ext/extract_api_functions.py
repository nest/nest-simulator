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
import re
from pathlib import Path

from sphinx.application import Sphinx

"""
Generate a JSON dictionary that stores the module name as key and corresponding
functions as values, along with the ``NestModule`` and the kernel attributes.
Used in a Jinja template to generate the autosummary for each module in
the API documentation (``ref_material/pynest_api/``)
"""


def find_all_variables(file_path, is_package_root=False):
    """
    Get the names of all functions listed in ``__all__`` in each of the PyNEST
    API files, along with the kernel attributes found in ``__init__.py`` of
    ``pynest/nest/``.
    """
    all_variables = None

    file_content = file_path.read_text(encoding="utf-8")

    if is_package_root:
        # Find the class definition
        match = re.search(r"class\s+NestModule\(.*?\):", file_content, re.DOTALL)
        if match:
            # Find the variable assignments within the class
            all_variables = re.findall(r"(\w+)\s*=\s*KernelAttribute", file_content)

    try:
        tree = ast.parse(file_content)
    except SyntaxError:
        # Skip files with syntax errors
        return None

    for node in ast.iter_child_nodes(tree):
        if isinstance(node, ast.Assign) and len(node.targets) == 1:
            target = node.targets[0]
            if isinstance(target, ast.Name) and target.id == "__all__":
                value = node.value
                if isinstance(value, ast.List):
                    all_variables = [
                        elem.value
                        for elem in value.elts
                        if isinstance(elem, ast.Constant) and isinstance(elem.value, str)
                    ]
                break

    return all_variables


def process_directory(package_dir):
    """
    Get the PyNEST API filenames and set the keys to the base name
    """
    api_dict = {}
    api_exception_list = ["raster_plot", "visualization", "voltage_trace"]

    api_name = None
    for file_path in sorted(package_dir.rglob("*.py")):
        # Module location relative to ``pynest/nest/``, e.g. ``lib/hl_api_nodes.py``.
        # Matching on this instead of on the absolute path keeps the result
        # independent of where the repository happens to be checked out.
        relative = file_path.relative_to(package_dir)

        # ignoring the low level api and connection_helpers and helper modules
        if "helper" in relative.name or "ll_api" in relative.name:
            continue

        # get the NestModule for the kernel attributes
        is_package_root = relative == Path("__init__.py")
        if is_package_root:
            api_name = "nest.NestModule"

        module_name = relative.stem
        # only get high level API modules
        if "hl_" in relative.name:
            module_path = ".".join(relative.parent.parts)
            api_name = f"nest.{module_path}.{module_name}"
        for item in api_exception_list:
            if item in relative.name:
                api_name = f"nest.{module_name}"

        all_variables = find_all_variables(file_path, is_package_root)
        if all_variables and api_name:
            api_dict[api_name] = all_variables

    return api_dict


def get_pynest_list(app, env, docname):
    # ``app.srcdir`` is ``<repo>/doc/htmldoc``, so its second parent is the repo root.
    package_dir = app.srcdir.parents[1] / "pynest" / "nest"

    if not hasattr(env, "pynest_dict"):
        env.pynest_dict = {}

    env.pynest_dict = process_directory(package_dir)


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
