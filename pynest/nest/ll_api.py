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
import atexit
import inspect
import keyword
import os
import sys
import typing

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

    # TODO-PYNEST-NG: set_communicator — who needs this?
    # engine.set_communicator(comm)


class KernelAttribute:
    """
    Descriptor that maps an attribute of the ``nest`` module onto an entry of the
    kernel status dictionary.

    Reading the attribute reads the kernel status, assigning to it writes the kernel
    status. Kernel attributes are declared as the metadata of a module-level
    annotation in ``nest/__init__.py``::

        resolution: Annotated[float, KernelAttribute("The resolution of the simulation (in ms)", default=0.1)]

    The annotated type is both what static analysis reports for ``nest.resolution``
    and what `bind` renders as the ``:type:`` field of the docstring, so it is stated
    once. `bind` turns such a declaration into a descriptor on `nest.NestModule`.

    Parameters
    ----------
    description : str
        What the attribute means, as one reStructuredText paragraph. It is what
        ``help()`` and the :ref:`sec_kernel_attributes` page show. Indentation is
        stripped, so a triple-quoted string indented to match the surrounding code
        renders correctly. A trailing period is optional; one is always rendered.
    readonly : bool, optional
        Whether assigning to the attribute raises an `AttributeError`.
    default : optional
        Value of the attribute in a freshly reset kernel. Documented only; the kernel,
        not this descriptor, applies it.
    localonly : bool, optional
        Whether the value describes the local MPI rank rather than the whole
        simulation.
    """

    def __init__(self, description, readonly=False, default=None, localonly=False):
        self.description = description
        self.readonly = readonly
        self.localonly = localonly
        self.default = default

    def bind(self, name, typehint):
        """Attach this descriptor to the kernel status entry `name`, of type `typehint`."""

        self._name = name
        # `list[str].__name__` is "list", so subscripted generics have to be rendered
        # through `str()` to keep their parameters.
        self.typehint = str(typehint) if typing.get_origin(typehint) else typehint.__name__
        self.__doc__ = self._build_docstring()

    def _build_docstring(self):
        summary = inspect.cleandoc(self.description).rstrip()
        if summary.endswith("."):
            summary = summary[:-1]
        if self.default is not None:
            summary += f", defaults to ``{self.default}``"
        scope = ", ".join(
            label
            for label, applies in (("**read only**", self.readonly), ("**local only**", self.localonly))
            if applies
        )
        return "\n\n".join(block for block in (summary + ".", scope, f":type: {self.typehint}") if block)

    def __get__(self, instance, cls=None):
        if instance is None:
            return self
        status = nestkernel.llapi_get_kernel_status()
        # `kernel_status` exposes the status dictionary itself, not an entry of it.
        return status if self._name == "kernel_status" else status[self._name]

    def __set__(self, instance, value):
        if self.readonly:
            raise AttributeError(f"`{self._name}` is a read only kernel attribute.")
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

    nestkernel.init(nest_argv)
    initialized = True

    if not quiet:
        build_info = nestkernel.llapi_get_kernel_status()["build_info"]
        print(f"""
             -- N E S T --

 Copyright (C) 2004 The NEST Initiative

 Version: {build_info["version"]}
 Built  : {build_info["built"]}

 This program is provided AS IS and comes with NO WARRANTY.
 See the file LICENSE for details.

 Problems or suggestions?
   Visit https://www.nest-simulator.org

 Type 'nest.help()' to find out more about NEST.
""")

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


@atexit.register
def shutdown():
    """Ensures that MPI_Finalize() is called if needed."""
    nestkernel.llapi_shutdown_nest(0)


init(sys.argv)
