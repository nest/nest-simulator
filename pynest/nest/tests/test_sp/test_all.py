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
__author__ = 'naveau'

import unittest
import nest

from nest.tests.test_sp import test_synaptic_elements
from nest.tests.test_sp import test_conn_builder
from nest.tests.test_sp import test_growth_curves
from nest.tests.test_sp import test_sp_manager
from nest.tests.test_sp import test_disconnect
from nest.tests.test_sp import test_disconnect_multiple


def suite():
    test_suite = unittest.TestSuite()

    test_suite.addTest(test_synaptic_elements.suite())
    test_suite.addTest(test_conn_builder.suite())
    test_suite.addTest(test_growth_curves.suite())
    test_suite.addTest(test_sp_manager.suite())
    test_suite.addTest(test_synaptic_elements.suite())
    test_suite.addTest(test_disconnect.suite())
    test_suite.addTest(test_disconnect_multiple.suite())

    return test_suite


if __name__ == "__main__":
    nest.set_verbosity('M_WARNING')
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
