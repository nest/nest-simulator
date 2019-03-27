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

# We search through the subdirectory "lib" of the "nest" module
# directory and import the content of all Python files therein into
# the global namespace. This makes the API functions of PyNEST itself
# and those of extra modules available to the user.


def _import_libs(mod_file, mod_dict, path,
                 prefix=None, ignore=frozenset(), level=1):
    """
    _import_libs: construct a relative import of all modules
      mod_file: name of calling module file (__file__)
      mod_dict: globals() of calling module
      path: relative path to check for modules
      prefix: string for import prefix (defaults to path)
      ignore: set of import names ($prefix.$name) to not import
      level: relative level (1 is relative to path, 2 is ../path, ...)
             (default 1)
      """
                 
    import ast
    import os

    if prefix is None:
        prefix = path

    # from .$prefix.$x import *
    # relative to mod_file which is the filename from which mod_dict was read
    # where $x.py is all files ./$path/*.py not starting with __
    libdir = os.path.join(os.path.dirname(mod_file), path)
    for name in os.listdir(libdir):
        if not name.endswith(".py") or name.startswith('__'):
            continue  # not a regular python module

        pkg_name = "{}.{}".format(prefix, name[:-3])
        if pkg_name in ignore:
            continue  # this package is not to be imported dynamically

        # construct from .pkg_name import * ; this is the old 'exec(string)'
        names = [ast.alias(name='*', asname=None)]
        body = [ast.ImportFrom(module=pkg_name, names=names, level=level)]
        module = ast.fix_missing_locations(ast.Module(body=body))

        code = compile(module, mod_file, 'exec')
        exec(code, mod_dict, mod_dict)


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
_import_libs(__file__, globals(), 'lib', ignore=_ignore_modules)
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
