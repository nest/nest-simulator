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

import os, sys

# We search through the subdirectory "lib" of the "nest" module
# directory and import the content of all Python files therein into
# the global namespace. This makes the API functions of PyNEST itself
# and those of extra modules available to the user.

# return all package attributes that from $pkg import * would put in the module namespace $pkg
# see https://docs.python.org/3/reference/simple_stmts.html#import
def _pkg_attrs(pkg):
    try: # if there's an "__all__", return that
        return pkg.__all__
    except AttributeError: # otherwise, return everything at top level that doesn't start with `_`
        return (attr for attr in pkg.__dict__ if attr[0] != '_')

# import * names from pkg to the dictionary of mod_dict
def _import_names(pkg, mod_dict):
    for attr in _pkg_attrs(pkg):
        mod_dict[attr] = getattr(pkg, attr)

# from .$prefix.$x import * ; relative to mod_file which is the filename from which mod_dict was read
# where $x.py is all files ./$path/*.py not starting with __
def _import_libs(mod_file, mod_dict, path, prefix, ignore_modules=frozenset()):
    libdir = os.path.join(os.path.dirname(mod_file), path)
    for name in os.listdir(libdir):
        if not name.endswith(".py") or name.startswith('__'):
            continue # not a regular python module
        
        pkg_name = "{}.{}".format(prefix, name[:-3])
        if pkg_name in ignore_modules:
            continue # this package is not to be imported dynamically

        pkg = __import__(pkg_name, mod_dict, locals(), ['*'], 1)
        _import_names(pkg, mod_dict)

#############################
# insert static imports here
# in the form:
# from lib.$X import * ; _ignore_modules.add('lib.$X')
############################
_ignore_modules = set()

############################
# Then whatever is left over, load dynamically
# then do `from .libs.$X import *` for every module in ./libs/$X.py that is left
_import_libs(__file__, globals(), 'lib', 'lib', _ignore_modules)
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
