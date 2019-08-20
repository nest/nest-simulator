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

from contextlib import contextmanager

from ..ll_api import *
from .hl_api_helper import *

__all__ = [
    'Cleanup',
    'DisableStructuralPlasticity',
    'EnableStructuralPlasticity',
    'GetKernelStatus',
    'GetStructuralPlasticityStatus',
    'Install',
    'Prepare',
    'ResetKernel',
    'Run',
    'RunManager',
    'SetKernelStatus',
    'SetStructuralPlasticityStatus',
    'Simulate',
]


@check_stack
def Simulate(t):
    """Simulate the network for `t` milliseconds.

    Parameters
    ----------
    t : float
        Time to simulate in ms

    See Also
    --------
    RunManager, ResumeSimulation

    KEYWORDS:
    """

    sps(float(t))
    sr('ms Simulate')


@check_stack
def Run(t):
    """Simulate the network for `t` milliseconds.

    Parameters
    ----------
    t : float
        Time to simulate in ms

    Call between `Prepare` and `Cleanup` calls, or within a
    ``with RunManager`` clause.

    Simulate(t): t' = t/m; Prepare(); for _ in range(m): Run(t'); Cleanup()

    `Prepare` must be called before `Run` to calibrate the system, and
    `Cleanup` must be called after `Run` to close files, cleanup handles, and
    so on. After `Cleanup`, `Prepare` can and must be called before more `Run`
    calls. Any calls to `SetStatus` between `Prepare` and `Cleanup` have
    undefined behaviour.

    See Also
    --------
    Prepare, Cleanup, RunManager, Simulate

    KEYWORDS:
    """

    sps(float(t))
    sr('ms Run')


@check_stack
def Prepare():
    """Calibrate the system before a `Run` call. Not needed for `Simulate`.

    Call before the first `Run` call, or before calling `Run` after changing
    the system, calling `SetStatus` or `Cleanup`.

    See Also
    --------
    Run, Cleanup

    KEYWORDS:
    """

    sr('Prepare')


@check_stack
def Cleanup():
    """Cleans up resources after a `Run` call. Not needed for `Simulate`.

    Closes state for a series of runs, such as flushing and closing files.
    A `Prepare` is needed after a `Cleanup` before any more calls to `Run`.

    See Also
    --------
    Run, Prepare

    KEYWORDS:
    """
    sr('Cleanup')


@contextmanager
def RunManager():
    """ContextManager for `Run`

    Calls `Prepare` before a series of `Run` calls, and calls `Cleanup` at end.

    E.g.:

    .. code-block:: python

        with RunManager():
            for i in range(10):
                Run()

    See Also
    --------
    Prepare, Run, Cleanup, Simulate

    KEYWORDS:
    """

    Prepare()
    try:
        yield
    finally:
        Cleanup()


@check_stack
def ResetKernel():
    """Reset the simulation kernel.

    This will destroy the network as well as all custom models created with
    CopyModel(). Calling this function is equivalent to restarting NEST.

    In particular,
    * all network nodes
    * all connections
    * all user-defined neuron and synapse models
    are deleted, and
    * time
    * random generators
    are reset. The only exception is that dynamically loaded modules are not
    unloaded. This may change in a future version of NEST.

    KEYWORDS:
   """

    sr('ResetKernel')


@check_stack
def SetKernelStatus(params):
    """Set parameters for the simulation kernel.

    Parameters
    ----------
    params : dict
        Dictionary of parameters to set.

    See Also
    --------
    GetKernelStatus

    KEYWORDS:
    """

    sps(params)
    sr('SetKernelStatus')


@check_stack
def GetKernelStatus(keys=None):
    """Obtain parameters of the simulation kernel.

    Parameters
    ----------
    keys : str or list, optional
        Single parameter name or ``list`` of parameter names

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
        If `keys` are of the wrong type.

    See Also
    --------
    SetKernelStatus

    KEYWORDS:
    """

    sr('GetKernelStatus')
    status_root = spp()

    if keys is None:
        return status_root
    elif is_literal(keys):
        return status_root[keys]
    elif is_iterable(keys):
        return tuple(status_root[k] for k in keys)
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
    handle
        NEST module identifier, required for unloading

    Notes
    -----
    Dynamically linked modules are searched in the ``LD_LIBRARY_PATH``
    (``DYLD_LIBRARY_PATH`` under OSX).

    **Example**

    .. code-block: python

        nest.Install("mymodule")

    KEYWORDS:
    """

    return sr("(%s) Install" % module_name)


@check_stack
def SetStructuralPlasticityStatus(params):
    """Set structural plasticity parameters for the network simulation.

    Parameters
    ----------
    params : dict
        Dictionary of structural plasticity parameters to set

    See Also
    --------
    GetStructuralPlasticityStatus

    KEYWORDS:
    """

    sps(params)
    sr('SetStructuralPlasticityStatus')


@check_stack
def GetStructuralPlasticityStatus(keys=None):
    """Get the current structural plasticity parameters

    Parameters
    ---------
    keys : str or list, optional
        Keys indicating the values of interest to be retrieved by the get call

    See Also
    --------
    SetStructuralPlasticityStatus

    KEYWORDS:
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

    See Also
    --------
    DisableStructuralPlasticity

    KEYWORDS:
    """

    sr('EnableStructuralPlasticity')


@check_stack
def DisableStructuralPlasticity():
    """Disable structural plasticity for the network simulation

    See Also
    --------
    EnableStructuralPlasticity

    KEYWORDS:
    """
    sr('DisableStructuralPlasticity')
