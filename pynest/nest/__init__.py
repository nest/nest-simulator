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

from . import pynestkernel as _kernel      # noqa
from .lib import hl_api_helper as hl_api   # noqa

engine = _kernel.NESTEngine()

sli_push = hl_api.sps = engine.push
sli_pop = hl_api.spp = engine.pop
hl_api.pcd = engine.push_connection_datums
hl_api.kernel = _kernel

initialized = False


def catching_sli_run(cmd):
    """Send a command string to the NEST kernel to be executed, catch
    SLI errors and re-raise them in Python.

    Parameters
    ----------
    cmd : str
        The SLI command to be executed.
    Raises
    ------
    NESTError
        SLI errors are bubbled to the Python API as NESTErrors.
    """

    if sys.version_info >= (3, ):
        def encode(s):
            return s

        def decode(s):
            return s
    else:
        def encode(s):
            return s.encode('utf-8')

        def decode(s):
            return s.decode('utf-8')

    engine.run('{%s} runprotected' % decode(cmd))
    if not sli_pop():
        errorname = sli_pop()
        message = sli_pop()
        commandname = sli_pop()
        engine.run('clear')

        exceptionCls = getattr(_kernel.NESTErrors, errorname)
        raise exceptionCls(commandname, message)

sli_run = hl_api.sr = catching_sli_run


def sli_func(s, *args, **kwargs):
    """Convenience function for executing an SLI command s with
    arguments args.

    This executes the SLI sequence:
    ``sli_push(args); sli_run(s); y=sli_pop()``

    Parameters
    ----------
    s : str
        Function to call
    *args
        Arbitrary number of arguments to pass to the SLI function
    **kwargs
        namespace : str
            The sli code is executed in the given SLI namespace.
        litconv : bool
            Convert string args beginning with / to literals.

    Returns
    -------
    The function may have multiple return values. The number of return values
    is determined by the SLI function that was called.

    Examples
    --------
    r,q = sli_func('dup rollu add',2,3)
    r   = sli_func('add',2,3)
    r   = sli_func('add pop',2,3)
    l   = sli_func('CreateLayer', {...}, namespace='topology')
    """

    # check for namespace
    slifun = 'sli_func'  # version not converting to literals
    if 'namespace' in kwargs:
        s = kwargs['namespace'] + ' using ' + s + ' endusing'
    elif 'litconv' in kwargs:
        if kwargs['litconv']:
            slifun = 'sli_func_litconv'
    elif len(kwargs) > 0:
        raise _kernel.NESTErrors.PyNESTError(
            "'namespace' and 'litconv' are the only valid keyword arguments.")

    sli_push(args)       # push array of arguments on SLI stack
    sli_push(s)          # push command string
    sli_run(slifun)      # SLI support code to execute s on args
    r = sli_pop()        # return value is an array

    if len(r) == 1:      # 1 return value is no tuple
        return r[0]

    if len(r) != 0:
        return r

hl_api.sli_func = sli_func


def init(argv):
    """Initializes NEST.

    Parameters
    ----------
    argv : list
        Command line arguments, passed to the NEST kernel

    Raises
    ------
    _kernel.NESTError.PyNESTError
    """

    global initialized

    if initialized:
        raise _kernel.NESTErrors.PyNESTError("NEST already initialized.")

    quiet = False

    # Some commandline arguments of NEST and Python have the same
    # name, but different meaning. To avoid unintended behavior, we
    # handle NEST's arguments here and pass it a modified copy, while
    # we leave the original list unchanged for further use by the user
    # or other modules.
    nest_argv = argv[:]

    if "--quiet" in nest_argv:
        quiet = True
        nest_argv.remove("--quiet")
    if "--debug" in nest_argv:
        nest_argv.remove("--debug")
    if "--sli-debug" in nest_argv:
        nest_argv.remove("--sli-debug")
        nest_argv.append("--debug")

    initialized = engine.init(nest_argv, __path__[0])

    if initialized:
        if not quiet:
            engine.run("pywelcome")

        # Dirty hack to get tab-completion for models in IPython.
        try:
            __IPYTHON__
        except NameError:
            pass
        else:
            try:
                import keyword
                keyword.kwlist += Models()
            except ImportError:
                pass

    else:
        raise _kernel.NESTErrors.PyNESTError("Initiatization of NEST failed.")


def test():
    """Runs all PyNEST unit tests."""
    from . import tests
    import unittest

    debug = hl_api.get_debug()
    hl_api.set_debug(True)

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(tests.suite())

    hl_api.set_debug(debug)

from .pynestkernel import *         # noqa
from .lib.hl_api_helper import *    # noqa

# We search through the subdirectory "lib" of the "nest" module
# directory and import the content of all Python files therein into
# the global namespace. This makes the API functions of PyNEST itself
# and those of extra modules available to the user.
for name in os.listdir(os.path.join(os.path.dirname(__file__), "lib")):
    if name.endswith(".py") and not name.startswith('__'):
        exec("from .lib.{0} import *".format(name[:-3]))

if 'DELAY_PYNEST_INIT' not in os.environ:
    init(sys.argv)
