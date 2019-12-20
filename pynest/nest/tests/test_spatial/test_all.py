# -*- coding: utf-8 -*-
#
# test_all.py
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
Testsuite for spatially distributed functions.

This testsuite mainly tests the part of the PyNEST interface
that relates to spatially distributed nodes.

It also tests the visualization functions that are available
in PyNEST only.
"""

import unittest

from nest.tests import compatibility

from . import test_basics
from . import test_connection_with_elliptical_mask
from . import test_dumping
from . import test_plotting
from . import test_rotated_rect_mask
from . import test_selection_function_and_elliptical_mask
from . import test_spatial_kernels


def suite():
    suite = unittest.TestSuite()

    suite.addTest(test_basics.suite())
    suite.addTest(test_connection_with_elliptical_mask.suite())
    suite.addTest(test_dumping.suite())
    suite.addTest(test_plotting.suite())
    suite.addTest(test_rotated_rect_mask.suite())
    suite.addTest(test_selection_function_and_elliptical_mask.suite())
    suite.addTest(test_spatial_kernels.suite())

    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
