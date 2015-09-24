# -*- coding: utf-8 -*-
#
# hl_api_nodes.py
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
Functions for node handling
"""

from .hl_api_helper import *

@check_stack
def Create(model, n=1, params=None):
    """
    Create n instances of type model. Parameters for the new nodes can
    are given as params (a single dictionary or a list of dictionaries
    with size n). If omitted, the model's defaults are used.
    """

    if isinstance(params, dict):
        cmd = "/%s 3 1 roll exch Create" % model
        sps(params)
    else:
        cmd = "/%s exch Create" % model

    sps(n)
    sr(cmd)

    last_gid = spp()
    gids = tuple(range(last_gid - n + 1, last_gid + 1))

    if params is not None and not isinstance(params, dict):
        try:
            SetStatus(gids, params)
        except:
            warnings.warn("SetStatus() call failed, but nodes have already been created! "
                          "The GIDs of the new nodes are: {0}.".format(gids))
            raise

    return gids


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


@check_stack
def GetLID(gid) :
    """
    Return the local id of a node with gid.
    GetLID(gid) -> lid
    """

    if len(gid) > 1:
        raise NESTError("GetLID() expects exactly one GID.")

    sps(gid[0])
    sr("GetLID")

    return spp()
