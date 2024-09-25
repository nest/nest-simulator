# -*- coding: utf-8 -*-
#
# hl_api_server_utils.py
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
import importlib
import io
import os
import sys
import traceback

MODULES = os.environ.get("NEST_SERVER_MODULES", "import nest")


class Capturing(list):
    """Monitor stdout contents i.e. print."""

    def __enter__(self):
        self._stdout = sys.stdout
        sys.stdout = self._stringio = io.StringIO()
        return self

    def __exit__(self, *args):
        self.extend(self._stringio.getvalue().splitlines())
        del self._stringio  # free up some memory
        sys.stdout = self._stdout


class ErrorHandler(Exception):
    status_code = 400
    lineno = -1

    def __init__(self, message: str, lineno: int = None, status_code: int = None, payload=None):
        super().__init__()
        self.message = message
        if status_code is not None:
            self.status_code = status_code
        if lineno is not None:
            self.lineno = lineno
        self.payload = payload

    def to_dict(self):
        rv = dict(self.payload or ())
        rv["message"] = self.message
        if self.lineno != -1:
            rv["lineNumber"] = self.lineno
        return rv


def clean_code(source):
    codes = source.split("\n")
    codes_cleaned = []  # noqa
    for code in codes:
        if code.startswith("import") or code.startswith("from"):
            codes_cleaned.append("#" + code)
        else:
            codes_cleaned.append(code)
    return "\n".join(codes_cleaned)


def get_arguments(request):
    """Get arguments from the request."""
    args, kwargs = [], {}
    if request.is_json:
        json = request.get_json()
        if isinstance(json, str) and len(json) > 0:
            args = [json]
        elif isinstance(json, list):
            args = json
        elif isinstance(json, dict):
            kwargs = json
            if "args" in kwargs:
                args = kwargs.pop("args")
    elif len(request.form) > 0:
        if "args" in request.form:
            args = request.form.getlist("args")
        else:
            kwargs = request.form.to_dict()
    elif len(request.args) > 0:
        if "args" in request.args:
            args = request.args.getlist("args")
        else:
            kwargs = request.args.to_dict()
    return list(args), kwargs


def get_boolean_environ(env_key, default_value="false"):
    env_value = os.environ.get(env_key, default_value)
    return env_value.lower() in ["yes", "true", "t", "1"]


def get_lineno(err, tb_idx):
    lineno = -1
    if hasattr(err, "lineno") and err.lineno is not None:
        lineno = err.lineno
    else:
        tb = sys.exc_info()[2]
        # if hasattr(tb, "tb_lineno") and tb.tb_lineno is not None:
        #     lineno = tb.tb_lineno
        # else:
        lineno = traceback.extract_tb(tb)[tb_idx][1]
    return lineno


def get_modules_from_env():
    """Get modules from environment variable NEST_SERVER_MODULES.

    This function converts the content of the environment variable NEST_SERVER_MODULES:
    to a formatted dictionary for updating the Python `globals`.

    Here is an example:
        `NEST_SERVER_MODULES="import nest; import numpy as np; from numpy import random"`
    is converted to the following dictionary:
        `{'nest': <module 'nest'> 'np': <module 'numpy'>, 'random': <module 'numpy.random'>}`
    """
    modules = {}
    try:
        parsed = ast.iter_child_nodes(ast.parse(MODULES))
    except (SyntaxError, ValueError):
        raise SyntaxError("The NEST server module environment variables contains syntax errors.")
    for node in parsed:
        if isinstance(node, ast.Import):
            for alias in node.names:
                modules[alias.asname or alias.name] = importlib.import_module(alias.name)
        elif isinstance(node, ast.ImportFrom):
            for alias in node.names:
                modules[alias.asname or alias.name] = importlib.import_module(f"{node.module}.{alias.name}")
    return modules
