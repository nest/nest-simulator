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

import functools
import inspect
"""
Low-level API of PyNEST Module
"""

import sys
import os

# This is a workaround for readline import errors encountered with Anaconda
# Python running on Ubuntu, when invoked from the terminal
# "python -c 'import nest'"
if 'linux' in sys.platform and 'Anaconda' in sys.version:
    import readline

# This is a workaround to avoid segmentation faults when importing
# scipy *after* nest. See https://github.com/numpy/numpy/issues/2521
try:
    import scipy
except ImportError:
    pass

# Make MPI-enabled NEST import properly. The underlying problem is that the
# shared object pynestkernel dynamically opens other libraries that open
# yet other libraries.
sys.setdlopenflags(os.RTLD_NOW | os.RTLD_GLOBAL)

from . import pynestkernel as kernel      # noqa
from . import nestkernel_api as nestkernel      # noqa

__all__ = [
    'check_stack',
    'connect_arrays',
    'set_communicator',
    'take_array_index',
    # 'KernelAttribute', TODO-PYNEST-NG: Enable again when it works without SLI
]


engine = kernel.NESTEngine()

take_array_index = engine.take_array_index
connect_arrays = engine.connect_arrays


initialized = False


def check_stack(thing):  # # TODO-PYNEST-NG: remove
    return thing


def set_communicator(comm):
    """Set global communicator for NEST.

    Parameters
    ----------
    comm: MPI.Comm from mpi4py

    Raises
    ------
    _kernel.NESTError
    """

    if "mpi4py" not in sys.modules:
        raise _kernel.NESTError("set_communicator: "
                                "mpi4py not loaded.")

    engine.set_communicator(comm)


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
    kernel.NESTError.PyNESTError
    """

    global initialized

    if initialized:
        raise kernel.NESTErrors.PyNESTError("NEST already initialized.")

    # Some commandline arguments of NEST and Python have the same
    # name, but different meaning. To avoid unintended behavior, we
    # handle NEST's arguments here and pass it a modified copy, while
    # we leave the original list unchanged for further use by the user
    # or other modules.
    nest_argv = argv[:]

    quiet = "--quiet" in nest_argv or 'PYNEST_QUIET' in os.environ
    if "--quiet" in nest_argv:
        nest_argv.remove("--quiet")
    if "--debug" in nest_argv:
        nest_argv.remove("--debug")
    if "--sli-debug" in nest_argv:
        nest_argv.remove("--sli-debug")
        nest_argv.append("--debug")

    if 'PYNEST_DEBUG' in os.environ and '--debug' not in nest_argv:
        nest_argv.append("--debug")

    path = os.path.dirname(__file__)
    initialized = engine.init(nest_argv)

    if initialized:
        if not quiet:
            print('NEST initialized successfully!')  # TODO-PYNEST-NG: Implement welcome in Python
            # engine.run("pywelcome")

        # TODO-PYNEST-NG: Enable again when it works without SLI
        # Dirty hack to get tab-completion for models in IPython.
        # try:
        #     __IPYTHON__
        # except NameError:
        #     pass
        # else:
        #     try:
        #         import keyword
        #         from .lib.hl_api_models import Models		# noqa
        #         keyword.kwlist += Models()
        #     except ImportError:
        #         pass

    else:
        raise kernel.NESTErrors.PyNESTError("Initialization of NEST failed.")


init(sys.argv)
