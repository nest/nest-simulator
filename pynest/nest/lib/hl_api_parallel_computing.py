# -*- coding: utf-8 -*-
#
# hl_api_parallel_computing.py
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
Functions for parallel computing
"""

from ..ll_api import *
from .. import pynestkernel as kernel
from .hl_api_helper import *

__all__ = [
    'NumProcesses',
    'Rank',
    'SetAcceptableLatency',
    'SetMaxBuffered',
    'SyncProcesses',
]


@check_stack
def Rank():
    """Return the MPI rank of the local process.

    Returns
    -------
    int:
        MPI rank of the local process

    Note
    ----
    DO NOT USE Rank() TO EXECUTE ANY FUNCTION IMPORTED FROM THE nest
    MODULE ON A SUBSET OF RANKS IN AN MPI-PARALLEL SIMULATION.

    This will lead to unpredictable behavior. Symptoms may be an
    error message about non-synchronous global random number generators
    or deadlocks during simulation. In the worst case, the simulation
    may complete but generate nonsensical results.
    """

    sr("Rank")
    return spp()


@check_stack
def NumProcesses():
    """Return the overall number of MPI processes.

    Returns
    -------
    int:
        Number of overall MPI processes
    """

    sr("NumProcesses")
    return spp()


@check_stack
def SetAcceptableLatency(port_name, latency):
    """Set the acceptable latency (in ms) for a MUSIC port.

    Parameters
    ----------
    port_name : str
        MUSIC port to set latency for
    latency : float
        Latency in ms
    """

    sps(kernel.SLILiteral(port_name))
    sps(latency)
    sr("SetAcceptableLatency")


@check_stack
def SetMaxBuffered(port_name, size):
    """Set the maximum buffer size for a MUSIC port.

    Parameters
    ----------
    port_name : str
        MUSIC port to set buffer size for
    size : int
        Buffer size
    """

    sps(kernel.SLILiteral(port_name))
    sps(size)
    sr("SetMaxBuffered")


@check_stack
def SyncProcesses():
    """Synchronize all MPI processes.
    """

    sr("SyncProcesses")
