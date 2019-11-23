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

import warnings

import nest
from ..ll_api import *
from .. import pynestkernel as kernel
from .hl_api_helper import *
from .hl_api_info import SetStatus
from .hl_api_types import NodeCollection, Parameter

__all__ = [
    'Create',
    'GetLocalNodeCollection',
    'GetNodes',
    'PrintNodes',
]


@check_stack
def Create(model, n=1, params=None, positions=None):
    """Create one or more nodes.

    Generates `n` new network objects of the supplied model type. If `n` is not
    given, a single node is created. Note that if setting parameters of the
    nodes fail, the nodes will still have been created.

    Parameters
    ----------
    model : str
        Name of the model to create
    n : int, optional
        Number of nodes to create
    params : dict, list or `Parameter`, optional
        Parameters for the new nodes. A single dictionary, a list of
        dictionaries with size n or a `Parameter`. If omitted, the model's defaults are used.
    positions: :py:class:`spatial.grid` or :py:class:`spatial.free` object, optional
        Object describing spatial posistions of the nodes. If omitted, the nodes have no spatial attatchment.

    Returns
    -------
    NodeCollection:
        Object representing global IDs of created nodes, see :py:class:`NodeCollection` for more.

    Raises
    ------
    NESTError
        If setting node parameters fail. However, the nodes will still have
        been created.
    """

    model_deprecation_warning(model)

    if positions is not None:
        if not isinstance(positions, (nest.spatial.free, nest.spatial.grid)):
            raise TypeError('`positions` must be either a nest.spatial.free object or nest.spatial.grid object')
        layer_specs = {'elements': model}
        layer_specs['edge_wrap'] = positions.edge_wrap
        if isinstance(positions, nest.spatial.free):
            layer_specs['positions'] = positions.pos
            if isinstance(positions.pos, Parameter):
                layer_specs['n'] = n
        else:
            if n > 1:
                raise kernel.NESTError(
                    'Cannot specify number of nodes with grid positions')
            layer_specs['shape'] = positions.shape
            if positions.center is not None:
                layer_specs['center'] = positions.center
        if positions.extent is not None:
            layer_specs['extent'] = positions.extent
        if params is None:
            params = {}
        layer = sli_func('CreateLayerParams', layer_specs, params)

        return layer

    params_contains_list = True
    if isinstance(params, dict):
        params_contains_list = [is_iterable(v) or isinstance(v, Parameter)
                                for k, v in params.items()]
        params_contains_list = max(params_contains_list)

    if not params_contains_list:
        cmd = "/%s 3 1 roll exch Create" % model
        sps(params)
    else:
        cmd = "/%s exch Create" % model

    sps(n)
    sr(cmd)

    node_ids = spp()

    if params is not None and params_contains_list:
        try:
            SetStatus(node_ids, params)
        except:
            warnings.warn(
                "SetStatus() call failed, but nodes have already been " +
                "created! The node IDs of the new nodes are: {0}.".format(node_ids))
            raise

    return node_ids


@check_stack
def PrintNodes():
    """Print the `node ID` ranges and model names of all the nodes in the network."""

    sr("PrintNodesToStream")
    print(spp())


def GetNodes(properties={}, local_only=False):
    """Return all nodes with the given properties as `NodeCollection`.

    Parameters
    ----------
    properties : dict, optional
        Only node IDs of nodes matching the properties given in the
        dictionary exactly will be returned. Matching properties with float
        values (e.g. the membrane potential) may fail due to tiny numerical
        discrepancies and should be avoided. Note that when a params dict is
        present, thread parallelization is not possible, the function will
        be run thread serial.
    local_only : bool, optional
        If True, only node IDs of nodes simulated on the local MPI process will
        be returned. By default, node IDs of nodes in the entire simulation
        will be returned. This requires MPI communication and may slow down
        the script.

    Returns
    -------
    NodeCollection:
        `NodeCollection` of nodes
    """

    return sli_func('GetNodes', properties, local_only)


@check_stack
def GetLocalNodeCollection(nc):
    """Get local nodes of a `NodeCollection` as a new `NodeCollection`.

    This function returns the local nodes of a `NodeCollection`. If there are no
    local elements, an empty `NodeCollection` is returned.

    Parameters:
    -----------
    nc: `NodeCollection`
        `NodeCollection` for which to get local nodes

    Returns
    -------
    NodeCollection:
        Object representing the local nodes of the given `NodeCollection`
    """
    if not isinstance(nc, NodeCollection):
        raise TypeError("GetLocalNodeCollection requires a NodeCollection in order to run")

    sps(nc)
    sr("LocalOnly")
    return spp()
