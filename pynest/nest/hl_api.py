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

import .import_libs as _il

#############################
# insert static imports here
# in the form:
# from lib.$X import * ; _ignore_modules.add('lib.$X')
############################
_ignore_modules = set()

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
    'BeginSubnet',  # deprecated
    'CGConnect',
    'CGParse',
    'CGSelectImplementation',
    'ChangeSubnet',  # deprecated
    'Cleanup',
    'Connect',
    'ConnectionRules',
    'CopyModel',
    'Create',
    'CurrentSubnet',  # deprecated
    'DataConnect',  # deprecated
    'DisableStructuralPlasticity',
    'Disconnect',
    'DisconnectOneToOne',
    'EnableStructuralPlasticity',
    'EndSubnet',  # deprecated
    'GetChildren',  # deprecated
    'GetConnections',
    'GetDefaults',
    'GetKernelStatus',
    'GetLeaves',  # deprecated
    'GetLID',  # deprecated
    'GetNetwork',  # deprecated
    'GetNodes',  # deprecated
    'GetStatus',
    'GetStructuralPlasticityStatus',
    'Install',
    'LayoutNetwork',  # deprecated
    'Models',
    'NumProcesses',
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
