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
import warnings

from ..ll_api import *
from .hl_api_helper import *
from .hl_api_parallel_computing import Rank

__all__ = [
    'Cleanup',
    'DisableStructuralPlasticity',
    'EnableStructuralPlasticity',
    'GetKernelStatus',
    'Install',
    'Prepare',
    'ResetKernel',
    'Run',
    'RunManager',
    'SetKernelStatus',
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
    calls.

    Be careful about modifying the network or neurons between `Prepare` and `Cleanup`
    calls. In particular, do not call `Create`, `Connect`, or `SetKernelStatus`.
    Calling `SetStatus` to change membrane potential `V_m` of neurons or synaptic
    weights (but not delays!) will in most cases work as expected, while changing
    membrane or synaptic times constants will not work correctly. If in doubt, assume
    that changes may cause undefined behavior and check these thoroughly.

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
            for _ in range(10):
                Run(100)
                # extract results

    Notes
    -----

    Be careful about modifying the network or neurons inside the `RunManager` context.
    In particular, do not call `Create`, `Connect`, or `SetKernelStatus`. Calling `SetStatus`
    to change membrane potential `V_m` of neurons or synaptic weights (but not delays!)
    will in most cases work as expected, while changing membrane or synaptic times
    constants will not work correctly. If in doubt, assume that changes may cause
    undefined behavior and check these thoroughly.

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
    r"""Set parameters for the simulation kernel.

    Parameters
    ----------

    params : dict
        Dictionary of parameters to set.


    **Note**

    All NEST kernel parameters are described below, grouped by topic.
    Some of them only provide information about the kernel status and
    cannot be set by the user. These are marked as *read only* and can
    be accessed using ``GetKernelStatus``.


    **Time and resolution**

    Parameters
    ----------

    resolution : float, default: 0.1
        The resolution of the simulation (in ms)
    time : float
        The current simulation time (in ms)
    to_do : int, read only
        The number of steps yet to be simulated
    max_delay : float, default: 0.1
        The maximum delay in the network
    min_delay : float, default: 0.1
        The minimum delay in the network
    ms_per_tic : float, default: 0.001
        The number of milliseconds per tic
    tics_per_ms : float, default: 1000.0
        The number of tics per millisecond
    tics_per_step : int, default: 100
        The number of tics per simulation time step
    T_max : float, read only
        The largest representable time value
    T_min : float, read only
        The smallest representable time value


    **Random number generators**

    Parameters
    ----------

    rng_types : list, read only
        Names of random number generator types available.
        Types: "Philox_32", "Philox_64", "Threefry_32", "Threefry_64", "mt19937", "mt19937_64"
    rng_type : str, default: mt19937_64
        Name of random number generator type used by NEST.
    rng_seed : int, default: 143202461
        Seed value used as base for seeding NEST random number generators
        (:math:`1 \leq s \leq 2^{32}-1`).


    **Parallel processing**

    Parameters
    ----------

    total_num_virtual_procs : int, default: 1
        The total number of virtual processes
    local_num_threads : int, default: 1
        The local number of threads
    num_processes : int, read only
        The number of MPI processes
    off_grid_spiking : bool, read only
        Whether to transmit precise spike times in MPI communication


    **MPI buffers**

    Parameters
    ----------

    adaptive_spike_buffers  : bool, default: True
        Whether MPI buffers for communication of spikes resize on the fly
    adaptive_target_buffers : bool, default: True
        Whether MPI buffers for communication of connections resize on the fly
    buffer_size_secondary_events : int, read only
        Size of MPI buffers for communicating secondary events (in bytes, per
        MPI rank, for developers)
    buffer_size_spike_data : int, default: 2
        Total size of MPI buffer for communication of spikes
    buffer_size_target_data : int, default: 2
        Total size of MPI buffer for communication of connections
    growth_factor_buffer_spike_data : float, default: 1.5
        If MPI buffers for communication of spikes resize on the fly, grow
        them by this factor each round
    growth_factor_buffer_target_data : float, default: 1.5
        If MPI buffers for communication of connections resize on the fly, grow
        them by this factor each round
    max_buffer_size_spike_data : int, default: 8388608
        Maximal size of MPI buffers for communication of spikes.
    max_buffer_size_target_data : int, default: 16777216
        Maximal size of MPI buffers for communication of connections


    **Gap junctions and rate models (waveform relaxation method)**

    Parameters
    ----------

    use_wfr : bool, default: True
        Whether to use waveform relaxation method
    wfr_comm_interval : float, default: 1.0
        Desired waveform relaxation communication interval
    wfr_tol : float, default: 0.0001
        Convergence tolerance of waveform relaxation method
    wfr_max_iterations : int, default: 15
        Maximal number of iterations used for waveform relaxation
    wfr_interpolation_order : int, default: 3
        Interpolation order of polynomial used in wfr iterations


    **Synapses**

    Parameters
    ----------

    max_num_syn_models : int, read only
        Maximal number of synapse models supported
    sort_connections_by_source : bool, default: True
        Whether to sort connections by their source; increases construction
        time of presynaptic data structures, decreases simulation time if the
        average number of outgoing connections per neuron is smaller than the
        total number of threads
    structural_plasticity_synapses : dict
        Defines all synapses which are plastic for the structural plasticity
        algorithm. Each entry in the dictionary is composed of a synapse model,
        the pre synaptic element and the postsynaptic element
    structural_plasticity_update_interval : int, default: 10000.0
        Defines the time interval in ms at which the structural plasticity
        manager will make changes in the structure of the network (creation
        and deletion of plastic synapses)
    use_compressed_spikes : bool, default: True
        Whether to use spike compression; if a neuron has targets on
        multiple threads of a process, this switch makes sure that only
        a single packet is sent to the process instead of one packet per
        target thread; requires sort_connections_by_source = true


    **Output**

    Parameters
    -------

    data_path : str
        A path, where all data is written to (default is the current
        directory)
    data_prefix : str
        A common prefix for all data files
    overwrite_files : bool, default: False
        Whether to overwrite existing data files
    print_time : bool, default: False
        Whether to print progress information during the simulation
    network_size : int, read only
        The number of nodes in the network
    num_connections : int, read only, local only
        The number of connections in the network
    local_spike_counter : int, read only
        Number of spikes fired by neurons on a given MPI rank during the most
        recent call to :py:func:`.Simulate`. Only spikes from "normal" neurons
        are counted, not spikes generated by devices such as ``poisson_generator``.
    recording_backends : list of str
        List of available backends for recording devices:
        "memory", "ascii", "screen"


    **Miscellaneous**

    Parameters
    ----------

    dict_miss_is_error : bool, default: True
        Whether missed dictionary entries are treated as errors
    keep_source_table : bool, default: True
        Whether to keep source table after connection setup is complete
    min_update_time: double, read only
        Shortest wall-clock time measured so far for a full update step [seconds].
    max_update_time: double, read only
        Longest wall-clock time measured so far for a full update step [seconds].
    update_time_limit: double
        Maximum wall-clock time for one full update step in seconds, default +inf.
        This can be used to terminate simulations that slow down significantly.
        Simulations may still get stuck if the slowdown occurs within a single update
        step.

    See Also
    --------

    GetKernelStatus

    """
    # Resolve if missing entries should raise errors
    raise_errors = params.get('dict_miss_is_error')
    if raise_errors is None:
        raise_errors = GetKernelStatus('dict_miss_is_error')

    # Check validity of passed parameters
    keys = list(params.keys())
    for key in keys:
        readonly = _sks_params.get(key)
        msg = None
        if readonly is None:
            # If the parameter is not in the docstring
            msg = f'`{key}` is not a valid kernel parameter, ' + \
                  'valid parameters are: ' + \
                  ', '.join(f"'{p}'" for p in _sks_params.keys())
        elif readonly:
            # If the parameter is tagged as read only
            msg = f'`{key}` is a read only parameter and cannot ' + \
                  'be defined using SetKernelStatus'
        # Raise error or warn the user
        if msg is not None:
            if raise_errors:
                raise ValueError(msg)
            else:
                warnings.warn(msg + f' \n`{key}` has been ignored')
                del params[key]

    sps(params)
    sr('SetKernelStatus')


# Parse the `SetKernelStatus` docstring to obtain all valid and readonly params
doc_lines = SetKernelStatus.__doc__.split('\n')
# Get the lines describing parameters
param_lines = (line.strip() for line in doc_lines if ' : ' in line)
# Exclude the first parameter `params`.
next(param_lines)
_sks_params = {ln.split(" :")[0]: "read only" in ln for ln in param_lines}
del doc_lines, param_lines


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
