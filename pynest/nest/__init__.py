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

"""
Initializer of PyNEST.
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
except:
    pass

# Make MPI-enabled NEST import properly. The underlying problem is that the
# shared object pynestkernel dynamically opens other libraries that open
# yet other libraries.
try:
    # Python 3.3 and later has flags in os
    sys.setdlopenflags(os.RTLD_NOW | os.RTLD_GLOBAL)
except AttributeError:
    # Python 2.6 and 2.7 have flags in ctypes, but RTLD_NOW may only
    # be available in dl or DLFCN and is required at least under
    # Ubuntu 14.04. The latter two are not available under OSX,
    # but OSX does not have and does not need RTLD_NOW. We therefore
    # first try dl and DLFCN, then ctypes just for OSX.
    try:
        import dl
        sys.setdlopenflags(dl.RTLD_GLOBAL | dl.RTLD_NOW)
    except (ImportError, AttributeError):
        try:
            import DLFCN
            sys.setdlopenflags(DLFCN.RTLD_GLOBAL | DLFCN.RTLD_NOW)
        except (ImportError, AttributeError):
            import ctypes
            try:
                sys.setdlopenflags(ctypes.RTLD_GLOBAL | ctypes.RTLD_NOW)
            except AttributeError:
                # We must test this last, since it is the only case without
                # RTLD_NOW (OSX)
                sys.setdlopenflags(ctypes.RTLD_GLOBAL)

from . import pynestkernel as _kernel

engine = _kernel.NESTEngine()
initialized = False

def init(argv):
    """Initializes NEST.

    Parameters
    ----------
    argv : list
        Command line arguments, passed to the NEST kernel

    Raises
    ------
    _kernel.NESTError
    """

    global initialized

    if initialized:
        raise _kernel.NESTError("NEST already initialized.")
        return

    initialized = engine.init(argv)

    if initialized:

        # Dirty hack to get tab-completion for models in IPython.
        try:
            __IPYTHON__
        except NameError:
            pass
        else:
            try:
                import keyword
 #               keyword.kwlist += Models()
            except ImportError:
                pass

    else:
        _kernel.NESTError("Initiatization of NEST failed.")


def test():
    """Runs all PyNEST unit tests."""
    from . import tests
    import unittest

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(tests.suite())

from .pynestkernel import *         # noqa
from .lib.hl_api_helper import *    # noqa

# We search through the subdirectory "lib" of the "nest" module
# directory and import the content of all Python files therein into
# the global namespace. This makes the API functions of PyNEST itself
# and those of extra modules available to the user.
#for name in os.listdir(os.path.join(os.path.dirname(__file__), "lib")):
#    if name.endswith(".py") and not name.startswith('__'):
#        exec("from .lib.{0} import *".format(name[:-3]))

if 'DELAY_PYNEST_INIT' not in os.environ:
    init(sys.argv)
