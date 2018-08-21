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
from .hl_api_helper import *
from .hl_api_info import SetStatus


@check_stack
def Create(model, n=1, params=None):
    """Create n instances of type model.

    Parameters
    ----------
    model : str
        Name of the model to create
    n : int, optional
        Number of instances to create
    params : TYPE, optional
        Parameters for the new nodes. A single dictionary or a list of
        dictionaries with size n. If omitted, the model's defaults are used.

    Returns
    -------
    GIDCollection:
        Object representing global IDs of created nodes
    """

    model_deprecation_warning(model)

    params_contains_list = True
    if isinstance(params, dict):
        # Convert Parameter to list
        for key in params.iterkeys():
            if isinstance(params[key], nest.Parameter):
                params[key] = [params[key].GetValue([0., 0.])
                               for _ in range(n)]
        params_contains_list = [is_iterable(v) for k, v in params.items()]
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
