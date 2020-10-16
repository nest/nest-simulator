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

import unittest
import nest

from . import test_conn_builder
from . import test_disconnect
from . import test_disconnect_multiple
from . import test_enable_multithread
from . import test_growth_curves
from . import test_mpitests
from . import test_sp_manager
from . import test_synaptic_elements
from . import test_update_synaptic_elements


HAVE_MPI = nest.ll_api.sli_func("statusdict/have_mpi ::")
if HAVE_MPI:
    print("Testing with MPI")
    from subprocess import call
    import sys
    import os

__author__ = 'naveau'


def suite():
    test_suite = unittest.TestSuite()

    test_suite.addTest(test_conn_builder.suite())
    test_suite.addTest(test_disconnect.suite())
    test_suite.addTest(test_disconnect_multiple.suite())
    test_suite.addTest(test_enable_multithread.suite())
    test_suite.addTest(test_growth_curves.suite())
    test_suite.addTest(test_mpitests.suite())
    test_suite.addTest(test_sp_manager.suite())
    test_suite.addTest(test_synaptic_elements.suite())
    test_suite.addTest(test_update_synaptic_elements.suite())

    return test_suite


if __name__ == "__main__":
    nest.set_verbosity('M_WARNING')
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
