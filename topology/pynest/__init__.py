#! /usr/bin/env python
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

import nest
import hl_api

hl_api.nest = nest

def test ():
    """ Runs a battery of unit tests on Topology PyNEST """
    import nest.topology.tests
    import unittest

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(nest.topology.tests.suite())


from hl_api import *
