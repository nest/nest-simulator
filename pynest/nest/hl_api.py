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

**Internal use only**, do not import from this module or its submodules in your
code.

This module imports all parts of the public API of the root ``nest`` module,
both static and dynamic submodules. During initialization of the ``nest`` module
all public attributes of `nest.hl_api` are copied into the ``nest`` module.
"""

from . import import_libs as _il

#############################
# insert static imports here
# in the form:
# from lib.$X import * ; _ignore_modules.add('lib.$X')
############################
_ignore_modules = set()
from .lib.hl_api_connection_helpers import *
_ignore_modules.add('lib.hl_api_connection_helpers')
from .lib.hl_api_connections import *
_ignore_modules.add('lib.hl_api_connections')
from .lib.hl_api_exceptions import *
_ignore_modules.add('lib.hl_api_exceptions')
from .lib.hl_api_helper import *
_ignore_modules.add('lib.hl_api_helper')
from .lib.hl_api_info import *
_ignore_modules.add('lib.hl_api_info')
from .lib.hl_api_models import *
_ignore_modules.add('lib.hl_api_models')
from .lib.hl_api_nodes import *
_ignore_modules.add('lib.hl_api_nodes')
from .lib.hl_api_parallel_computing import *
_ignore_modules.add('lib.hl_api_parallel_computing')
from .lib.hl_api_simulation import *
_ignore_modules.add('lib.hl_api_simulation')
from .lib.hl_api_spatial import *
_ignore_modules.add('lib.hl_api_spatial')
from .lib.hl_api_types import *
_ignore_modules.add('lib.hl_api_types')

############################
# Then whatever is left over, load dynamically
# then do
#   `from .libs.$X import *`
# for every module in ./libs/$X.py that is left
_il.import_libs(__file__, globals(), 'lib', ignore=_ignore_modules)

############################

# With '__all__' we provide an explicit index of the package. Without any
# imported submodules and any redundant functions we could minimize list.
__all__ = [
    'Cleanup',
    'CollocatedSynapses',
    'Connect',
    'ConnectionRules',
    'SynapseCollection',
    'CopyModel',
    'Create',
    'CreateMask',
    'CreateParameter',
    'DisableStructuralPlasticity',
    'Disconnect',
    'Displacement',
    'Distance',
    'DumpLayerConnections',
    'DumpLayerNodes',
    'EnableStructuralPlasticity',
    'FindCenterElement',
    'FindNearestElement',
    'GetConnections',
    'GetDefaults',
    'GetKernelStatus',
    'GetLocalNodeCollection',
    'GetLocalVPs',
    'GetNodes',
    'GetPosition',
    'GetStatus',
    'GetTargetNodes',
    'GetTargetPositions',
    'NodeCollection',
    'Install',
    'Mask',
    'Models',
    'NumProcesses',
    'Parameter',
    'PlotLayer',
    'PlotProbabilityParameter',
    'PlotTargets',
    'Prepare',
    'PrintNodes',
    'Rank',
    'ResetKernel',
    'Run',
    'RunManager',
    'SelectNodesByMask',
    'SetAcceptableLatency',
    'SetDefaults',
    'SetKernelStatus',
    'SetMaxBuffered',
    'SetStatus',
    'Simulate',
    'authors',
    'get_verbosity',
    'help',
    'helpdesk',
    'message',
    'set_verbosity',
    'sysinfo',
]
