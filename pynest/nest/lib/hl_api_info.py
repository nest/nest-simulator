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

__all__ = [
    "GetStatus",
    "get_verbosity",
    "help",
    "helpdesk",
    "SetStatus",
    "set_verbosity",
    "verbosity",
]

verbosity = nestkernel.severity_t


def helpdesk():
    """Open the NEST documentation index in a browser.

    This command opens the NEST documentation index page using the
    system's default browser.

    Please note that the help pages will only be available if you ran
    ``make html`` prior to installing NEST. For more details, see
    :ref:`doc_workflow`.

    """

    docdir = nestkernel.ll_api_get_kernel_status()["docdir"]
    help_fname = os.path.join(docdir, "html", "index.html")

    if not os.path.isfile(help_fname):
        msg = "Sorry, the help index cannot be opened. "
        msg += "Did you run 'make html' before running 'make install'?"
        raise FileNotFoundError(msg)

    webbrowser.open_new(f"file://{help_fname}")


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
            print(
                textwrap.dedent(
                    f"""
                Sorry, there is no help for model '{obj}'.
                Use the Python help() function to obtain help on PyNEST functions."""
                )
            )
    else:
        print(nest.__doc__)


def get_verbosity():
    """Return verbosity level of NEST's messages.

    - M_ALL=0,  display all messages
    - M_INFO=10, display information messages and above
    - M_DEPRECATED=18, display deprecation warnings and above
    - M_WARNING=20, display warning messages and above
    - M_ERROR=30, display error messages and above
    - M_FATAL=40, display failure messages and above

    Returns
    -------
    severity_t:
        The current verbosity level
    """

    return nestkernel.llapi_get_verbosity()


def set_verbosity(level):
    """Change verbosity level for NEST's messages.

    - M_ALL=0,  display all messages
    - M_INFO=10, display information messages and above
    - M_DEPRECATED=18, display deprecation warnings and above
    - M_WARNING=20, display warning messages and above
    - M_ERROR=30, display error messages and above
    - M_FATAL=40, display failure messages and above

    .. note::

       To suppress the usual output when NEST starts up (e.g., the welcome message and
       version information), you can run ``export PYNEST_QUIET=1`` on the command
       line before executing your simulation script.

    Parameters
    ----------
    level : severity_t, default: 'M_ALL'
        Can be one of the values of the nest.verbosity enum.
    """

    if type(level) is not verbosity:
        raise TypeError('"level" must be a value of the nest.verbosity enum.')

    nestkernel.llapi_set_verbosity(level)


@deprecated("get", "Instead of GetStatus(nrns|conns, args), use nrns|conns.get(args).")
def GetStatus(nodes_or_conns, keys=None, output=""):
    if keys:
        return nodes_or_conns.get(keys, output=output)
    else:
        return nodes_or_conns.get(output=output)


@deprecated("set", "Instead of SetStatus(nrns|conns, args), use nrns|conns.set(args).")
def SetStatus(nodes_or_conns, params, val=None):
    nodes_or_conns.set(params if val is None else {params: val})
