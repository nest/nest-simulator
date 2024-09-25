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

import inspect
import os
import sys
import time
import traceback

import nest
import RestrictedPython
from nest.lib.hl_api_exceptions import NESTError

from .hl_api_server_utils import (
    Capturing,
    ErrorHandler,
    clean_code,
    get_boolean_environ,
    get_lineno,
    get_modules_from_env,
)

_default_origins = "http://localhost:*,http://127.0.0.1:*"
ACCESS_TOKEN = os.environ.get("NEST_SERVER_ACCESS_TOKEN", "")
AUTH_DISABLED = get_boolean_environ("NEST_SERVER_DISABLE_AUTH")
CORS_ORIGINS = os.environ.get("NEST_SERVER_CORS_ORIGINS", _default_origins).split(",")
EXEC_CALL_ENABLED = get_boolean_environ("NEST_SERVER_ENABLE_EXEC_CALL")
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


def get_or_error(func):
    """Wrapper to exec function."""

    def func_wrapper(*args, **kwargs):
        try:
            return func(*args, **kwargs)

        except NESTError as err:
            error_class = err.errorname + " (NESTError)"
            detail = err.errormessage
            lineno = get_lineno(err, 1)

        except (KeyError, SyntaxError, TypeError, ValueError) as err:
            error_class = err.__class__.__name__
            detail = err.args[0]
            lineno = get_lineno(err, 1)

        except Exception as err:
            error_class = err.__class__.__name__
            detail = err.args[0]
            lineno = get_lineno(err, -1)

        for line in traceback.format_exception(*sys.exc_info()):
            print(line, flush=True)

        if lineno == -1:
            message = "%s: %s" % (error_class, detail)
        else:
            message = "%s at line %d: %s" % (error_class, lineno, detail)
        raise ErrorHandler(message, lineno)

    return func_wrapper


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
