# -*- coding: utf-8 -*-
#
# hl_api.py
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
High-level API of PyNEST Module
"""

import os

# We search through the subdirectory "lib" of the "nest" module
# directory and import the content of all Python files therein into
# the global namespace. This makes the API functions of PyNEST itself
# and those of extra modules available to the user.
for name in os.listdir(os.path.join(os.path.dirname(__file__), "lib")):
    if name.endswith(".py") and not name.startswith('__'):
        exec("from .lib.{0} import *".format(name[:-3]))
from .random.hl_api_random import *

# With '__all__' we provide an explicit index of the package. Without any
# imported submodules and any redundant functions we could minimize list.
__all__ = [
    'CGConnect',
    'CGParse',
    'CGSelectImplementation',
    'Cleanup',
    'Connect',
    'ConnectionRules',
    'Connectome',
    'CopyModel',
    'Create',
    'DataConnect',  # deprecated
    'DisableStructuralPlasticity',
    'Disconnect',
    'EnableStructuralPlasticity',
    'GetConnections',
    'GetDefaults',
    'GetKernelStatus',
    'GetStatus',
    'GetStructuralPlasticityStatus',
    'GIDCollection',
    'Install',
    'Mask',
    'Models',
    'NumProcesses',
    'Parameter',
    'Prepare',
    'Rank',
    'ResetKernel',
    'ResetNetwork',
    'Run',
    'RunManager',
    'SetAcceptableLatency',
    'SetDefaults',
    'SetKernelStatus',
    'SetMaxBuffered',
    'SetStatus',
    'SetStructuralPlasticityStatus',
    'Simulate',
    'authors',
    'help',
    'helpdesk',
    'sysinfo',
    'version',
]
