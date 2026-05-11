# -*- coding: utf-8 -*-
#
# hl_api_info.py
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

"""
Functions to get information on NEST.
"""

import inspect
import os
import textwrap
import webbrowser

import nest

from .. import nestkernel_api as nestkernel
from .hl_api_helper import (
    broadcast,
    deprecated,
    is_iterable,
    load_help,
    show_help_with_pager,
)
from .hl_api_types import to_json

__all__ = ["GetStatus", "help", "SetStatus", "VerbosityLevel", "message", "get_verbosity", "set_verbosity"]


VerbosityLevel = nestkernel.VerbosityLevel


def help(obj=None, return_text=False):
    """Display the help page for the given object in a pager.

    If ``return_text`` is omitted or explicitly given as ``False``,
    this command opens the help text for ``object`` in the default
    pager using the ``pydoc`` module.

    If ``return_text`` is ``True``, the help text is returned as a
    string in reStructuredText format instead of displaying it.

    Parameters
    ----------
    obj : object, optional
        Object to display help for
    return_text : bool, optional
        Option for returning the help text

    Returns
    -------
    None or str
        The help text of the object if `return_text` is `True`.

    """

    if obj is not None:
        try:
            if return_text:
                return load_help(obj)
            else:
                show_help_with_pager(obj)
        except FileNotFoundError:
            print(textwrap.dedent(f"""
                Sorry, there is no help for model '{obj}'.
                Use the Python help() function to obtain help on PyNEST functions."""))
    else:
        print(nest.__doc__)


@deprecated(
    "get",
    " ".join(
        (
            "GetStatus() is deprecated and will be removed in a future version of NEST.",
            "Instead of GetStatus(nrns|conns, args), use nrns|conns.get(args).",
            "For json output, compatibility of results with NEST 3.9 is not ensured.",
        )
    ),
)
def GetStatus(nodes_or_conns, keys=None, output=""):
    if len(nodes_or_conns) == 0:
        return "[]" if output == "json" else tuple()

    # Operations below ensure that output matches the NEST 3.9 output format
    if keys:
        result = nodes_or_conns.get(keys, output=output)

        if isinstance(keys, str):
            if len(nodes_or_conns) == 1:
                result = (result,)
        else:
            if len(nodes_or_conns) == 1:
                result = (tuple(result.values()),)
            else:
                result = tuple(zip(*result.values()))
    else:
        result = nodes_or_conns.get(output=output)
        if len(nodes_or_conns) == 1:
            result = (result,)
        else:
            result = tuple({k: v[j] for k, v in result.items()} for j in range(len(nodes_or_conns)))

    return result


@deprecated(
    "set",
    " ".join(
        (
            "SetStatus() is deprecated and will be removed in a future version of NEST.",
            "Instead of SetStatus(nrns|conns, args), use nrns|conns.set(args).",
        )
    ),
)
def SetStatus(nodes_or_conns, params, val=None):
    nodes_or_conns.set(params if val is None else {params: val})


def message(
    message,
    severity=nest.NestModule.ll_api.nestkernel.VerbosityLevel.INFO,
    *,
    function=None,
    filename=None,
    lineno=None,
):
    """
    Issue message via NEST logging mechanism.

    Calling function, the name of the source code file and the pertaining line number are added automatically if not
    explicitly given. These values are only displayed by the NEST logging mechanism if verbosity is DEBUG or ALL.

    When setting the severity of the issue, pass `severity=nest.VerbosityLevel.WARNING` and similar. Default is `INFO`.
    """

    frame = inspect.stack()[1]

    function = function or frame.function
    filename = filename or frame.filename
    lineno = lineno or frame.lineno

    nestkernel.llapi_message(severity, function, message, filename, lineno)


@deprecated("", "Provided for backward compatibility only. Look up `nest.verbosity` instead.")
def get_verbosity():
    """Return numeric value for NEST verbosity"""

    return int(nest.NestModule.ll_api.nestkernel.llapi_get_kernel_status()["verbosity"])


@deprecated("", "Provided for backward compatibility only. Set `nest.verbosity = nest.VerbosityLevel.XYZ` instead.")
def set_verbosity(level):
    """Change verbosity level for NEST's messages."""

    nest.NestModule.ll_api.nestkernel.llapi_set_kernel_status(
        {"verbosity": getattr(nestkernel.VerbosityLevel, level.split("_")[1])}
    )  # level must be of form "M_XYZ"
