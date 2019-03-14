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

# return all package attributes that from pkg import * would put in the module namespace
# see https://docs.python.org/3/reference/simple_stmts.html#import
def _pkg_attrs(pkg):
    try: # if there's an "all", return that
        return pkg.__all__
    except AttributeError: # otherwise, return everything at top level that doesn't start with `_`
        return (attr for attr in pkg.__dict__ if attr[0] != '_')

# import * names from pkg to the dictionary of mod_dict
def _import_names(mod_dict, pkg):
    for attr in _pkg_attrs(pkg):
        mod_dict[attr] = getattr(pkg, attr)

# get all files ./lib/*.py except specials starting with __
def _import_libs(mod_dict):
    libdir = os.path.join(os.path.dirname(__file__), "lib")
    for name in os.listdir(libdir):
        if name.endswith(".py") and not name.startswith('__'):
            pkg = __import__("lib.{}".format(name[:-3]), globals(), locals(), ['*'], 1)
            _import_names(mod_dict, pkg)

# do `from .libs.$X import *` for every module in ./libs/$X.py
_import_libs(globals())

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
