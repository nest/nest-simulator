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

from ..ll_api import *
from .. import pynestkernel as kernel
from .hl_api_helper import *
from .hl_api_info import SetStatus
from .hl_api_types import Parameter

__all__ = [
    'Create',
    'PrintNodes',
]


@check_stack
def Create(model, n=1, params=None, positions=None, edge_wrap=None):
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
    params : dict or list, optional
        Parameters for the new nodes. A single dictionary or a list of
        dictionaries with size n. If omitted, the model's defaults are used.

    Returns
    -------
    GIDCollection:
        Object representing global IDs of created nodes

    Raises
    ------
    NESTError
        If setting node parameters fail. However, the nodes will still have
        been created.

    KEYWORDS:
    """

    model_deprecation_warning(model)

    if positions is not None:
        layer_specs = {'elements': model}
        if edge_wrap is not None:
            layer_specs['edge_wrap'] = edge_wrap
        if isinstance(positions, Parameter):
            layer_specs['n'] = n
        if isinstance(positions, (Parameter, list, tuple)):
            layer_specs['positions'] = positions
        else:
            if n > 1:
                raise kernel.NESTError(
                    'Cannot specify number of nodes with grid positions')
            layer_specs['rows'] = positions.rows
            layer_specs['columns'] = positions.columns
            if positions.extent is not None:
                layer_specs['extent'] = positions.extent
            if positions.center is not None:
                layer_specs['center'] = positions.center
            if positions.depth is not None:
                layer_specs['layers'] = positions.depth
        if params is None:
            params = {}
        layer = sli_func('CreateLayer', layer_specs, params)
        layer.set_spatial()

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

    gids = spp()

    if params is not None and params_contains_list:
        try:
            SetStatus(gids, params)
        except:
            warnings.warn(
                "SetStatus() call failed, but nodes have already been " +
                "created! The GIDs of the new nodes are: {0}.".format(gids))
            raise

    return gids


@check_stack
def PrintNodes():
    """Print the GID ranges and model names of the nodes in the network."""

    sr("PrintNodesToStream")
    print(spp())
