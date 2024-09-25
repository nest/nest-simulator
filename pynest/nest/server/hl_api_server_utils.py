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

import os
import sys
import traceback

from nest.lib.hl_api_exceptions import NESTError


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
