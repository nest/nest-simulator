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

import os
import subprocess
import unittest

cmd = ["nest", "-c", "statusdict/have_mpi :: =only"]
HAVE_MPI = subprocess.check_output(cmd) == "true"


class TestSPwithMPI(unittest.TestCase):

    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    def testWithMPI(self):
        script_dir = os.path.dirname(os.path.realpath(__file__))
        scripts = ["mpitest_issue_578_sp.py"]

        failing = []
        for script_name in scripts:
            print("")
            script = os.path.join(script_dir, script_name)
            cmd = ["nest", "-c", "2 (nosetests) (%s) mpirun =only" % script]
            test_cmd = subprocess.check_output(cmd)
            process = subprocess.Popen(cmd)
            process.communicate()
            if process.returncode != 0:
                failing.append(script_name)

        print("")
        cmd = ["nest", "-c", "2 (nosetests) ([script]) mpirun =only"]
        test_str = subprocess.check_output(cmd)
        self.assertTrue(not failing, 'The following tests failed when ' +
                        'executing "%s": %s' % (test_str, ", ".join(failing)))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSPwithMPI)
    return suite

if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
