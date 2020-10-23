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

# Python 3.3 and later has flags in os
sys.setdlopenflags(os.RTLD_NOW | os.RTLD_GLOBAL)

from . import pynestkernel as kernel      # noqa

__all__ = [
    'check_stack',
    'connect_arrays',
    'set_communicator',
    'get_debug',
    'set_debug',
    'sli_func',
    'sli_pop',
    'sli_push',
    'sli_run',
    'spp',
    'sps',
    'sr',
    'stack_checker',
    'take_array_index',
]


engine = kernel.NESTEngine()

sli_push = sps = engine.push
sli_pop = spp = engine.pop
take_array_index = engine.take_array_index
connect_arrays = engine.connect_arrays


def catching_sli_run(cmd):
    """Send a command string to the NEST kernel to be executed, catch
    SLI errors and re-raise them in Python.

    Parameters
    ----------
    cmd : str
        The SLI command to be executed.
    Raises
    ------
    kernel.NESTError
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

        exceptionCls = getattr(kernel.NESTErrors, errorname)
        raise exceptionCls(commandname, message)


sli_run = sr = catching_sli_run


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
    """

    # check for namespace
    slifun = 'sli_func'  # version not converting to literals
    if 'namespace' in kwargs:
        s = kwargs['namespace'] + ' using ' + s + ' endusing'
    elif 'litconv' in kwargs:
        if kwargs['litconv']:
            slifun = 'sli_func_litconv'
    elif len(kwargs) > 0:
        raise kernel.NESTErrors.PyNESTError(
            "'namespace' and 'litconv' are the only valid keyword arguments.")

    sli_push(args)       # push array of arguments on SLI stack
    sli_push(s)          # push command string
    sli_run(slifun)      # SLI support code to execute s on args
    r = sli_pop()        # return value is an array

    if len(r) == 1:      # 1 return value is no tuple
        return r[0]

    if len(r) != 0:
        return r


__debug = False


def get_debug():
    """Return the current value of the debug flag of the low-level API.

    Returns
    -------
    bool:
        current value of the debug flag
    """

    return __debug


def set_debug(dbg=True):
    """Set the debug flag of the low-level API.

    Parameters
    ----------
    dbg : bool, optional
        Value to set the debug flag to
    """

    global __debug
    __debug = dbg


def stack_checker(f):
    """Decorator to add stack checks to functions using PyNEST's
    low-level API.

    This decorator works only on functions. See
    check_stack() for the generic version for functions and
    classes.

    Parameters
    ----------
    f : function
        Function to decorate

    Returns
    -------
    function:
        Decorated function

    Raises
    ------
    kernel.NESTError
    """

    @functools.wraps(f)
    def stack_checker_func(*args, **kwargs):
        if not get_debug():
            return f(*args, **kwargs)
        else:
            sr('count')
            stackload_before = spp()
            result = f(*args, **kwargs)
            sr('count')
            num_leftover_elements = spp() - stackload_before
            if num_leftover_elements != 0:
                eargs = (f.__name__, num_leftover_elements)
                etext = "Function '%s' left %i elements on the stack."
                raise kernel.NESTError(etext % eargs)
            return result

    return stack_checker_func


def check_stack(thing):
    """Convenience wrapper for applying the stack_checker decorator to
    all class methods of the given class, or to a given function.

    If the object cannot be decorated, it is returned unchanged.

    Parameters
    ----------
    thing : function or class
        Description

    Returns
    -------
    function or class
        Decorated function or class

    Raises
    ------
    ValueError
    """

    if inspect.isfunction(thing):
        return stack_checker(thing)
    elif inspect.isclass(thing):
        for name, mtd in inspect.getmembers(thing, predicate=inspect.ismethod):
            if name.startswith("test_"):
                setattr(thing, name, stack_checker(mtd))
        return thing
    else:
        raise ValueError("unable to decorate {0}".format(thing))


initialized = False


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
    initialized = engine.init(nest_argv, path)

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
                from .lib.hl_api_models import Models		# noqa
                keyword.kwlist += Models()
            except ImportError:
                pass

    else:
        raise kernel.NESTErrors.PyNESTError("Initialization of NEST failed.")


init(sys.argv)
