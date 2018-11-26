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

# With '__all__' we provide an explicit index of the package. Without any
# imported submodules and any redundant functions we could achieve shorter
# time of importing 'nest'.
__all__ = [
    'BeginSubnet',  # deprecated?
    'CGConnect',
    'CGParse',
    'CGSelectImplementation',
    'ChangeSubnet',  # deprecated?
    'Cleanup',
    'Connect',
    'ConnectionRules',
    'CopyModel',
    'Create',
    'CurrentSubnet',  # deprecated?
    'DataConnect',  # deprecated?
    'DisableStructuralPlasticity',
    'Disconnect',
    'DisconnectOneToOne',
    'EnableStructuralPlasticity',
    'EndSubnet',  # deprecated?
    'GetChildren',  # deprecated?
    'GetConnections',
    'GetDefaults',
    'GetKernelStatus',
    'GetLeaves',  # deprecated?
    'GetLID',  # deprecated?
    'GetNetwork',  # deprecated?
    'GetNodes',  # deprecated?
    'GetStatus',
    'GetStructuralPlasticityStatus',
    'Install',
    'LayoutNetwork',  # deprecated?
    'Models',
    'NumProcesses',
    'Prepare',
    'Rank',
    'ResetKernel',
    'ResetNetwork',
    'ResumeSimulation',
    'Run',
    'RunManager',
    'SetAcceptableLatency',
    'SetDefaults',
    'SetKernelStatus',
    'SetMaxBuffered',
    'SetStatus',
    'SetStructuralPlasticityStatus',
    'Simulate',
    # 'SuppressedDeprecationWarning',  # redundant?
    # 'Template',
    'authors',
    # 'broadcast',  # redundant?
    # 'catching_sli_run',  # redundant?
    # 'check_stack',  # redundant?
    # 'deprecated',  # redundant?
    # 'functools',  # redundant?
    # 'get_argv',  # redundant?
    # 'get_debug',  # redundant?
    # 'get_help_filepath',  # redundant?
    # 'get_unistring_type',  # redundant?
    # 'get_verbosity',
    # 'get_wrapped_text',  # redundant?
    'help',
    'helpdesk',
    # 'is_coercible_to_sli_array',  # redundant?
    # 'is_iterable',  # redundant?
    # 'is_literal',  # redundant?
    # 'is_sequence_of_connections',  # redundant?
    # 'is_sequence_of_gids',  # redundant?
    # 'is_string',  # redundant?
    # 'load_help',  # redundant?
    # 'message',  # redundant?
    # 'model_deprecation_warning',  # redundant?
    # 'name',  # redundant?
    # 'set_debug',  # redundant?
    # 'set_verbosity',
    # 'show_deprecation_warning',  # redundant?
    # 'show_help_with_pager',  # redundant?
    # 'stack_checker',  # redundant?
    'sysinfo',
    'version',
]
