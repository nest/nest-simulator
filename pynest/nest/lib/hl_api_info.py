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

from .hl_api_helper import *
import sys
import os
import webbrowser
from distutils.util import strtobool


@check_stack
def sysinfo():
    """Print information on the platform on which NEST was compiled."""

    sr("sysinfo")


@check_stack
def version():
    """Return the NEST version.

    Returns
    -------
    str:
        The version of NEST.
    """

    sr("statusdict [[ /kernelname /version ]] get")
    return " ".join(spp())


@check_stack
def authors():
    """Print the authors of NEST."""

    sr("authors")


@check_stack
def helpdesk():
    """Open the NEST helpdesk in browser.

    Use the system default browser.
    """
    if sys.version_info < (2, 7, 8):
        print("The NEST Helpdesk is only available with Python 2.7.8 or "
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

    The default pager is more.

    Parameters
    ----------
    obj : object, optional
        Object to display help for
    pager : str, optional
        Pager to use
    return_text : bool, optional
        Option for returning the help text
    """
    hlpobj = obj
    if hlpobj is not None:
        if isinstance(return_text, str):
            return_text = bool(strtobool(return_text))
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
        print("For more information visit http://www.nest-simulator.org.")


@check_stack
def get_argv():
    """Return argv as seen by NEST.

    This is similar to Python sys.argv but might have changed after
    MPI initialization.

    Returns
    -------
    tuple:
        Argv, as seen by NEST.
    """

    sr('statusdict')
    statusdict = spp()
    return statusdict['argv']


@check_stack
def message(level, sender, text):
    """Print a message using NEST's message system.

    Parameters
    ----------
    level :
        Level
    sender :
        Message sender
    text : str
        Text to be sent in the message
    """

    sps(level)
    sps(sender)
    sps(text)
    sr('message')


@check_stack
def SetStatus(nodes, params, val=None):
    """Set the parameters of nodes or connections to params.

    If val is given, params has to be the name
    of an attribute, which is set to val on the nodes/connections. val
    can be a single value or a list of the same size as nodes.

    Parameters
    ----------
    nodes : list or tuple
        Either a list of global ids of nodes, or a tuple of connection
        handles as returned by GetConnections()
    params : str or dict or list
        Dictionary of parameters or list of dictionaries of parameters of
        same length as nodes. If val is given, this has to be the name of
        a model property as a str.
    val : str, optional
        If given, params has to be the name of a model property.

    Raises
    ------
    TypeError
        Description
    """

    if not is_coercible_to_sli_array(nodes):
        raise TypeError("nodes must be a list of nodes or synapses")

    # This was added to ensure that the function is a nop (instead of,
    # for instance, raising an exception) when applied to an empty list,
    # which is an artifact of the API operating on lists, rather than
    # relying on language idioms, such as comprehensions
    #
    if len(nodes) == 0:
        return

    if val is not None and is_literal(params):
        if is_iterable(val) and not isinstance(val, (uni_str, dict)):
            params = [{params: x} for x in val]
        else:
            params = {params: val}

    params = broadcast(params, len(nodes), (dict,), "params")
    if len(nodes) != len(params):
        raise TypeError(
            "status dict must be a dict, or list of dicts of length 1 "
            "or len(nodes)")

    if is_sequence_of_connections(nodes):
        pcd(nodes)
    else:
        sps(nodes)

    sps(params)
    sr('2 arraystore')
    sr('Transpose { arrayload pop SetStatus } forall')


@check_stack
def GetStatus(nodes, keys=None):
    """Return the parameter dictionaries of nodes or connections.

    If keys is given, a list of values is returned instead. keys may also be a
    list, in which case the returned list contains lists of values.

    Parameters
    ----------
    nodes : list or tuple
        Either a list of global ids of nodes, or a tuple of connection
        handles as returned by GetConnections()
    keys : str or list, optional
        String or a list of strings naming model properties. GetDefaults then
        returns a single value or a list of values belonging to the keys
        given.

    Returns
    -------
    dict:
        All parameters
    type:
        If keys is a string, the corrsponding default parameter is returned
    list:
        If keys is a list of strings, a list of corrsponding default parameters
        is returned

    Raises
    ------
    TypeError
        Description
    """

    if not is_coercible_to_sli_array(nodes):
        raise TypeError("nodes must be a list of nodes or synapses")

    if len(nodes) == 0:
        return nodes

    if keys is None:
        cmd = '{ GetStatus } Map'
    elif is_literal(keys):
        cmd = '{{ GetStatus /{0} get }} Map'.format(keys)
    elif is_iterable(keys):
        keys_str = " ".join("/{0}".format(x) for x in keys)
        cmd = '{{ GetStatus }} Map {{ [ [ {0} ] ] get }} Map'.format(keys_str)
    else:
        raise TypeError("keys should be either a string or an iterable")

    if is_sequence_of_connections(nodes):
        pcd(nodes)
    else:
        sps(nodes)

    sr(cmd)

    return spp()
