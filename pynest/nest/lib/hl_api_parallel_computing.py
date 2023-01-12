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
from .. import nestkernel_api as nestkernel

__all__ = [
    'NumProcesses',
    'Rank',
    'GetLocalVPs',
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
    DO NOT USE `Rank()` TO EXECUTE ANY FUNCTION IMPORTED FROM THE `nest`
    MODULE ON A SUBSET OF RANKS IN AN MPI-PARALLEL SIMULATION.

    This will lead to unpredictable behavior. Symptoms may be an
    error message about non-synchronous global random number generators
    or deadlocks during simulation. In the worst case, the simulation
    may complete but generate nonsensical results.
    """

    return nestkernel.llapi_get_rank()


@check_stack
def NumProcesses():
    """Return the overall number of MPI processes.

    Returns
    -------
    int:
        Number of overall MPI processes
    """

    return nestkernel.llapi_get_num_mpi_processes()


@check_stack
def SetAcceptableLatency(port_name, latency):
    """Set the acceptable `latency` (in ms) for a MUSIC port.

    Parameters
    ----------
    port_name : str
        MUSIC port to set latency for
    latency : float
        Latency in ms
    """

    # PYNEST-NG
    # sps(kernel.SLILiteral(port_name))
    # sps(latency)
    # sr("SetAcceptableLatency")
    pass


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

    # PYNEST-NG
    # sps(kernel.SLILiteral(port_name))
    # sps(size)
    # sr("SetMaxBuffered")
    pass


@check_stack
def SyncProcesses():
    """Synchronize all MPI processes.
    """

    # PYNEST-NG
    # sr("SyncProcesses")
    pass


@check_stack
def GetLocalVPs():
    """Return iterable representing the VPs local to the MPI rank.
    """

    # Compute local VPs as range based on round-robin logic in
    # VPManager::get_vp(). mpitest_get_local_vps ensures this is in
    # sync with the kernel.
    
    # PYNEST-NG
    # n_vp = sli_func("GetKernelStatus /total_num_virtual_procs get")
    n_vp = 1
    return range(Rank(), n_vp, NumProcesses())
