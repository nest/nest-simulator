# -*- coding: utf-8 -*-
#
# __init__.py
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

"""PyNEST - Python interface for the NEST simulator

* ``nest.helpdesk()`` opens the NEST documentation in your browser.

* ``nest.__version__`` displays the NEST version.

* ``nest.Models()`` shows all available neuron, device and synapse models.

* ``nest.help('model_name') displays help for the given model, e.g., ``nest.help('iaf_psc_exp')``

* To get help on functions in the ``nest`` package, use Python's ``help()`` function
  or IPython's ``?``, e.g.

     - ``help(nest.Create)``
     - ``nest.Connect?``

For more information visit https://www.nest-simulator.org.

"""


# Store interpreter-given module attributes to copy into replacement module
# instance later on. Use `.copy()` to prevent pollution with other variables
_original_module_attrs = globals().copy()

import sys, types, importlib
if sys.version_info[0] == 2:
    msg = "Python 2 is no longer supported. Please use Python >= 3.6."
    raise Exception(msg)


def _rel_import_star(module, import_module_name):
    """Emulates `from X import *` into `module`"""

    imported = importlib.import_module(import_module_name, __name__)
    imp_iter = vars(imported).items()
    if hasattr(module, "__all__"):
        # If a public api is defined using the `__all__` attribute, copy that.
        module.update(kv for kv in imp_iter if kv[0] in imported.__all__)
    else:
        # Otherwise follow "underscore is private" convention.
        module.update(kv for kv in imp_iter if not kv[0].startswith("_"))


def _lazy_module_property(module_name):
    """
    Returns a property that lazy loads a module and substitutes itself with it.
    The class variable name must match given `module_name`::

      class ModuleClass(types.ModuleType):
          lazy_module_xy = _lazy_module_property("lazy_module_xy")
    """
    def lazy_loader(self):
        cls = type(self)
        delattr(cls, module_name)
        module = importlib.import_module("." + module_name, __name__)
        setattr(cls, module_name, module)
        return module

    return property(lazy_loader)


class NestModule(types.ModuleType):
    """
    A module class for the `nest` root module to control the dynamic generation
    of module level attributes such as the KernelAttributes and lazy loading
    some submodules.
    """
    from . import ll_api                             # noqa
    from .ll_api import set_communicator             # noqa

    from . import pynestkernel as kernel             # noqa

    from . import random                             # noqa
    from . import math                               # noqa
    from . import spatial_distributions              # noqa
    from . import logic                              # noqa

    try:
        from . import server                         # noqa
    except ImportError:
        pass

    # Lazy load the `spatial` module to avoid circular imports.
    spatial = _lazy_module_property("spatial")

    __version__ = ll_api.sli_func("statusdict /version get")

# Instantiate a NestModule
_module = NestModule(__name__)
# We manipulate the nest module instance through its `__dict__` (= vars())
_module_dict = vars(_module)
# Copy over the original module attributes to preverse all interpreter given
# magic attributes such as `__name__`, `__path__`, `__package__`, ...
_module_dict.update(_original_module_attrs)

# Import public API of `.hl_api` into the nest module instance
_rel_import_star(_module_dict, ".hl_api")

class KernelAttribute:
    """
    A `KernelAttribute` dispatches attribute access of a `nest` attribute to the
    nest kernel by deferring to `nestGetKernelStatus` or `nest.SetKernelStatus`.
    """
    def __init__(self, name, doc, readonly=False):
        self._name = name
        self.__doc__ = doc
        self._readonly = readonly

    def __get__(self, instance, cls=None):
        if instance is None:
            return self
        return instance.GetKernelStatus(self._name)

    def __set__(self, instance, value):
        if self._readonly:
            raise ValueError(f"`{self._name}` is a read only kernel attribute.")
        return instance.SetKernelStatus({self._name: value})

# Parse the `SetKernelStatus` docstring to obtain the kernel attributes.
_doc_lines = _module.SetKernelStatus.__doc__.split('\n')
# Get the lines describing parameters
_param_lines = (line for line in _doc_lines if ' : ' in line)
# Exclude the first parameter `params`.
next(_param_lines)
# Process each parameter line into a KernelAttribute and add it to the nest
# module instance.
for ln in _param_lines:
    _param = ln.split(":")[0].strip()
    _readonly = "read only" in ln
    _kernel_attr = KernelAttribute(_param, None, _readonly)
    setattr(NestModule, _param, _kernel_attr)

# Finalize the nest module instance by generating its public API.
_module.__all__ = list(k for k in _module_dict if not k.startswith("_"))

# Set the nest module object as the return value of `import nest` using sys
sys.modules[__name__] = _module

# Some compiled/binary components (`pynestkernel.pyx` for example) of NEST
# obtain a reference to this file's original module object instead of what's in
# `sys.modules`. For these edge cases we make available all attributes of the
# nest module instance to this file's module object.
globals().update(_module_dict)

# Clean up obsolete references
del _param_lines, _doc_lines, _rel_import_star, _lazy_module_property, \
    _param, _readonly, _kernel_attr, _module_dict, _original_module_attrs
