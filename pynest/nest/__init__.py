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

class NestModule(types.ModuleType):
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

    @property
    def spatial(self):
        # Classic switch-a-roo: while the `spatial` property is called
        # we remove the `spatial` property, so that we avoid inf. recursion
        # into this property when we try to `from . import spatial`.
        # Then when the `spatial` module has been retrieved we put its value
        # where the property was and return it.
        cls = type(self)
        del cls.spatial
        from . import spatial
        cls.spatial = spatial
        return spatial

    __version__ = ll_api.sli_func("statusdict /version get")

# Create a module instance and copy over the original module attributes
_module = NestModule(__name__)
_module_dict = vars(_module)
_module_dict.update(_original_module_attrs)

def rel_import_star(module_name):
    global _module_dict
    # Emulate a bit of import machinery to replace module level
    # `from .X import *` statements.
    module = importlib.import_module(module_name, __name__)
    mod_iter = vars(module).items()
    if hasattr(module, "__all__"):
        _module_dict.update(kv for kv in mod_iter if kv[0] in module.__all__)
    else:
        _module_dict.update(kv for kv in mod_iter if not kv[0].startswith("_"))

# Import public API of `.hl_api` into the module instance
rel_import_star(".hl_api")

# POC kernel attributes
class KernelAttribute:
    def __init__(self, name, doc, readonly=False):
        self._name = name
        self.__doc__ = doc
        self._readonly = readonly

    def __get__(self, instance, cls=None):
        if cls is not None:
            return self
        return instance.GetKernelStatus(self._name)

    def __set__(self, instance, value):
        if self._readonly:
            raise ValueError(f"`{self._name}` is a read only kernel attribute.")
        return instance.SetKernelStatus({self._name: value})

# Foobar list of processed kernel attributes, to be replaced by some tbd source
for attr in [
    KernelAttribute("local_num_threads", "This is a docstring"),
    KernelAttribute("network_size", "This is a docstring", readonly=True),
]:
    setattr(NestModule, attr._name, attr)

# Update the public API of the module
_module.__all__ = list(k for k in _module_dict if not k.startswith("_"))

# Set the interpreter's `nest` module to the curated module instance
# so that `import nest` returns the `_module` object.
sys.modules[__name__] = _module

# Some compiled/binary components of NEST manage to snag a reference to the
# original module instead of the one we put in `sys.modules`. Thus we unpack
# the module instance here to provide all the references at the module level for
# these edge cases.
globals().update(_module_dict)
