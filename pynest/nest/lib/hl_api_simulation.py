# -*- coding: utf-8 -*-
#
# hl_api_simulation.py
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
Functions for simulation control
"""

from contextlib import contextmanager

from ..ll_api import *
from .hl_api_helper import *

__all__ = [
    'Cleanup',
    'DisableStructuralPlasticity',
    'EnableStructuralPlasticity',
    'GetKernelStatus',
    'GetStructuralPlasticityStatus',
    'Install',
    'Prepare',
    'ResetKernel',
    'Run',
    'RunManager',
    'SetKernelStatus',
    'SetStructuralPlasticityStatus',
    'Simulate',
]


@check_stack
def Simulate(t):
    """Simulate the network for `t` milliseconds.

    Parameters
    ----------
    t : float
        Time to simulate in ms

    See Also
    --------
    RunManager

    """

    sps(float(t))
    sr('ms Simulate')


@check_stack
def Run(t):
    """Simulate the network for `t` milliseconds.

    Parameters
    ----------
    t : float
        Time to simulate in ms

    Notes
    ------

    Call between `Prepare` and `Cleanup` calls, or within a
    ``with RunManager`` clause.

    Simulate(t): t' = t/m; Prepare(); for _ in range(m): Run(t'); Cleanup()

    `Prepare` must be called before `Run` to calibrate the system, and
    `Cleanup` must be called after `Run` to close files, cleanup handles, and
    so on. After `Cleanup`, `Prepare` can and must be called before more `Run`
    calls. Any calls to `SetStatus` between `Prepare` and `Cleanup` have
    undefined behaviour.

    See Also
    --------
    Prepare, Cleanup, RunManager, Simulate

    """

    sps(float(t))
    sr('ms Run')


@check_stack
def Prepare():
    """Calibrate the system before a `Run` call. Not needed for `Simulate`.

    Call before the first `Run` call, or before calling `Run` after changing
    the system, calling `SetStatus` or `Cleanup`.

    See Also
    --------
    Run, Cleanup

    """

    sr('Prepare')


@check_stack
def Cleanup():
    """Cleans up resources after a `Run` call. Not needed for `Simulate`.

    Closes state for a series of runs, such as flushing and closing files.
    A `Prepare` is needed after a `Cleanup` before any more calls to `Run`.

    See Also
    --------
    Run, Prepare

    """
    sr('Cleanup')


@contextmanager
def RunManager():
    """ContextManager for `Run`

    Calls `Prepare` before a series of `Run` calls, and calls `Cleanup` at end.

    E.g.:

    ::

        with RunManager():
            for i in range(10):
                Run()

    See Also
    --------
    Prepare, Run, Cleanup, Simulate

    """

    Prepare()
    try:
        yield
    finally:
        Cleanup()


@check_stack
def ResetKernel():
    """Reset the simulation kernel.

    This will destroy the network as well as all custom models created with
    :py:func:`.CopyModel`. Calling this function is equivalent to restarting NEST.

    In particular,

    * all network nodes
    * all connections
    * all user-defined neuron and synapse models
    are deleted, and

    * time
    * random generators
    are reset. The only exception is that dynamically loaded modules are not
    unloaded. This may change in a future version of NEST.

   """

    sr('ResetKernel')


@check_stack
def SetKernelStatus(params):
    """Set parameters for the simulation kernel.

    Parameters
    ----------

    params : dict
        Dictionary of parameters to set.


    Params dictionary

    Some of the keywords in the kernel status dictionary are internally
    calculated, and cannot be defined by the user. These are flagged as
    `read only` in the parameter list. Use GetKernelStatus to access their
    assigned values.


    Time and resolution

    Parameters
    ----------

    resolution : float
        The resolution of the simulation (in ms)
    time : float
        The current simulation time (in ms)
    to_do : int, read only
        The number of steps yet to be simulated
    max_delay : float
        The maximum delay in the network
    min_delay : float
        The minimum delay in the network
    ms_per_tic : float
        The number of milliseconds per tic
    tics_per_ms : float
        The number of tics per millisecond
    tics_per_step : int
        The number of tics per simulation time step
    T_max : float, read only
        The largest representable time value
    T_min : float, read only
        The smallest representable time value


    Parallel processing

    Parameters
    ----------

    total_num_virtual_procs : int
        The total number of virtual processes
    local_num_threads : int
        The local number of threads
    num_processes : int, read only
        The number of MPI processes
    off_grid_spiking : bool
        Whether to transmit precise spike times in MPI communication
    grng_seed : int
        Seed for global random number generator used synchronously by all
        virtual processes to create, e.g., fixed fan-out connections.
    rng_seeds : array
        Seeds for the per-virtual-process random number generators used for
        most purposes. Array with one integer per virtual process, all must
        be unique and differ from grng_seed.


    MPI buffers

    Parameters
    ----------

    adaptive_spike_buffers  : bool
        Whether MPI buffers for communication of spikes resize on the fly
    adaptive_target_buffers : bool
        Whether MPI buffers for communication of connections resize on the fly
    buffer_size_secondary_events : int, read only
        Size of MPI buffers for communicating secondary events (in bytes, per
        MPI rank, for developers)
    buffer_size_spike_data : int
        Total size of MPI buffer for communication of spikes
    buffer_size_target_data : int
        Total size of MPI buffer for communication of connections
    growth_factor_buffer_spike_data : float
        If MPI buffers for communication of spikes resize on the fly, grow
        them by this factor each round
    growth_factor_buffer_target_data : float
        If MPI buffers for communication of connections resize on the fly, grow
        them by this factor each round
    max_buffer_size_spike_data : int
        Maximal size of MPI buffers for communication of spikes.
    max_buffer_size_target_data : int
        Maximal size of MPI buffers for communication of connections


    Waveform relaxation method (wfr)

    Parameters
    ----------

    use_wfr : bool
        Whether to use waveform relaxation method
    wfr_comm_interval : float
        Desired waveform relaxation communication interval
    wfr_tol : float
        Convergence tolerance of waveform relaxation method
    wfr_max_iterations : int
        Maximal number of iterations used for waveform relaxation
    wfr_interpolation_order : int
        Interpolation order of polynomial used in wfr iterations


    Synapses

    Parameters
    ----------

    max_num_syn_models : int, read only
        Maximal number of synapse models supported
    sort_connections_by_source : bool
        Whether to sort connections by their source; increases construction
        time of presynaptic data structures, decreases simulation time if the
        average number of outgoing connections per neuron is smaller than the
        total number of threads
    structural_plasticity_synapses : dict
        Defines all synapses which are plastic for the structural plasticity
        algorithm. Each entry in the dictionary is composed of a synapse model,
        the pre synaptic element and the post synaptic element
    structural_plasticity_update_interval : int
        Defines the time interval in ms at which the structural plasticity
        manager will make changes in the structure of the network (creation
        and deletion of plastic synapses)


    Output

    Returns
    -------

    data_path : str
        A path, where all data is written to (default is the current
        directory)
    data_prefix : str
        A common prefix for all data files
    overwrite_files : bool
        Whether to overwrite existing data files
    print_time : bool
        Whether to print progress information during the simulation
    network_size : int, read only
        The number of nodes in the network
    num_connections : int, read only, local only
        The number of connections in the network
    local_spike_counter : int, read only
        Number of spikes fired by neurons on a given MPI rank since NEST was
        started or the last ResetKernel. Only spikes from "normal" neurons
        (neuron models with proxies) are counted, not spikes generated by
        devices such as poisson_generator.


    Miscellaneous

    Other Parameters
    ----------------

    dict_miss_is_error : bool
        Whether missed dictionary entries are treated as errors
    keep_source_table : bool
        Whether to keep source table after connection setup is complete

    See Also
    --------

    GetKernelStatus

    """

    sps(params)
    sr('SetKernelStatus')


@check_stack
def GetKernelStatus(keys=None):
    """Obtain parameters of the simulation kernel.

    Parameters
    ----------

    keys : str or list, optional
        Single parameter name or `list` of parameter names

    Returns
    -------

    dict:
        Parameter dictionary, if called without argument
    type:
        Single parameter value, if called with single parameter name
    list:
        List of parameter values, if called with list of parameter names

    Raises
    ------

    TypeError
        If `keys` are of the wrong type.

    Notes
    -----
    See SetKernelStatus for documentation on each parameter key.

    See Also
    --------
    SetKernelStatus

    """

    sr('GetKernelStatus')
    status_root = spp()

    if keys is None:
        return status_root
    elif is_literal(keys):
        return status_root[keys]
    elif is_iterable(keys):
        return tuple(status_root[k] for k in keys)
    else:
        raise TypeError("keys should be either a string or an iterable")


@check_stack
def Install(module_name):
    """Load a dynamically linked NEST module.

    Parameters
    ----------
    module_name : str
        Name of the dynamically linked module

    Returns
    -------
    handle
        NEST module identifier, required for unloading

    Notes
    -----
    Dynamically linked modules are searched in the NEST library
    directory (``<prefix>/lib/nest``) and in ``LD_LIBRARY_PATH`` (on
    Linux) or ``DYLD_LIBRARY_PATH`` (on OSX).

    **Example**
    ::

        nest.Install("mymodule")

    """

    return sr("(%s) Install" % module_name)


@check_stack
def SetStructuralPlasticityStatus(params):
    """Set structural plasticity parameters for the network simulation.

    Parameters
    ----------
    params : dict
        Dictionary of structural plasticity parameters to set

    See Also
    --------
    GetStructuralPlasticityStatus

    """

    sps(params)
    sr('SetStructuralPlasticityStatus')


@check_stack
def GetStructuralPlasticityStatus(keys=None):
    """Get the current structural plasticity parameters

    Parameters
    ---------
    keys : str or list, optional
        Keys indicating the values of interest to be retrieved by the get call

    See Also
    --------
    SetStructuralPlasticityStatus

    """

    sps({})
    sr('GetStructuralPlasticityStatus')
    d = spp()
    if keys is None:
        return d
    elif is_literal(keys):
        return d[keys]
    elif is_iterable(keys):
        return tuple(d[k] for k in keys)
    else:
        raise TypeError("keys must be either empty, a string or a list")


@check_stack
def EnableStructuralPlasticity():
    """Enable structural plasticity for the network simulation

    See Also
    --------
    DisableStructuralPlasticity

    """

    sr('EnableStructuralPlasticity')


@check_stack
def DisableStructuralPlasticity():
    """Disable structural plasticity for the network simulation

    See Also
    --------
    EnableStructuralPlasticity

    """
    sr('DisableStructuralPlasticity')
