# -*- coding: utf-8 -*-
#
# ll_api.py
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
Low-level API of PyNEST Module
"""

# Since this is a low level module, we need some more trickery, thus:
# pylint: disable=wrong-import-position
import keyword
import os
import sys

# This is a workaround to avoid segmentation faults when importing
# scipy *after* nest. See https://github.com/numpy/numpy/issues/2521
try:
    import scipy  # noqa: F401
except ImportError:
    pass

# Make MPI-enabled NEST import properly. The underlying problem is that the
# shared object pynestkernel dynamically opens other libraries that open
# yet other libraries.
sys.setdlopenflags(os.RTLD_NOW | os.RTLD_GLOBAL)

from . import nestkernel_api as nestkernel  # noqa

__all__ = [
    "set_communicator",
    # 'take_array_index',
    "KernelAttribute",
]


initialized = False


def set_communicator(comm):
    """Set global communicator for NEST.

    Parameters
    ----------
    comm: MPI.Comm from mpi4py

    Raises
    ------
    ModuleNotFoundError
    """

    if "mpi4py" not in sys.modules:
        raise ModuleNotFoundError("No module named 'mpi4py'.")

    # TODO-PYNEST-NG: set_communicator
    # engine.set_communicator(comm)


class KernelAttribute:
    """
    Descriptor that dispatches attribute access to the nest kernel.
    """

    def __init__(self, typehint, description, readonly=False, default=None, localonly=False):
        self._readonly = readonly
        self._localonly = localonly
        self._default = default

        readonly = readonly and "**read only**"
        localonly = localonly and "**local only**"

        self.__doc__ = (
            description
            + ("." if default is None else f", defaults to ``{default}``.")
            + ("\n\n" if readonly or localonly else "")
            + ", ".join(c for c in (readonly, localonly) if c)
            + f"\n\n:type: {typehint}"
        )

    def __set_name__(self, cls, name):
        self._name = name
        self._full_status = name == "kernel_status"

    def __get__(self, instance, cls=None):
        if instance is None:
            return self

        status_root = nestkernel.llapi_get_kernel_status()

        if self._full_status:
            return status_root
        else:
            return status_root[self._name]

    def __set__(self, instance, value):
        if self._readonly:
            msg = f"`{self._name}` is a read only kernel attribute."
            raise AttributeError(msg)
        nestkernel.llapi_set_kernel_status({self._name: value})


def init(argv):
    """Initializes NEST.

    If the environment variable PYNEST_QUIET is set, NEST will not print
    welcome text containing the version and other information. Likewise,
    if the environment variable PYNEST_DEBUG is set, NEST starts in debug
    mode. Note that the same effect can be achieved by using the
    commandline arguments --quiet and --debug respectively.

    Parameters
    ----------
    argv : list
        Command line arguments, passed to the NEST kernel

    Raises
    ------
    RuntimeError
    """

    global initialized

    if initialized:
        raise RuntimeError("NEST is already initialized.")

    # Some commandline arguments of NEST and Python have the same
    # name, but different meaning. To avoid unintended behavior, we
    # handle NEST's arguments here and pass it a modified copy, while
    # we leave the original list unchanged for further use by the user
    # or other modules.
    nest_argv = argv[:]

    quiet = "--quiet" in nest_argv or "PYNEST_QUIET" in os.environ
    if "--quiet" in nest_argv:
        nest_argv.remove("--quiet")
    if "--debug" in nest_argv:
        nest_argv.remove("--debug")

    if "PYNEST_DEBUG" in os.environ and "--debug" not in nest_argv:
        nest_argv.append("--debug")

    path = os.path.dirname(__file__)
    nestkernel.init(nest_argv)
    initialized = True

    if not quiet:
        print("NEST initialized successfully!")

    # Dirty hack to get tab-completion for models in IPython.
    try:
        __IPYTHON__
    except NameError:
        pass
    else:
        from .lib.hl_api_simulation import GetKernelStatus  # noqa

        keyword_lists = (
            "connection_rules",
            "node_models",
            "recording_backends",
            "rng_types",
            "stimulation_backends",
            "synapse_models",
        )
        for kwl in keyword_lists:
            keyword.kwlist += GetKernelStatus(kwl)


init(sys.argv)
