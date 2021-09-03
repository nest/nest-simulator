# -*- coding: utf-8 -*-
#
# test_arbor_backend.py
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

# This file runs arbor_backend_test_nest_side.py and arbor_backend_test_arbor_side.py
# to test communication between NEST and Arbor.

import unittest
import nest
import os
import numpy as np
from subprocess import call

HAVE_MPI = nest.ll_api.sli_func("statusdict/have_mpi ::")
HAVE_ARBOR = nest.ll_api.sli_func("statusdict/have_recordingbackend_arbor ::")

try:
    from mpi4py import MPI
    HAVE_MPI4PY = True
except ImportError:
    HAVE_MPI4PY = False


class TestArborBackend(unittest.TestCase):

    def setUp(self):
        self.num_procs = 1
        self.nest_output_fname = 'spike_recorder-102-0.dat'
        self.arbor_output_fname = 'arbor_spikes.dat'

    def tearDown(self):
        for fname in [self.nest_output_fname, self.arbor_output_fname]:
            if os.path.isfile(fname):
                os.remove(fname)

    @unittest.skipIf(not HAVE_MPI, "NEST was compiled without MPI")
    @unittest.skipIf(not HAVE_MPI4PY, "mpi4py is not available")
    @unittest.skipIf(not HAVE_ARBOR, "NEST was compiled without Arbor backend")
    def test_arbor_backend(self):
        """
        Transmitting spikes with Arbor backend
        """
        path = os.path.dirname(__file__)
        nest_side = os.path.join(path, "arbor_backend_test_nest_side.py")
        arbor_side = os.path.join(path, "arbor_backend_test_arbor_side.py")
        args = f"{arbor_side} : -np {self.num_procs} python {nest_side}"
        command = nest.ll_api.sli_func("mpirun", self.num_procs, "python", args)
        print("Executing test with command: " + command)
        command = command.split()
        my_env = os.environ.copy()
        returncode = call(command, env=my_env)

        self.assertEqual(returncode, 0)

        nest_output_file_exists = os.path.isfile(self.nest_output_fname)
        arbor_output_file_exists = os.path.isfile(self.arbor_output_fname)
        self.assertTrue(nest_output_file_exists)
        self.assertTrue(arbor_output_file_exists)

        nest_spikes = np.loadtxt(self.nest_output_fname, skiprows=3)
        arbor_spikes = np.loadtxt(self.arbor_output_fname)
        arbor_spikes[:, 0] -= 100  # Need to shift Arbor IDs to make them comparable to NEST IDs
        np.testing.assert_array_equal(nest_spikes, arbor_spikes)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestArborBackend)
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
