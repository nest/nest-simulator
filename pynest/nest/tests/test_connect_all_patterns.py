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
import subprocess
import unittest

cmd = ["nest", "-c", "statusdict/have_mpi :: =only"]
HAVE_MPI = subprocess.check_output(cmd) == "true"


class TestConnectAllPatterns(unittest.TestCase):

    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    def testWithMPI(self):
        directory = os.path.dirname(os.path.realpath(__file__))
        scripts = [
            "test_connect_all_to_all.py",
            "test_connect_one_to_one.py",
            "test_connect_fixed_indegree.py",
            "test_connect_fixed_outdegree.py",
            "test_connect_fixed_total_number.py",
            "test_connect_pairwise_bernoulli.py"
        ]

        failing_tests = []
        for script in scripts:
            script_abs = os.path.join(directory, script)
            cmd = ["nest", "-c", "2 (nosetests) (%s) mpirun =only" % script_abs]
            test_cmd = subprocess.check_output(cmd)
            process = subprocess.Popen(cmd)
            process.communicate()
            if process.returncode != 0:
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
