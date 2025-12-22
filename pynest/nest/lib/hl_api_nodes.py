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

import nest
import numpy as np

from .. import nestkernel_api as nestkernel
from .hl_api_helper import is_iterable, model_deprecation_warning
from .hl_api_parallel_computing import NumProcesses, Rank
from .hl_api_types import NodeCollection, Parameter

__all__ = [
    "Create",
    "GetLocalNodeCollection",
    "GetNodes",
    "PrintNodes",
]


def Create(model, n=1, params=None, positions=None):
    """Create one or more nodes.

    Generates `n` new network objects of the supplied model type. If `n` is not
    given, a single node is created. Note that if setting parameters of the
    nodes fail, the nodes will still have been created.

    Note
    ----
    If `Create()` is called with two arguments and the second argument (`n`) is a dictionary,
    this dictionary will be intepreted as `params` for backward compatibility.

    During network construction, create all nodes representing model neurons first, then all nodes
    representing devices (generators, recorders, or detectors), or all devices first and then all neurons.
    Otherwise, network connection can be slow, especially in parallel simulations of networks
    with many devices.

    Parameters
    ----------
    model : str
        Name of the model to create
    n : int, optional
        Number of nodes to create
    params : dict or list, optional
        Parameters for the new nodes. Can be any of the following:

        - A dictionary with either single values or lists of size n.
          The single values will be applied to all nodes, while the lists will be distributed across
          the nodes. Both single values and lists can be given at the same time.
        - A list with n dictionaries, one dictionary for each node.

        Values may be :py:class:`.Parameter` objects. If omitted,
        the model's defaults are used.
    positions: :py:class:`.grid` or :py:class:`.free` object, optional
        Object describing spatial positions of the nodes. If omitted, the nodes have no spatial attachment.

    Returns
    -------
    NodeCollection:
        Object representing the IDs of created nodes, see :py:class:`.NodeCollection` for more.

    Raises
    ------
    NESTError
        If setting node parameters fail. However, the nodes will still have
        been created.
    TypeError
        If the positions object is of wrong type.
    """

    model_deprecation_warning(model)

    if isinstance(n, dict):
        if not (params is None and positions is None):
            raise ValueError(
                "A parameter dictionary can be passed as second argument only of Create() is called with two arguments."
            )
        params = n
        n = 1

    if int(n) != n:
        raise TypeError("n must have an integer value")
    n = int(n)

    if isinstance(params, (list, tuple)) and len(params) != n:
        raise TypeError("list of params must have one dictionary per node")

    if params is not None and not (
        isinstance(params, dict) or (isinstance(params, (list, tuple)) and all(isinstance(e, dict) for e in params))
    ):
        raise TypeError("params must be either a dict of parameters or a list or tuple of dicts")

    if positions is not None:
        # Explicitly retrieve lazy loaded spatial property from the module class.
        # This is needed because the automatic lookup fails. See #2135.
        spatial = getattr(nest.NestModule, "spatial")
        # We only accept positions as either a free object or a grid object.
        if not isinstance(positions, (spatial.free, spatial.grid)):
            raise TypeError("`positions` must be either a nest.spatial.free or a nest.spatial.grid object")
        layer_specs = {"elements": model}
        layer_specs["edge_wrap"] = positions.edge_wrap
        if isinstance(positions, spatial.free):
            layer_specs["positions"] = positions.pos
            # If the positions are based on a parameter object, the number of nodes must be specified.
            if isinstance(positions.pos, Parameter):
                layer_specs["n"] = n
        else:
            # If positions is not a free object, it must be a grid object.
            if n > 1:
                raise ValueError("Cannot specify number of nodes with grid positions")
            layer_specs["shape"] = positions.shape
            if positions.center is not None:
                layer_specs["center"] = [float(v) for v in positions.center]
        if positions.extent is not None:
            layer_specs["extent"] = [float(v) for v in positions.extent]

        layer = nestkernel.llapi_create_spatial(layer_specs)
        layer.set(params if params else {})
        return layer

    node_ids = nestkernel.llapi_create(model, n)

    if (isinstance(params, dict) and params) or isinstance(params, (list, tuple)):
        # if params is a non-empty dict or a list of dicts
        node_ids.set(params)

    return node_ids


def PrintNodes():
    """Print the `node ID` ranges and `model names` of all the nodes in the network."""

    print(nestkernel.llapi_print_nodes())


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

    return nestkernel.llapi_get_nodes(properties, local_only)


def GetLocalNodeCollection(nc):
    """Get local nodes of a `NodeCollection` as a new `NodeCollection`.

    This function returns the local nodes of a `NodeCollection`. If there are no
    local elements, an empty `NodeCollection` is returned.

    Parameters
    ----------
    nc: NodeCollection
        `NodeCollection` for which to get local nodes

    Returns
    -------
    NodeCollection:
        Object representing the local nodes of the given `NodeCollection`
    """
    if not isinstance(nc, NodeCollection):
        raise TypeError("GetLocalNodeCollection requires a NodeCollection in order to run")

    return nc[nc.local]
