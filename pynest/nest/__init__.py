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

r"""PyNEST - Python interface for the NEST Simulator

* ``nest.__version__`` displays the NEST version.

* ``nest.build_info`` provides detailed information about the NEST build.

* ``nest.node_models`` shows all available neuron and device models.

* ``nest.synapse_models`` shows all available synapse models.

* To get details on the model equations and parameters,
  please check out our model documentation at https://nest-simulator.readthedocs.io/en/stable/models/index.html.

* To get help on functions in the ``nest`` package, use Python's
  ``help()`` function or IPython's ``?``, e.g.
     - ``help(nest.Create)``
     - ``nest.Connect?``

For more information visit https://www.nest-simulator.org.
"""

# The `nest` module is a container of lazily imported submodules, lazily loaded
# attribute shortcuts to said submodules, and kernel attributes (which read and write
# kernel values). The dynamic behaviour of the module is achieved by retyping this
# module object to the `NestModule` type defined below. Assigning a module's
# `__class__` is supported for exactly this purpose since Python 3.5; see
# https://docs.python.org/3/reference/datamodel.html#module.__class__. There are two
# main dynamic behaviours:
#
# 1. Submodule attribute shortcuts
# --------------------------------
#
# All the public attributes of the nest submodules are directly available on the nest
# module itself:
#
#     import nest
#
#     nest.Create(...)
#
# This is achieved by a `__getattr__` method on the `NestModule`, backed by a symbol
# map between the public interface members that the submodules declare in their
# `__all__` and the submodule that defines them. The map is read from the sources, so
# building it imports nothing, and looking a member up imports only the one submodule
# that defines it. Static typing is enabled by importing all submodules in a
# type-checking only block (which is skipped at runtime).
#
# 2. Kernel attributes
# --------------------
#
# Kernel attributes read and write values of the C++ nest kernel. They are declared as
# static type hints, which `_install_kernel_attributes` turns into descriptors on the
# `NestModule` type, so that reading and writing `nest.<name>` reads and writes the
# kernel status.
#
# The layout of the file follows those two halves: it begins with the regular import
# statements, then the static declarations (the type-checking block for (1) and the
# kernel attribute type hints for (2)), then the `NestModule` type that gives them
# their dynamic behaviour, and it ends by retyping the module object.
#
# pylint: disable=wildcard-import, unused-wildcard-import, no-name-in-module, invalid-name

import sys
import types
import typing
from typing import Annotated

from . import ll_api as _ll_api  # noqa: F401  (importing `ll_api` starts the kernel)
from .ll_api import KernelAttribute, set_communicator  # noqa: F401

if typing.TYPE_CHECKING:
    # Static analysis has no way to follow `NestModule.__getattr__`, so the namespace it
    # builds at runtime is spelled out here for the benefit of type checkers and IDEs.
    # Nothing reads this block at runtime, since the runtime discovers both the submodules
    # and the `hl_api` modules from the package directory, so a stale entry costs type
    # information, never correctness. `test_api_surface.py` fails if the two drift apart.
    from . import (  # noqa: F401
        ll_api,
        logic,
        math,
        random,
        raster_plot,
        server,
        spatial,
        spatial_distributions,
        visualization,
        voltage_trace,
    )
    from .lib.hl_api_connections import *  # noqa: F401,F403
    from .lib.hl_api_info import *  # noqa: F401,F403
    from .lib.hl_api_models import *  # noqa: F401,F403
    from .lib.hl_api_nodes import *  # noqa: F401,F403
    from .lib.hl_api_parallel_computing import *  # noqa: F401,F403
    from .lib.hl_api_simulation import *  # noqa: F401,F403
    from .lib.hl_api_sonata import *  # noqa: F401,F403
    from .lib.hl_api_spatial import *  # noqa: F401,F403
    from .lib.hl_api_types import *  # noqa: F401,F403

try:
    # Bound under a private name so that it stays out of the `nest` API.
    import versionchecker as _versionchecker  # noqa: F401
except ImportError:
    pass

NESTErrors = _ll_api.nestkernel.NESTErrors
NESTError = _ll_api.nestkernel.NESTErrors.KernelException


# Define the kernel attributes.
#
# FORMATTING NOTES:
# * The description is dedented and a `.` is appended, so write it as a normal sentence
#   and indent continuation lines to match the surrounding code.
# * Strings containing a colon render incorrectly.

kernel_status: Annotated[dict, KernelAttribute("Get the complete kernel status", readonly=True)]
resolution: Annotated[float, KernelAttribute("The resolution of the simulation (in ms)", default=0.1)]
biological_time: Annotated[float, KernelAttribute("The current simulation time (in ms)")]
build_info: Annotated[
    dict, KernelAttribute("Information about the build and compile configuration of NEST", readonly=True)
]
memory_size: Annotated[int, KernelAttribute("Memory size of NEST process in kB (-1 if unavailable)", readonly=True)]
to_do: Annotated[int, KernelAttribute("The number of steps yet to be simulated", readonly=True)]
max_delay: Annotated[float, KernelAttribute("The maximum delay in the network", default=0.1)]
min_delay: Annotated[float, KernelAttribute("The minimum delay in the network", default=0.1)]
ms_per_tic: Annotated[
    float,
    KernelAttribute("The number of milliseconds per tic. Calculated by ms_per_tic = 1 / tics_per_ms", readonly=True),
]
tics_per_ms: Annotated[
    float,
    KernelAttribute(
        """
        The number of tics per millisecond. Change of tics_per_ms requires simultaneous
        specification of resolution
        """,
        default=1000.0,
    ),
]
tics_per_step: Annotated[
    int,
    KernelAttribute(
        """
        The number of tics per simulation time step. Calculated as tics_per_step = resolution *
        tics_per_ms
        """,
        readonly=True,
    ),
]
T_max: Annotated[float, KernelAttribute("The largest representable time value", readonly=True)]
T_min: Annotated[float, KernelAttribute("The smallest representable time value", readonly=True)]
rng_types: Annotated[list[str], KernelAttribute("List of available random number generator types", readonly=True)]
rng_type: Annotated[str, KernelAttribute("Name of random number generator type used by NEST", default="mt19937_64")]
rng_seed: Annotated[
    int,
    KernelAttribute(
        r"""
        Seed value used as base for seeding NEST random number generators
        (:math:`1 \leq s\leq 2^{32}-1`)
        """,
        default=143202461,
    ),
]
total_num_virtual_procs: Annotated[int, KernelAttribute("The total number of virtual processes", default=1)]
local_num_threads: Annotated[int, KernelAttribute("The local number of threads", default=1)]
num_processes: Annotated[int, KernelAttribute("The number of MPI processes", readonly=True)]
off_grid_spiking: Annotated[
    bool, KernelAttribute("Whether to transmit precise spike times in MPI communication", readonly=True)
]
adaptive_target_buffers: Annotated[
    bool, KernelAttribute("Whether MPI buffers for communication of connections resize on the fly", default=True)
]
send_buffer_size_secondary_events: Annotated[
    int,
    KernelAttribute(
        """
        Size of MPI send buffers for communicating secondary events (in bytes, per MPI rank, for
        developers)
        """,
        readonly=True,
    ),
]
recv_buffer_size_secondary_events: Annotated[
    int,
    KernelAttribute(
        """
        Size of MPI recv buffers for communicating secondary events (in bytes, per MPI rank, for
        developers)
        """,
        readonly=True,
    ),
]
buffer_size_spike_data: Annotated[
    int, KernelAttribute("Total size of MPI buffer for communication of spikes", default=2)
]
buffer_size_target_data: Annotated[
    int, KernelAttribute("Total size of MPI buffer for communication of connections", default=2)
]
growth_factor_buffer_target_data: Annotated[
    float,
    KernelAttribute(
        """
        If MPI buffers for communication of connections resize on the fly, grow them by this factor
        each round
        """,
        default=1.5,
    ),
]
max_buffer_size_target_data: Annotated[
    int, KernelAttribute("Maximal size of MPI buffers for communication of connections", default=16777216)
]
spike_buffer_grow_extra: Annotated[
    float,
    KernelAttribute(
        """
        When spike exchange buffer is expanded, resize it to `(1 + spike_buffer_grow_extra) *
        required_buffer_size`
        """,
        default=0.5,
    ),
]
spike_buffer_shrink_limit: Annotated[
    float,
    KernelAttribute(
        """
        If the largest number of spikes sent from any rank to any rank is less than
        `spike_buffer_shrink_limit * buffer_size`, then reduce buffer size.
        `spike_buffer_shrink_limit == 0` means that buffers never shrink. See
        ``spike_buffer_shrink_spare`` for how the new buffer size is determined
        """,
        default=0.2,
    ),
]
spike_buffer_shrink_spare: Annotated[
    float,
    KernelAttribute(
        """
        When the buffer shrinks, set the new size to `(1 + spike_buffer_shrink_spare) *
        required_buffer_size`. See `spike_buffer_shrink_limit` for when buffers shrink
        """,
        default=0.1,
    ),
]
spike_buffer_resize_log: Annotated[
    dict,
    KernelAttribute(
        """
        Log of spike buffer resizing as a dictionary. It contains the `times` of the resizings
        (simulation clock in steps, always multiple of ``min_delay``), ``global_max_spikes_sent``,
        that is, the observed spike number that triggered the resize, and the ``new_buffer_size``.
        Sizes for the buffer section sent from one rank to another rank
        """,
        readonly=True,
    ),
]
cycle_time_log: Annotated[
    dict, KernelAttribute("Information on the duration and spike counts within each update cycle.", readonly=True)
]
use_wfr: Annotated[bool, KernelAttribute("Whether to use waveform relaxation method", default=True)]
wfr_comm_interval: Annotated[float, KernelAttribute("Desired waveform relaxation communication interval", default=1.0)]
wfr_tol: Annotated[float, KernelAttribute("Convergence tolerance of waveform relaxation method", default=0.0001)]
wfr_max_iterations: Annotated[
    int, KernelAttribute("Maximal number of iterations used for waveform relaxation", default=15)
]
wfr_interpolation_order: Annotated[
    int, KernelAttribute("Interpolation order of polynomial used in wfr iterations", default=3)
]
max_num_syn_models: Annotated[int, KernelAttribute("Maximal number of synapse models supported", readonly=True)]
structural_plasticity_synapses: Annotated[
    dict,
    KernelAttribute(
        """
        Defines all synapses which are plastic for the structural plasticity algorithm. Each entry
        in the dictionary is composed of a synapse model, the presynaptic element and the
        postsynaptic element
        """,
    ),
]
structural_plasticity_update_interval: Annotated[
    int,
    KernelAttribute(
        """
        Defines the time interval in ms at which the structural plasticity manager will make changes
        in the structure of the network ( creation and deletion of plastic synapses)
        """,
        default=10000,
    ),
]
growth_curves: Annotated[
    list[str], KernelAttribute("The list of the available structural plasticity growth curves", readonly=True)
]
use_compressed_spikes: Annotated[
    bool,
    KernelAttribute(
        """
        Whether to use spike compression; if a neuron has targets on multiple threads of a process,
        this switch makes sure that only a single packet is sent to the process instead of one
        packet per target thread; it implies that connections are sorted by source.
        """,
        default=True,
    ),
]
data_path: Annotated[str, KernelAttribute("A path, where all data is written to, defaults to current directory")]
data_prefix: Annotated[str, KernelAttribute("A common prefix for all data files")]
overwrite_files: Annotated[bool, KernelAttribute("Whether to overwrite existing data files", default=False)]
print_time: Annotated[
    bool, KernelAttribute("Whether to print progress information during the simulation", default=False)
]
network_size: Annotated[int, KernelAttribute("The number of nodes in the network", readonly=True)]
num_connections: Annotated[
    int, KernelAttribute("The number of connections in the network", readonly=True, localonly=True)
]
connection_rules: Annotated[list[str], KernelAttribute("The list of available connection rules", readonly=True)]
node_models: Annotated[
    list[str], KernelAttribute("The list of the available node (i.e., neuron or device) models", readonly=True)
]
synapse_models: Annotated[list[str], KernelAttribute("The list of the available synapse models", readonly=True)]
local_spike_counter: Annotated[
    int,
    KernelAttribute(
        """
        Number of spikes fired by neurons on a given MPI rank during the most recent call to
        :py:func:`.Simulate`. Only spikes from "normal" neurons are counted, not spikes generated by
        devices such as ``poisson_generator``. Resets on each call to ``Simulate`` or ``Run``.
        """,
        readonly=True,
    ),
]
recording_backends: Annotated[
    list[str], KernelAttribute("List of available backends for recording devices", readonly=True)
]
stimulation_backends: Annotated[
    list[str], KernelAttribute("List of available backends for stimulation devices", readonly=True)
]
dict_miss_is_error: Annotated[
    bool, KernelAttribute("Whether missed dictionary entries are treated as errors", default=True)
]
keep_source_table: Annotated[
    bool, KernelAttribute("Whether to keep source table after connection setup is complete", default=True)
]
min_update_time: Annotated[
    float, KernelAttribute("Shortest wall-clock time measured so far for a full update step [seconds]", readonly=True)
]
max_update_time: Annotated[
    float, KernelAttribute("Longest wall-clock time measured so far for a full update step [seconds]", readonly=True)
]
update_time_limit: Annotated[
    float,
    KernelAttribute(
        """
        Maximum wall-clock time for one full update step [seconds]. This can be used to terminate
        simulations that slow down significantly. Simulations may still get stuck if the slowdown
        occurs within a single update step
        """,
        default=float("+inf"),
    ),
]
eprop_update_interval: Annotated[
    float, KernelAttribute("Task-specific update interval of the e-prop plasticity mechanism [ms].", default=1000.0)
]
eprop_learning_window: Annotated[
    float, KernelAttribute("Task-specific learning window of the e-prop plasticity mechanism [ms].", default=1000.0)
]
eprop_reset_neurons_on_update: Annotated[
    bool, KernelAttribute("If True, reset dynamic variables of e-prop neurons upon e-prop update.", default=True)
]
verbosity: Annotated[
    _ll_api.nestkernel.VerbosityLevel,
    KernelAttribute(
        """
        Controls NEST's verbosity. The following levels are available, from most to least chatty:
        ALL, DEBUG, STATUS, INFO, PROGRESS, DEPRECATED, WARNING, ERROR, FATAL, QUIET. Default
        verbosity is INFO. To start NEST with a different verbosity and supress the startup message,
        set the environment variable PYNEST_QUIET=1
        """,
        default=_ll_api.nestkernel.VerbosityLevel.INFO,
    ),
]


class NestModule(types.ModuleType):
    """
    Type of the ``nest`` root module.

    The kernel attributes are descriptors: reading ``nest.<attribute>`` reads the
    status of the running NEST kernel and assigning to it writes that status.
    Descriptors are only honoured when they live on a type, which is why the ``nest``
    module object is given this type rather than plain ``ModuleType``.
    """

    # Submodules that `nest` uses to reach the kernel or that only matter during
    # start-up. They stay reachable, but `nest` does not advertise them.
    _PRIVATE_SUBMODULES = frozenset({"lib", "nestkernel_api", "versionchecker"})

    # Filled in on first use by `_submodules()` and `_symbols()`.
    _submodule_names = None
    _symbol_map = None

    def _submodules(self):
        """Names of the submodules `nest` exposes, read from the package directory."""

        import pkgutil

        if NestModule._submodule_names is None:
            NestModule._submodule_names = frozenset(
                name
                for _, name, _ in pkgutil.iter_modules(self.__path__)
                if not name.startswith("_") and name not in NestModule._PRIVATE_SUBMODULES
            )
        return NestModule._submodule_names

    def _symbols(self):
        """
        Map each name that the ``lib.hl_api_*`` modules export onto the module that
        exports it. The whole map is built on first use and kept on the class, and it
        is read from the sources, so building it imports nothing and resolving a name
        imports only the one module that owns it. The ``*_helper`` modules are internal
        and are skipped, as they are by the documentation build.

        `os` rather than `pathlib`, because `nest` has already imported the one and
        importing the other costs more than the whole map build.
        """

        import os

        if NestModule._symbol_map is None:
            symbols = {}
            lib = os.path.join(self.__path__[0], "lib")
            for filename in sorted(os.listdir(lib)):
                if filename.startswith("hl_api_") and filename.endswith(".py") and "helper" not in filename:
                    module = f".lib.{filename[:-len('.py')]}"
                    with open(os.path.join(lib, filename), encoding="utf-8") as source:
                        symbols.update((name, module) for name in self._exported_names(source.read()))
            NestModule._symbol_map = symbols
        return NestModule._symbol_map

    def _exported_names(self, source):
        """
        The names that `source` declares in its ``__all__``.

        In every `hl_api` module that declaration is a plain list of string literals, so
        it is matched directly instead of parsing or tokenizing the module, which is
        what keeps building the whole map to well under a millisecond. Two details do
        the work of a parser. The anchor is `__all__ = [` at the start of a line, which
        prose in a docstring cannot satisfy. And comments go before the closing bracket
        is looked for, so a bracket written inside one cannot end the list early.

        A declaration this does not understand yields fewer names, never different ones,
        because only quoted words between the brackets are read. `test_api_surface.py`
        checks the map against what the modules really export, so a declaration that
        outgrows this fails the test suite rather than `nest` itself.
        """

        import re

        opening = re.search(r"^__all__\s*=\s*\[", source, re.MULTILINE)
        if opening is None:
            return []

        # An exported name holds no `#`, so every `#` that is left starts a comment.
        body = re.sub(r"#.*", "", source[opening.end() :])
        closing = body.find("]")
        if closing == -1:
            return []

        return re.findall(r"[\"'](\w+)[\"']", body[:closing])

    def set(self, **kwargs):
        "Forward kernel attribute setting to `SetKernelStatus()`."
        return self.SetKernelStatus(kwargs)

    def get(self, *args):
        "Forward kernel attribute getting to `GetKernelStatus()`."
        if not args:
            return self.GetKernelStatus()
        if len(args) == 1:
            return self.GetKernelStatus(args[0])
        return self.GetKernelStatus(args)

    def __dir__(self):
        # `__all__` first: building it caches it in the module dictionary, so that the
        # `vars(self)` below sees it.
        api = set(self.__all__)
        return list(api | {name for name in vars(self) if name not in NestModule._PRIVATE_SUBMODULES})

    def __getattr__(self, attr):
        """
        Resolve a name that `nest` exposes but has not imported yet: one of its
        submodules, or one of the names that a ``lib.hl_api_*`` module exports. The
        symbol map names the module that defines it, so only that one module is
        imported. The result is cached in the module dictionary, so this runs once per
        name.
        """
        import importlib

        if attr == "__all__":
            api = {name for name in vars(self) if not name.startswith("_")}
            api |= {name for name in dir(type(self)) if not name.startswith("_")}
            api |= self._submodules()
            api |= set(self._symbols())
            # `NestModule` is reachable as `nest.NestModule` for the documentation
            # build, but it is machinery rather than API.
            api -= NestModule._PRIVATE_SUBMODULES | {"NestModule"}
            self.__dict__["__all__"] = sorted(api)
            return self.__dict__["__all__"]

        if not attr.startswith("_"):
            if attr in self._submodules():
                module = importlib.import_module("." + attr, __name__)
                self.__dict__[attr] = module
                return module

            module = self._symbols().get(attr)
            if module is not None:
                value = getattr(importlib.import_module(module, __name__), attr)
                self.__dict__[attr] = value
                return value

        raise AttributeError(f"module {__name__!r} has no attribute {attr!r}")

    def __setattr__(self, attr, value):
        """
        Refuse to assign a name that no descriptor on `NestModule` claims: the module
        namespace is a curated API, not a scratch pad. Use `nest.userdict` to attach
        data of your own.

        Writing a kernel attribute needs nothing from this method. `object.__setattr__`
        finds the `KernelAttribute` on the type and calls its `__set__`, which is what
        carries the value to the kernel.
        """
        # Imported here to keep `types` out of the `nest` namespace.
        import types

        if isinstance(value, types.ModuleType):
            # The import machinery attaches submodules to their parent package.
            self.__dict__[attr] = value
            return

        descriptor = getattr(type(self), attr, None)
        if descriptor is None or not hasattr(descriptor, "__set__"):
            raise AttributeError(f"Cannot set attribute '{attr}' on module 'nest'")
        super().__setattr__(attr, value)

    userdict = {}
    """
    The variable userdict allows users to store custom data with the NEST kernel.

    Example: nest.userdict["nodes"] = [1,2,3,4]
    """


def _install_kernel_attributes(cls, annotations):
    """
    Turn the ``Annotated[<type>, KernelAttribute(...)]`` declarations above into
    descriptors on `cls`, and record their names for `SetKernelStatus()` to validate
    against.

    The annotation is the whole declaration of a kernel attribute: its type is what
    static analysis reports for ``nest.<name>`` and what the ``:type:`` field of the
    generated docstring shows, and its metadata carries description and defaults.
    """

    attributes = {}
    for name, annotation in annotations.items():
        metadata = getattr(annotation, "__metadata__", ())
        attribute = next((meta for meta in metadata if isinstance(meta, KernelAttribute)), None)  # noqa: F405
        if attribute is None:
            continue
        attribute.bind(name, typing.get_args(annotation)[0])  # noqa: F405
        setattr(cls, name, attribute)
        attributes[name] = attribute

    # Kernel attribute indices, used for fast lookup in `lib/hl_api_simulation.py`
    cls._kernel_attr_names = frozenset(attributes)
    cls._readonly_kernel_attrs = frozenset(name for name, a in attributes.items() if a.readonly)


_install_kernel_attributes(NestModule, __annotations__)

__version__ = _ll_api.nestkernel.llapi_get_kernel_status()["build_info"]["version"]

# Retype the module object, putting the kernel attribute descriptors in force.
sys.modules[__name__].__class__ = NestModule

# What is left in this file's namespace is the `nest` namespace, so drop the scaffolding
# this file needed to build it.
del sys, types, typing, Annotated, KernelAttribute, _ll_api, _install_kernel_attributes
