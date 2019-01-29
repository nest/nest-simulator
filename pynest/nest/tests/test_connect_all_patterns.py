# -*- coding: utf-8 -*-
#
# test_connect_all_patterns.py
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

import os
import subprocess as sp
import unittest
import nest

HAVE_MPI = nest.sli_func("statusdict/have_mpi ::")


class TestConnectAllPatterns(unittest.TestCase):

    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    def testWithMPI(self):
        # Check that we can import mpi4py
        try:
            from mpi4py import MPI
        except ImportError:
            raise unittest.SkipTest("mpi4py required")
        directory = os.path.dirname(os.path.realpath(__file__))
        scripts = ["test_connect_all_to_all.py",
                   "test_connect_one_to_one.py",
                   "test_connect_fixed_indegree.py",
                   "test_connect_fixed_outdegree.py",
                   "test_connect_fixed_total_number.py",
                   "test_connect_pairwise_bernoulli.py"
                   ]
        failing_tests = []
        for script in scripts:
            test_script = os.path.join(directory, script)
            command = nest.sli_func("mpirun", 2, "nosetests",
                                    test_script)
            command = command.split()
            process = sp.Popen(command, stdout=sp.PIPE, stderr=sp.PIPE)
            stdout, stderr = process.communicate()
            retcode = process.returncode
            if retcode != 0:
                failing_tests.append(script)
        self.assertTrue(not failing_tests, 'The following tests failed when ' +
                        'executing with "mpirun -np 2 nosetests [script]": ' +
                        ", ".join(failing_tests))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestConnectAllPatterns)
    return suite

if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
