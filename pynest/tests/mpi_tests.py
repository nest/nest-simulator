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

from __future__ import print_function
import sys
import os
import subprocess
import unittest
import shlex


def check_output(cmd):
    return subprocess.check_output(shlex.split(cmd)).decode("utf-8")


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


cmd = "nest -c 'statusdict/have_mpi :: =only'"
HAVE_MPI = check_output(cmd) == "true"


class TestConnectAllPatterns(unittest.TestCase):

    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    def testWithMPI(self):
        script_dir = os.path.dirname(os.path.realpath(__file__))
        scripts = [
            "test_connect_all_to_all.py",
            "test_connect_one_to_one.py",
            "test_connect_fixed_indegree.py",
            "test_connect_fixed_outdegree.py",
            "test_connect_fixed_total_number.py",
            "test_connect_pairwise_bernoulli.py",
            "test_sp/mpitest_issue_578_sp.py",
        ]

        failing = []
        for script_name in scripts:
            script = os.path.join(script_dir, script_name)
            cmd = "nest -c '2 (python) ({}) mpirun =only'".format(script)
            test_cmd = check_output(cmd)
            try:
                output = check_output(test_cmd)
                eprint(output)
            except subprocess.CalledProcessError as e:
                eprint(e.output)
                failing.append(script_name)

        if failing:
            cmd = "nest -c '2 (python) ([script]) mpirun =only'"
            test_str = check_output(cmd)
            self.fail("The following tests failed when executing '{}': {}"
                      .format(test_str, ", ".join(failing)))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestConnectAllPatterns)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
