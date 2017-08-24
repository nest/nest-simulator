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
        directory = os.path.dirname(os.path.realpath(__file__))
        scripts = ["test_connect_all_to_all.py",
                   "test_connect_one_to_one.py",
                   "test_connect_fixed_indegree.py",
                   "test_connect_fixed_outdegree.py",
                   "test_connect_fixed_total_number.py",
                   "test_connect_pairwise_bernoulli.py"
                   ]
        retcodes = []
        for script in scripts:
            test_script = os.path.join(directory, script)
            command = nest.sli_func("mpirun", 2, "nosetests",
                                    test_script)
            print("Executing test with command: " + command)
            command = command.split()
            retcodes.append(sp.call(command))
        failed_tests = ''
        for script, retcode in zip(scripts, retcodes):
            if retcode != 0:
                failed_tests += script + ' '
        self.assertEqual(failed_tests, '',
                         'Error using MPI with the following test(s): ' +
                         failed_tests)


if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestConnectAllPatterns)
    unittest.TextTestRunner(verbosity=2).run(suite)
