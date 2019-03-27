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


# The lines below ensure that Popened subprocesses run in a minimal
# environment that prevents mpirun from noticing that the parent is
# already using MPI and the child application itself from interfering
# with the MPI context of the parent.
# This is a rather crude hack to Popen MPI-enabled PyNEST from this
# MPI-enabled Python script (MPI is imported via NEST in the toplevel
# testsuite runner). The reason for requiring this hack is that some
# MPI implementations (e.g. OpenMPI > version 3) prevent running
# applications in such a way.
# A clean solution would require to run each PyNEST test as its own
# process from a runner that itself does not import MPI (here: nest or
# mpi4py). The clean solution requires a massive restructuring and
# will probably only be implemented when we move to another test
# framework for PyNEST altogether (see also https://git.io/fjUCg).
required_env_vars = ("PATH", "PYTHONPATH", "HOME")
env = {k: v for k, v in os.environ.items() if k in required_env_vars}


def check_output(cmd):
    cmd = shlex.split(cmd)
    output = subprocess.check_output(cmd, env=env)
    return output.decode("utf-8")


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
            "test_connect_pairwise_bernoulli.py"
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
