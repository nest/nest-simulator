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
from .hl_api_info import SetStatus


@check_stack
def Create(model, n=1, params=None):
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
    list:
        Global IDs of created nodes

    Raises
    ------
    NESTError
        If setting node parameters fail. However, the nodes will still have
        been created.

    KEYWORDS:
    """

    model_deprecation_warning(model)

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
            warnings.warn(
                "SetStatus() call failed, but nodes have already been " +
                "created! The GIDs of the new nodes are: {0}.".format(gids))
            raise

    return gids


@check_stack
@deprecated('', 'GetLID is deprecated and will be removed in NEST 3.0. Use \
index into GIDCollection instead.')
def GetLID(gid):
    """Return the local id of a node with the global ID gid.

    .. deprecated:: 2.14
    `GetLID` is deprecated and will be removed in NEST 3.0. Use
    index into `GIDCollection` instead.

    Parameters
    ----------
    gid : int
        Global id of node

    Returns
    -------
    int:
        Local id of node

    Raises
    ------
    NESTError
        If `gid` contains more than one GID

    KEYWORDS:
    """

    if len(gid) > 1:
        raise NESTError("GetLID() expects exactly one GID.")

    sps(gid[0])
    sr("GetLID")

    return spp()
