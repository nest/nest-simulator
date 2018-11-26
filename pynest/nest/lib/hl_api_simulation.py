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

from ..ll_api import *
from .hl_api_helper import *
from contextlib import contextmanager

__all__ = [
    'Simulate',
    'Run',
    'Prepare',
    'Cleanup',
    'RunManager',
    'ResumeSimulation',
    'ResetKernel',
    'ResetNetwork',
    'SetKernelStatus',
    'GetKernelStatus',
    'Install',
    'SetStructuralPlasticityStatus',
    'GetStructuralPlasticityStatus',
    'EnableStructuralPlasticity',
    'DisableStructuralPlasticity',
]


@check_stack
def Simulate(t):
    """Simulate the network for t milliseconds.

    Parameters
    ----------
    t : float
        Time to simulate in ms
    """

    sps(float(t))
    sr('ms Simulate')


@check_stack
def Run(t):
    """Simulate the network for t milliseconds.

    Parameters
    ----------
    t : float
        Time to simulate in ms

    Call between Prepare and Cleanup calls, or within an
    with RunManager: clause
    Simulate(t): t' = t/m; Prepare(); for _ in range(m): Run(t'); Cleanup()
    Prepare() must be called before to calibrate, etc; Cleanup() afterward to
    close files, cleanup handles and so on. After Cleanup(), Prepare() can and
    must be called before more Run() calls.
    Any calls to set_status between Prepare() and Cleanup() have undefined
    behavior.
    """

    sps(float(t))
    sr('ms Run')


@check_stack
def Prepare():
    """Prepares network before a Run call. Not needed for Simulate.

    See Run(t), Cleanup(). Call before any sequence of Runs(). Do all
    set_status calls before Prepare().
    """

    sr('Prepare')


@check_stack
def Cleanup():
    """Cleans up resources after a Run call. Not needed for Simulate.

    See Run(t), Prepare().
    Closes state for a series of runs, such as flushing and closing files.
    A Prepare() is needed after a Cleanup() before any more calls to Run().
    """
    sr('Cleanup')


@contextmanager
def RunManager():
    """ContextManager for Run.

    Calls Prepare() before a series of Run() calls,
    and  adds a Cleanup() at end.

    So:
    with RunManager():
        for i in range(10):
            Run()
    """

    Prepare()
    try:
        yield
    finally:
        Cleanup()


@check_stack
def ResumeSimulation():
    """Resume an interrupted simulation.
    """

    sr("ResumeSimulation")


@check_stack
def ResetKernel():
    """Reset the simulation kernel.

    This will destroy the network as well as all custom models created with
    CopyModel(). Calling this function is equivalent to restarting NEST.
    """

    sr('ResetKernel')


@check_stack
def ResetNetwork():
    """Reset all nodes and connections to their original state.
    """

    sr('ResetNetwork')


@check_stack
def SetKernelStatus(params):
    """Set parameters for the simulation kernel.

    Parameters
    ----------
    params : dict
        Dictionary of parameters to set.

    See also
    --------
    GetKernelStatus
    """

    sps(0)
    sps(params)
    sr('SetStatus')


@check_stack
def GetKernelStatus(keys=None):
    """Obtain parameters of the simulation kernel.

    Parameters
    ----------
    keys : str or list, optional
        Single parameter name or list of parameter names

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
    """

    sr('0 GetStatus')
    status_root = spp()

    sr('/subnet GetDefaults')
    status_subnet = spp()

    d = dict((k, v) for k, v in status_root.items() if k not in status_subnet)

    if keys is None:
        return d
    elif is_literal(keys):
        return d[keys]
    elif is_iterable(keys):
        return tuple(d[k] for k in keys)
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
    NEST module identifier, required for unloading.

    Example
    -------
    nest.Install("mymodule")

    Notes
    -----
    Dynamically linked modules are searched in the LD_LIBRARY_PATH
    (DYLD_LIBRARY_PATH under OSX).
    """

    return sr("(%s) Install" % module_name)


@check_stack
def SetStructuralPlasticityStatus(params):
    """Set structural plasticity parameters for the network simulation.

    Parameters
    ----------
    params : dict
        Dictionary of structural plasticity parameters to set
    """

    sps(params)
    sr('SetStructuralPlasticityStatus')


@check_stack
def GetStructuralPlasticityStatus(keys=None):
    """Get the current structural plasticity parameters for the network
    simulation.

    Parameters
    ---------
    keys : str or list, optional
        Keys indicating the values of interest to be retrieved by the get call
    """

    sps({})
    sr('GetStructuralPlasticityStatus')
    d = spp()
    if keys is None:
        return d
    elif is_literal(keys):
        return d[keys]
    elif is_iterable(keys):
        return tuple(d[k] for k in keys)
    else:
        raise TypeError("keys must be either empty, a string or a list")


@check_stack
def EnableStructuralPlasticity():
    """Enable structural plasticity for the network simulation
    """

    sr('EnableStructuralPlasticity')


@check_stack
def DisableStructuralPlasticity():
    """Disable structural plasticity for the network simulation
    """
    sr('DisableStructuralPlasticity')
