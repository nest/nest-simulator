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

"""PyNEST - Python bindings for the NEST simulator

Type ``nest.helpdesk()`` to access the online documentation.
Type ``nest.help(object)`` to get help on a NEST object.
Type ``nest.__version__`` to get the NEST version.

* see documentation for neuron or synapse models by typing
  ``nest.help('model_name')``

* see the documentation of a PyNEST API function, by typing
  something like ``help(nest.Create)``

* get a list of available models by typing ``nest.Models()``

For more information visit https://www.nest-simulator.org.

"""

import sys
if sys.version_info[0] == 2:
    msg = "Python 2 is no longer supported. Please use Python >= 3.6."
    raise Exception(msg)

from . import ll_api                  # noqa
from .ll_api import set_communicator  # noqa

from . import pynestkernel as kernel  # noqa
from .hl_api import *                 # noqa

from . import random                  # noqa
from . import math                    # noqa
from . import spatial_distributions   # noqa
from . import logic                   # noqa

# spatial needs to be imported last because of documentation generation
from . import spatial                 # noqa

try:
    from . import server              # noqa
except ImportError:
    pass

__version__ = ll_api.sli_func("statusdict /version get")


def test():
    """Runs all PyNEST unit tests."""
    from . import tests
    import unittest

    debug = ll_api.get_debug()
    ll_api.set_debug(True)

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(tests.suite())

    ll_api.set_debug(debug)
