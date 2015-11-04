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
