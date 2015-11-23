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

@check_stack
def sysinfo():
    """
    Print information on the platform on which NEST was compiled.
    """

    sr("sysinfo")


@check_stack
def version():
    """
    Return the NEST version.
    """

    sr("statusdict [[ /kernelname /version ]] get")
    return " ".join(spp())
    

@check_stack
def authors():
    """
    Print the authors of NEST.
    """

    sr("authors")


@check_stack
def helpdesk(browser="firefox"):
    """
    Open the NEST helpdesk in the given browser. The default browser is firefox.
    """
    
    sr("/helpdesk << /command (%s) >> SetOptions" % browser)
    sr("helpdesk")


@check_stack
def help(obj=None, pager="less"):
    """
    Show the help page for the given object using the given pager. The
    default pager is less.
    """

    if obj is not None:
        sr("/page << /command (%s) >> SetOptions" % pager)
        sr("/%s help" % obj)
    else:
        print("Type 'nest.helpdesk()' to access the online documentation in a browser.")
        print("Type 'nest.help(object)' to get help on a NEST object or command.")
        print()
        print("Type 'nest.Models()' to see a list of available models in NEST.")
        print()
        print("Type 'nest.authors()' for information about the makers of NEST.")
        print("Type 'nest.sysinfo()' to see details on the system configuration.")
        print("Type 'nest.version()' for information about the NEST version.")
        print()
        print("For more information visit http://www.nest-simulator.org.")


@check_stack
def get_verbosity():
    """
    Return verbosity level of NEST's messages.
    """
    
    sr('verbosity')
    return spp()


@check_stack
def set_verbosity(level):
    """
    Change verbosity level for NEST's messages. level is a string and
    can be one of M_FATAL, M_ERROR, M_WARNING, or M_INFO.
    """

    sr("%s setverbosity" % level)


@check_stack
def get_argv():
    """
    Return argv as seen by NEST. This is similar to Python sys.argv
    but might have changed after MPI initialization.
    """
    sr ('statusdict')
    statusdict = spp ()
    return statusdict['argv']


@check_stack
def message(level,sender,text):
    """
    Print a message using NEST's message system.
    """

    sps(level)
    sps(sender)
    sps(text)
    sr('message')


@check_stack
def SetStatus(nodes, params, val=None):
    """
    Set the parameters of nodes (identified by global ids) or
    connections (identified by handles as returned by
    GetConnections()) to params, which may be a single dictionary or a
    list of dictionaries. If val is given, params has to be the name
    of an attribute, which is set to val on the nodes/connections. val
    can be a single value or a list of the same size as nodes.
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
        raise TypeError("status dict must be a dict, or list of dicts of length 1 or len(nodes)")

    if is_sequence_of_connections(nodes):
        pcd(nodes)
    else:
        sps(nodes)

    sps(params)
    sr('2 arraystore')
    sr('Transpose { arrayload pop SetStatus } forall')


@check_stack
def GetStatus(nodes, keys=None):
    """
    Return the parameter dictionaries of the given list of nodes
    (identified by global ids) or connections (identified
    by handles as returned by GetConnections()). If keys is given, a
    list of values is returned instead. keys may also be a list, in
    which case the returned list contains lists of values.
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
