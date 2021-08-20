# -*- coding: utf-8 -*-
#
# __init__.py
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

"""PyNEST - Python interface for the NEST simulator

* ``nest.helpdesk()`` opens the NEST documentation in your browser.

* ``nest.__version__`` displays the NEST version.

* ``nest.Models()`` shows all available neuron, device and synapse models.

* ``nest.help('model_name') displays help for the given model, e.g., ``nest.help('iaf_psc_exp')``

* To get help on functions in the ``nest`` package, use Python's ``help()`` function
  or IPython's ``?``, e.g.

     - ``help(nest.Create)``
     - ``nest.Connect?``

For more information visit https://www.nest-simulator.org.

Kernel attributes
=================

**Note**

All NEST kernel attributes are described below, grouped by topic.
Some of them only provide information about the kernel status and
cannot be set by the user, these are marked as *read only*.


Time and resolution
-------------------

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


Random number generators
------------------------

rng_types : list, read only
    Names of random number generator types available.
    Types: "Philox_32", "Philox_64", "Threefry_32", "Threefry_64", "mt19937", "mt19937_64"
rng_type : str, default: mt19937_64
    Name of random number generator type used by NEST.
rng_seed : int, default: 143202461
    Seed value used as base for seeding NEST random number generators
    (:math:`1 \leq s \leq 2^{32}-1`).


Parallel processing
-------------------

total_num_virtual_procs : int, default: 1
    The total number of virtual processes
local_num_threads : int, default: 1
    The local number of threads
num_processes : int, read only
    The number of MPI processes
off_grid_spiking : bool, read only
    Whether to transmit precise spike times in MPI communication


MPI buffers
-----------

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


Gap junctions and rate models (waveform relaxation method)
----------------------------------------------------------

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


Synapses
--------

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


Output
------

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


Miscellaneous
-------------

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

"""


# Store interpreter-given module attributes to copy into replacement module
# instance later on. Use `.copy()` to prevent pollution with other variables
_original_module_attrs = globals().copy()

from .ll_api import KernelAttribute
import sys, types, importlib
if sys.version_info[0] == 2:
    msg = "Python 2 is no longer supported. Please use Python >= 3.6."
    raise Exception(msg)


def _rel_import_star(module, import_module_name):
    """Emulates `from X import *` into `module`"""

    imported = importlib.import_module(import_module_name, __name__)
    imp_iter = vars(imported).items()
    if hasattr(module, "__all__"):
        # If a public api is defined using the `__all__` attribute, copy that.
        module.update(kv for kv in imp_iter if kv[0] in imported.__all__)
    else:
        # Otherwise follow "underscore is private" convention.
        module.update(kv for kv in imp_iter if not kv[0].startswith("_"))


def _lazy_module_property(module_name):
    """
    Returns a property that lazy loads a module and substitutes itself with it.
    The class variable name must match given `module_name`::

      class ModuleClass(types.ModuleType):
          lazy_module_xy = _lazy_module_property("lazy_module_xy")
    """
    def lazy_loader(self):
        cls = type(self)
        delattr(cls, module_name)
        module = importlib.import_module("." + module_name, __name__)
        setattr(cls, module_name, module)
        return module

    return property(lazy_loader)


class NestModule(types.ModuleType):
    """
    A module class for the `nest` root module to control the dynamic generation
    of module level attributes such as the KernelAttributes and lazy loading
    some submodules.
    """
    from . import ll_api                             # noqa
    from .ll_api import set_communicator             # noqa

    from . import pynestkernel as kernel             # noqa

    from . import random                             # noqa
    from . import math                               # noqa
    from . import spatial_distributions              # noqa
    from . import logic                              # noqa

    try:
        from . import server                         # noqa
    except ImportError:
        pass

    # Lazy load the `spatial` module to avoid circular imports.
    spatial = _lazy_module_property("spatial")
    # Property for the full SLI `GetKernelStatus` dictionary
    kernel_status = KernelAttribute(None, "Get kernel status.", readonly=True)

    __version__ = ll_api.sli_func("statusdict /version get")

    def __dir__(self):
        return list(set(vars(self).keys()) | set(self.__all__))


# Instantiate a NestModule
_module = NestModule(__name__)
# We manipulate the nest module instance through its `__dict__` (= vars())
_module_dict = vars(_module)
# Copy over the original module attributes to preverse all interpreter given
# magic attributes such as `__name__`, `__path__`, `__package__`, ...
_module_dict.update(_original_module_attrs)

# Import public API of `.hl_api` into the nest module instance
_rel_import_star(_module_dict, ".hl_api")

_kernel_attr_names = set()
# Parse this module's docstring to obtain the kernel attributes.
for _line in __doc__.split('\n'):
    # Parse the `parameter : description. read only` lines
    if not ' : ' in _line:
        continue
    _param = _line.split(":")[0].strip()
    _readonly = "read only" in _line
    _kernel_attr_names.add(_param)
    # Create a kernel attribute descriptor and add it to the nest module
    _kernel_attr = KernelAttribute(_param, None, _readonly)
    setattr(NestModule, _param, _kernel_attr)
_module._kernel_attr_names = _kernel_attr_names

# Finalize the nest module instance by generating its public API.
_api = list(k for k in _module_dict if not k.startswith("_"))
_api.extend(k for k in dir(NestModule) if not k.startswith("_"))
_module.__all__ = list(set(_api))

# Set the nest module object as the return value of `import nest` using sys
sys.modules[__name__] = _module

# Some compiled/binary components (`pynestkernel.pyx` for example) of NEST
# obtain a reference to this file's original module object instead of what's in
# `sys.modules`. For these edge cases we make available all attributes of the
# nest module instance to this file's module object.
globals().update(_module_dict)

# Clean up obsolete references
del _kernel_attr_names, _rel_import_star, _lazy_module_property, _readonly, \
    _kernel_attr, _module, _module_dict, _original_module_attrs, _line, _param
