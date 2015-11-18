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

from .hl_api_helper import *

@check_stack
def Simulate(t):
    """
    Simulate the network for t milliseconds.
    """

    sps(float(t))
    sr('ms Simulate')


@check_stack
def ResumeSimulation():
    """
    Resume an interrupted simulation.
    """

    sr("ResumeSimulation")


@check_stack
def ResetKernel():
    """
    Reset the simulation kernel. This will destroy the network as
    well as all custom models created with CopyModel(). Calling this
    function is equivalent to restarting NEST.
    """

    sr('ResetKernel')


@check_stack
def ResetNetwork():
    """
    Reset all nodes and connections to their original state.
    """

    sr('ResetNetwork')


@check_stack
def SetKernelStatus(params):
    """
    Set parameters for the simulation kernel.
    """
    
    sps(0)
    sps(params)
    sr('SetStatus')


@check_stack
def GetKernelStatus(keys=None):
    """
    Obtain parameters of the simulation kernel.

    Returns:
    - Parameter dictionary if called without argument
    - Single parameter value if called with single parameter name
    - List of parameter values if called with list of parameter names
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
    """
    Load a dynamically linked NEST module.

    Example:
    nest.Install("mymodule")

    Returns:
    NEST module identifier, required for unloading.

    Note: Dynamically linked modules are searched in the
    LD_LIBRARY_PATH (DYLD_LIBRARY_PATH under OSX).
    """

    return sr("(%s) Install" % module_name)
