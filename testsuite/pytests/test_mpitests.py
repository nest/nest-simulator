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
import os
from subprocess import call, check_output

have_mpi_cmd = ["sli", "-c", "statusdict/have_mpi :: =only"]
HAVE_MPI = check_output(have_mpi_cmd) == b'true'

try:
    from mpi4py import MPI
    HAVE_MPI4PY = True
except ImportError:
    HAVE_MPI4PY = False


class TestMPIDependentTests(unittest.TestCase):

    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    @unittest.skipIf(not HAVE_MPI4PY, 'mpi4py is not available')
    def testsWithMPI(self):
        failing_tests = []
        path = os.path.dirname(os.path.realpath(__file__))
        # all MPI tests as a list of pairs (filename, n_proc)
        mpitests = [
            ('mpitest_get_local_vps.py', 4),
            ('test_sp/mpitest_issue_578_sp.py', 2),
            ("test_connect_all_to_all.py", 2),
            ("test_connect_one_to_one.py", 2),
            ("test_connect_fixed_indegree.py", 2),
            ("test_connect_fixed_outdegree.py", 2),
            ("test_connect_fixed_total_number.py", 2),
            ("test_connect_pairwise_bernoulli.py", 2),
        ]

        for test, num_procs in mpitests:
            test = os.path.join(path, test)
            mpirun_cmd = ["sli", "-c", f"{num_procs} (nosetests) ({test}) mpirun =only"]
            command = check_output(mpirun_cmd).decode("utf-8")
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
        TestMPIDependentTests)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
