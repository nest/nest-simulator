# -*- coding: utf-8 -*-
#
# hl_api_server_helpers.py
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
import inspect
import io
import os
import sys
import time

import nest
import RestrictedPython

from .hl_api_server_utils import get_boolean_environ, get_or_error

_default_origins = "http://localhost:*,http://127.0.0.1:*"
ACCESS_TOKEN = os.environ.get("NEST_SERVER_ACCESS_TOKEN", "")
AUTH_DISABLED = get_boolean_environ("NEST_SERVER_DISABLE_AUTH")
CORS_ORIGINS = os.environ.get("NEST_SERVER_CORS_ORIGINS", _default_origins).split(",")
EXEC_CALL_ENABLED = get_boolean_environ("NEST_SERVER_ENABLE_EXEC_CALL")
RESTRICTION_DISABLED = get_boolean_environ("NEST_SERVER_DISABLE_RESTRICTION")
MODULES = os.environ.get("NEST_SERVER_MODULES", "import nest")
RESTRICTION_DISABLED = get_boolean_environ("NEST_SERVER_DISABLE_RESTRICTION")

__all__ = [
    "nestify",
]

nest_calls = dir(nest)
nest_calls = list(filter(lambda x: not x.startswith("_"), nest_calls))
nest_calls.sort()


def _check_security():
    """
    Checks the security level of the NEST Server instance.
    """

    msg = []
    if AUTH_DISABLED:
        msg.append("AUTH:\tThe authorization settings are disabled.")
    if "*" in CORS_ORIGINS:
        msg.append("CORS:\tThe allowed origins are not restricted.")
    if EXEC_CALL_ENABLED:
        msg.append("EXEC CALL:\tThe exec route is enabled and scripts can be executed.")
        if RESTRICTION_DISABLED:
            msg.append("RESTRICTION: The execution of scripts is not protected by RestrictedPython.")

    if len(msg) > 0:
        print(
            "WARNING: You chose to disable important access restrictions!\n"
            " This allows other computers to execute code on this machine as the current user!\n"
            " Be sure you understand the implications of these settings and take"
            " appropriate measures to protect your runtime environment!"
        )
        print("\n - ".join([" "] + msg) + "\n")


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


def clean_code(source):
    codes = source.split("\n")
    codes_cleaned = []  # noqa
    for code in codes:
        if code.startswith("import") or code.startswith("from"):
            codes_cleaned.append("#" + code)
        else:
            codes_cleaned.append(code)
    return "\n".join(codes_cleaned)


def do_exec(kwargs):
    source_code = kwargs.get("source", "")
    source_cleaned = clean_code(source_code)

    locals_ = dict()
    response = dict()
    if RESTRICTION_DISABLED:
        with Capturing() as stdout:
            globals_ = globals().copy()
            globals_.update(get_modules_from_env())
            get_or_error(exec)(source_cleaned, globals_, locals_)
        if len(stdout) > 0:
            response["stdout"] = "\n".join(stdout)
    else:
        code = RestrictedPython.compile_restricted(source_cleaned, "<inline>", "exec")  # noqa
        globals_ = get_restricted_globals()
        globals_.update(get_modules_from_env())
        get_or_error(exec)(code, globals_, locals_)
        if "_print" in locals_:
            response["stdout"] = "".join(locals_["_print"].txt)

    if "return" in kwargs:
        if isinstance(kwargs["return"], list):
            data = dict()
            for variable in kwargs["return"]:
                data[variable] = locals_.get(variable, None)
        else:
            data = locals_.get(kwargs["return"], None)
        response["data"] = get_or_error(nest.serialize_data)(data)
    return response


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


def get_restricted_globals():
    """Get restricted globals for exec function."""

    def getitem(obj, index):
        typelist = (list, tuple, dict, nest.NodeCollection)
        if obj is not None and type(obj) in typelist:
            return obj[index]
        msg = f"Error getting restricted globals: unidentified object '{obj}'."
        raise TypeError(msg)

    restricted_builtins = RestrictedPython.safe_builtins.copy()
    restricted_builtins.update(RestrictedPython.limited_builtins)
    restricted_builtins.update(RestrictedPython.utility_builtins)
    restricted_builtins.update(
        dict(
            max=max,
            min=min,
            sum=sum,
            time=time,
        )
    )

    restricted_globals = dict(
        __builtins__=restricted_builtins,
        _print_=RestrictedPython.PrintCollector,
        _getattr_=RestrictedPython.Guards.safer_getattr,
        _getitem_=getitem,
        _getiter_=iter,
        _unpack_sequence_=RestrictedPython.Guards.guarded_unpack_sequence,
        _write_=RestrictedPython.Guards.full_write_guard,
    )

    return restricted_globals


def nestify(call_name, args, kwargs):
    """Get the NEST API call and convert arguments if necessary."""

    call = getattr(nest, call_name)
    objectnames = ["nodes", "source", "target", "pre", "post"]
    paramKeys = list(inspect.signature(call).parameters.keys())
    args = [nest.NodeCollection(arg) if paramKeys[idx] in objectnames else arg for (idx, arg) in enumerate(args)]
    for key, value in kwargs.items():
        if key in objectnames:
            kwargs[key] = nest.NodeCollection(value)

    return call, args, kwargs
