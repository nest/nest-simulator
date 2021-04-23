# -*- coding: utf-8 -*-
#
# test_mpitests.py
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
import os
from subprocess import call

HAVE_MPI = nest.ll_api.sli_func("statusdict/have_mpi ::")


class TestMPIDependentTests(unittest.TestCase):

    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    def testsWithMPI(self):
        if HAVE_MPI:
            failing_tests = []
            mpitests = [('mpitest_get_local_vps.py', 4),
                        ('test_sp/mpitest_issue_578_sp.py', 2)]
            path = os.path.dirname(__file__)

            for test, num_procs in mpitests:
                test = os.path.join(path, test)
                command = nest.ll_api.sli_func("mpirun", num_procs, "nosetests", test)
                print("Executing test with command: " + command)
                command = command.split()
                my_env = os.environ.copy()
                returncode = call(command, env=my_env)
                if returncode != 0:  # call returns 0 for passing tests
                    failing_tests.append((test, num_procs))

            self.assertTrue(not failing_tests, 'The following tests failed ' +
                            'when executing with "mpirun -np N nosetests ' +
                            '[script]": {}'.format(failing_tests))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(
        TestStructuralPlasticityMPI)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
