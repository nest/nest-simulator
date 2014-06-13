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

import sys, os

# This is a workaround to avoid segmentation faults when importing
# scipy after nest. See https://github.com/numpy/numpy/issues/2521
# and 
try:
    import scipy
except:
    pass


# The following is a workaround to make MPI-enabled NEST import
# properly. The basic problem is that the shared object pynestkernel
# dynamically opens other libraries that open other libraries...
try:
    try:
        import dl
    except:
        import DLFCN as dl
    sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)
except:
    # this is a hack for Python 2.6 on Mac, where RTDL_NOW is nowhere to
    # be found. See ticket #397
    import ctypes
    sys.setdlopenflags(ctypes.RTLD_GLOBAL) 

from . import pynestkernel as _kernel
from . import hl_api

engine = _kernel.NESTEngine()

sli_push = engine.push
hl_api.sps = sli_push

sli_pop = engine.pop
hl_api.spp = sli_pop

hl_api.pcd = engine.push_connection_datums

initialized = False

def catching_sli_run(cmd):
    """
    Send a command string to the NEST kernel to be executed. This
    function is is a wrapper around _kernel.runsli to raise errors that
    happen on the SLI level as Python errors. cmd is the command to be
    executed.
    """

    engine.run('{%s} runprotected' % cmd)
    if not sli_pop():
        errorname = sli_pop()
        message = sli_pop()
        commandname = sli_pop()
        engine.run('clear')
        raise hl_api.NESTError("{0} in {1}{2}".format(errorname, commandname, message))


sli_run = catching_sli_run
hl_api.sr = sli_run


def sli_func(s, *args, **kwargs):
    """This function is a convenience function for executing the 
       sequence sli_push(args); sli_run(s); y=sli_pop(). It takes
       an arbitrary number of arguments and may have multiple
       return values. The number of return values is determined by
       the SLI function that was called.

       Keyword arguments:
       namespace - string: The sli code is executed in the given SLI namespace.
       litconv   - bool  : Convert string args beginning with / to literals.
       
       Examples:
         r,q = sli_func('dup rollu add',2,3)
         r   = sli_func('add',2,3)
         r   = sli_func('add pop',2,3)
         l   = sli_func('CreateLayer', {...}, namespace='topology')
         opt = sli_func('GetOptions', '/RandomConvergentConnect', litconv=True)
    """

    # check for namespace
    slifun = 'sli_func'  # version not converting to literals
    if 'namespace' in kwargs:
        s = kwargs['namespace'] + ' using ' + s + ' endusing'
    elif 'litconv' in kwargs:
        if kwargs['litconv']:
            slifun = 'sli_func_litconv'
    elif len(kwargs) > 0:
        hl_api.NESTError("'namespace' and 'litconv' are the only valid keyword arguments.")
    
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
    """Initialize. argv is passed to the NEST kernel."""

    global initialized

    if initialized:
        raise hl_api.NESTError("NEST already initialized.")
        return

    quiet = False
    if argv.count("--quiet"):
        quiet = True
        argv.remove("--quiet")

    initialized |= engine.init(argv, __path__[0])

    if initialized:
        if not quiet :
            engine.run("pywelcome")

        # Dirty hack to get tab-completion for models in IPython.
        try:
            __IPYTHON__
        except NameError:
            pass
        else:
            try:
                import keyword
                keyword.kwlist += hl_api.Models()
            except ImportError:
                pass

    else:
        hl_api.NESTError("Initiatization of NEST failed.")


def test():
    """ Runs a battery of unit tests on PyNEST """
    from . import tests
    import unittest

    debug = hl_api.get_debug()
    hl_api.set_debug(True)

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(tests.suite())

    hl_api.set_debug(debug)


if not 'DELAY_PYNEST_INIT' in os.environ:
    init(sys.argv)

from .hl_api import *
