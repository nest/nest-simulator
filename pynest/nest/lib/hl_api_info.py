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

import sys
import os
import webbrowser

from ..ll_api import *
from .hl_api_helper import *
import nest

__all__ = [
    'authors',
    'get_argv',
    'GetStatus',
    'get_verbosity',
    'help',
    'helpdesk',
    'message',
    'SetStatus',
    'set_verbosity',
    'sysinfo',
    'version',
]


@check_stack
def sysinfo():
    """Print information on the platform on which NEST was compiled.

    KEYWORDS: info
    """

    sr("sysinfo")


@check_stack
def version():
    """Return the NEST version.

    Returns
    -------
    str
        The version of NEST

    KEYWORDS: info
    """

    sr("statusdict [[ /kernelname /version ]] get")
    return " ".join(spp())


@check_stack
def authors():
    """Print the authors of NEST.

    KEYWORDS: info
    """

    sr("authors")


@check_stack
def helpdesk():
    """Open the NEST helpdesk in browser.

    Use the system default browser.

    KEYWORDS: info
    """

    if sys.version_info < (2, 7, 8):
        print("The NEST helpdesk is only available with Python 2.7.8 or "
              "later. \n")
        return

    if 'NEST_DOC_DIR' not in os.environ:
        print(
            'NEST help needs to know where NEST is installed.'
            'Please source nest_vars.sh or define NEST_DOC_DIR manually.')
        return

    helpfile = os.path.join(os.environ['NEST_DOC_DIR'], 'help',
                            'helpindex.html')

    # Under Windows systems webbrowser.open is incomplete
    # See <https://bugs.python.org/issue8232>
    if sys.platform[:3] == "win":
        os.startfile(helpfile)

    # Under MacOs we need to ask for the browser explicitly.
    # See <https://bugs.python.org/issue30392>.
    if sys.platform[:3] == "dar":
        webbrowser.get('safari').open_new(helpfile)
    else:
        webbrowser.open_new(helpfile)


@check_stack
def help(obj=None, pager=None, return_text=False):
    """Show the help page for the given object using the given pager.

    The default pager is `more` (See `.nestrc`).

    Parameters
    ----------
    obj : object, optional
        Object to display help for
    pager : str, optional
        Pager to use
    return_text : bool, optional
        Option for returning the help text

    Returns
    -------
    None or str
        The help text of the object if `return_text` is ``True``.

    KEYWORDS: info
    """
    hlpobj = obj
    if hlpobj is not None:
        if return_text:
            return load_help(hlpobj)
        else:
            show_help_with_pager(hlpobj, pager)

    else:
        print("Type 'nest.helpdesk()' to access the online documentation "
              "in a browser.")
        print("Type 'nest.help(object)' to get help on a NEST object or "
              "command.\n")
        print("Type 'nest.Models()' to see a list of available models "
              "in NEST.")
        print("Type 'nest.authors()' for information about the makers "
              "of NEST.")
        print("Type 'nest.sysinfo()' to see details on the system "
              "configuration.")
        print("Type 'nest.version()' for information about the NEST "
              "version.\n")
        print("For more information visit https://www.nest-simulator.org.")


@check_stack
def get_argv():
    """Return argv as seen by NEST.

    This is similar to Python :code:`sys.argv` but might have changed after
    MPI initialization.

    Returns
    -------
    tuple
        Argv, as seen by NEST

    KEYWORDS: info
    """

    sr('statusdict')
    statusdict = spp()
    return statusdict['argv']


@check_stack
def message(level, sender, text):
    """Print a message using message system of NEST.

    Parameters
    ----------
    level :
        Level
    sender :
        Message sender
    text : str
        Text to be sent in the message

    KEYWORDS: info
    """

    sps(level)
    sps(sender)
    sps(text)
    sr('message')


@check_stack
def get_verbosity():
    """Return verbosity level of NEST's messages.

    M_ALL=0,  display all messages
    M_INFO=10, display information messages and above
    M_DEPRECATED=18, display deprecation warnings and above
    M_WARNING=20, display warning messages and above
    M_ERROR=30, display error messages and above
    M_FATAL=40, display failure messages and above

    Returns
    -------
    int:
        The current verbosity level
    """

    sr('verbosity')
    return spp()


@check_stack
def set_verbosity(level):
    """Change verbosity level for NEST's messages.

    M_ALL=0,  display all messages
    M_INFO=10, display information messages and above
    M_DEPRECATED=18, display deprecation warnings and above
    M_WARNING=20, display warning messages and above
    M_ERROR=30, display error messages and above
    M_FATAL=40, display failure messages and above

    Parameters
    ----------
    level : str
        Can be one of 'M_FATAL', 'M_ERROR', 'M_WARNING', 'M_DEPRECATED',
        'M_INFO' or 'M_ALL'.
    """

    sr("{} setverbosity".format(level))


@check_stack
def SetStatus(nodes, params, val=None):
    """Set parameters of nodes or connections.

    Parameters of nodes or connections, given in `nodes`, is set as specified
    by `params`. If `val` is given, `params` has to be a string with the
    name of an attribute, which is set to `val` on the nodes/connections. `val`
    can be a single value or a list of the same size as nodes.

    Parameters
    ----------
    nodes : GIDCollection or tuple
        Either a `GIDCollection` representing nodes, or a `Connectome`
        of connection handles as returned by
        :py:func:`.GetConnections()`.
    params : str or dict or list
        Dictionary of parameters or list of dictionaries of parameters
        of same length as `nodes`. If `val` is given, this has to be
        the name of a model property as a str.
    val : int, list, optional
        If given, params has to be the name of a model property.

    Raises
    ------
    TypeError
        If `nodes` is not a list of nodes or synapses, or if the
        number of parameters don't match the number of nodes or
        synapses.

    See Also
    -------
    :py:func:`.GetStatus`

    """

    if not isinstance(nodes, (nest.GIDCollection, nest.Connectome)):
        raise TypeError("'nodes' must be GIDCollection or a Connectome.")

    # This was added to ensure that the function is a nop (instead of,
    # for instance, raising an exception) when applied to an empty
    # list, which is an artifact of the API operating on lists, rather
    # than relying on language idioms, such as comprehensions
    if len(nodes) == 0:
        return

    n0 = nodes[0]
    params_is_dict = isinstance(params, dict)
    set_status_nodes = isinstance(nodes, nest.GIDCollection)
    set_status_local_nodes = set_status_nodes and n0.get('local')

    if (params_is_dict and set_status_local_nodes):
        contains_list = [is_iterable(vals) and not is_iterable(n0.get(key))
                         for key, vals in params.items()]

        if any(contains_list):
            temp_param = [{} for _ in range(len(nodes))]

            for key, vals in params.items():
                if not is_iterable(vals):
                    for temp_dict in temp_param:
                        temp_dict[key] = vals
                else:
                    for i, temp_dict in enumerate(temp_param):
                        temp_dict[key] = vals[i]
            params = temp_param

    if val is not None and is_literal(params):
        if is_iterable(val) and not isinstance(val, (uni_str, dict)):
            params = [{params: x} for x in val]
        else:
            params = {params: val}

    if isinstance(params, (list, tuple)) and len(nodes) != len(params):
        raise TypeError(
            "status dict must be a dict, or a list of dicts of length "
            "len(nodes)")

    if isinstance(nodes, nest.Connectome):
        params = broadcast(params, len(nodes), (dict,), "params")

        sps(nodes)
        sps(params)

        sr('2 arraystore')
        sr('Transpose { arrayload pop SetStatus } forall')
    else:
        sli_func('SetStatus', nodes, params)


@check_stack
def GetStatus(nodes, keys=None, output=''):
    """Return the parameter dictionaries of nodes or connections.

    If `keys` is given, a list of values is returned instead. `keys` may
    also be a list, in which case the returned list contains lists of
    values.

    Parameters
    ----------
    nodes : GIDCollection or tuple
        Either a `GIDCollection` representing nodes, or a `Connectome` of
        connection handles as returned by :py:func:`.GetConnections()`.
    keys : str or list, optional
        string or a list of strings naming model properties.
        `GetStatus` then returns a single value or a list of values
        belonging to the keys given.
    output : str, optional
        Whether the returned data should be in a selected format
        (``output='json'``).

    Returns
    -------
    dict :
        All parameters
    type :
        If `keys` is a string, the corrsponding default parameter is returned.
    list :
        If keys is a list of strings, a list of corrsponding default parameters is returned.
    str :
        If `output` is `json`, parameters is returned in JSON format.

    Raises
    ------
    TypeError
        If `nodes` or `keys` are on the wrong form.

    See Also
    -------
    :py:func:`.SetStatus`
    """

    if not (isinstance(nodes, nest.GIDCollection) or isinstance(nodes, nest.Connectome)):
        raise TypeError("The first input (nodes) must be GIDCollection or a Connectome with connection handles ")

    if len(nodes) == 0:
        return nodes

    if keys is None:
        cmd = 'GetStatus'
    elif is_literal(keys):
        cmd = 'GetStatus {{ /{0} get }} Map'.format(keys)
    elif is_iterable(keys):
        keys_str = " ".join("/{0}".format(x) for x in keys)
        cmd = 'GetStatus {{ [ [ {0} ] ] get }} Map'.format(keys_str)
    else:
        raise TypeError("keys should be either a string or an iterable")

    sps(nodes)

    sr(cmd)

    result = spp()

    if isinstance(result, dict):
        # We have taken GetStatus on a layer object, or another GIDCollection with metadata, which returns a
        # dictionary from C++, so we need to turn it into a tuple for consistency.
        result = (result,)

    if output == 'json':
        result = to_json(result)

    return result
