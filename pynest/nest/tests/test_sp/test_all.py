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

from . import test_synaptic_elements
from . import test_conn_builder
from . import test_growth_curves
from . import test_sp_manager
from . import test_disconnect
from . import test_disconnect_multiple
from . import test_enable_multithread
try:
    from mpi4py import MPI
except ImportError:
    # Test without MPI
    print ("MPI is not available. Skipping MPI tests.")
    mpi_test = 0
else:
    # Test with MPI
    mpi_test = 1
    print ("Testing with MPI")
    from subprocess import call
    import sys
    import os

__author__ = 'naveau'


def suite():
    # MPI tests
    if mpi_test:
        try:
            command = getMPITestCommand()
            command = command.replace(
                "  scriptfile", "test_sp/mpitest_issue_578_sp.py")
            print ("Executing test with command: " + command)
            command = command.split()
            call(command)
        except:
            print (sys.exc_info()[0])
            print ("Test call with MPI ended in error")
            raise
    test_suite = unittest.TestSuite()

    test_suite.addTest(test_synaptic_elements.suite())
    test_suite.addTest(test_conn_builder.suite())
    test_suite.addTest(test_growth_curves.suite())
    test_suite.addTest(test_sp_manager.suite())
    test_suite.addTest(test_synaptic_elements.suite())
    test_suite.addTest(test_disconnect.suite())
    test_suite.addTest(test_disconnect_multiple.suite())
    test_suite.addTest(test_enable_multithread.suite())

    return test_suite


def getMPITestCommand():
    # Open nestrc file and obtain the right mpi exec command
    # Substitute the scriptfile with the test file and substitute the number
    # of processes with 2 for this tests
    path = os.path.expanduser('~/.nestrc')
    nestrcf = open(path, "r")
    for line in nestrcf:
        if "(mpi" in line:
            line = line.replace(" numproc", "2")
            line = line.replace("cvs ( ) executable", "python")
            line = line.replace("(", "")
            line = line.replace(")", "")
            print ("MPI test command: " + line)
            nestrcf.close()
            return line

if __name__ == "__main__":
    nest.set_verbosity('M_WARNING')
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
