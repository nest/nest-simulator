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

from .hl_api_helper import *


@check_stack
def Rank():
    """Return the MPI rank of the local process.

    .. warning::

        DO NOT USE `Rank` TO EXECUTE ANY FUNCTION IMPORTED FROM THE `nest`
        MODULE ON A SUBSET OF RANKS IN AN MPI-PARALLEL SIMULATION.

        This will lead to unpredictable behavior. Symptoms may be an
        error message about non-synchronous global random number generators
        or deadlocks during simulation. In the worst case, the simulation
        may complete but generate nonsensical results.

    Returns
    -------
    int :
        MPI rank of the local process

    See Also
    --------
    NumProcesses

    KEYWORDS:
    """

    sr("Rank")
    return spp()


@check_stack
def NumProcesses():
    """Return the overall number of MPI processes.

    Returns
    -------
    int :
        Number of overall MPI processes

    See Also
    --------
    Rank

    KEYWORDS:
    """

    sr("NumProcesses")
    return spp()


@check_stack
def SetNumRecProcesses(nrp):
    """Set the number of recording MPI processes.

    Usually, spike detectors are distributed over all processes and record
    from local neurons only. If a number of processes is dedicated to spike
    detection, each spike detector is hosted on one of these processes and
    records globally from all simulating processes.

    The number of recording MPI processes has to be lower than the total number
    of MPI processes.

    Parameters
    ----------
    nrp : int
        Number of recording MPI processes

    Raises
    ------
    NESTError
        if `nrp` is not smaller than total number of processes

    KEYWORDS:
    """

    sr("%d SetNumRecProcesses" % nrp)


@check_stack
def SetAcceptableLatency(port_name, latency):
    """Set the acceptable latency (in ms) for a MUSIC port.

    Note that you need to have compiled NEST with MUSIC for this to work.

    Parameters
    ----------
    port_name : str
        MUSIC port to set latency for
    latency : float
        Latency in ms

    See Also
    --------
    SetMaxBuffered

    KEYWORDS:
    """

    sps(kernel.SLILiteral(port_name))
    sps(latency)
    sr("SetAcceptableLatency")


@check_stack
def SetMaxBuffered(port_name, size):
    """Set the maximum buffer size for a MUSIC port.

    Note that you need to have compiled NEST with MUSIC for this to work.

    Parameters
    ----------
    port_name : str
        MUSIC port to set buffer size for
    size : int
        Buffer size

    See Also
    --------
    SetAcceptableLatency

    KEYWORDS:
    """

    sps(kernel.SLILiteral(port_name))
    sps(size)
    sr("SetMaxBuffered")
