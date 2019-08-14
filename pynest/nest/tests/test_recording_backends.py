# -*- coding: utf-8 -*-
#
# test_recording_backends.py
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

HAVE_SIONLIB = nest.ll_api.sli_func("statusdict/have_sionlib ::")

class TestRecordingBackendstes(unittest.TestCase):

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
            command = nest.ll_api.sli_func(
                "mpirun", 2, "nosetests", test_script)
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
    suite = unittest.TestLoader().loadTestsFromTestCase(TestRecordingBackends)
    return suite

if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())




#
#(=============================================) =
#
#0 GetStatus /recording_backends get ==
#0 GetStatus /recording_backend get =
#0 GetStatus /recording_backend_status get info
#
#(=============================================) =
#
#0 << /recording_backend /ascii >> SetStatus
#0 GetStatus /recording_backend get =
#0 GetStatus /recording_backend_status get info
#
#(=============================================) =
#
#0 << /recording_backend /screen >> SetStatus
#0 GetStatus /recording_backend get =
#0 GetStatus /recording_backend_status get info
#
#(=============================================) =
#
#0 << /recording_backend /sionlib >> SetStatus
#0 GetStatus /recording_backend get =
#0 GetStatus /recording_backend_status get info
#
#(=============================================) =
#
#0 << /recording_backend /memory >> SetStatus
#/iaf_psc_alpha << /I_e 1800.0 >> Create /n Set
#/spike_detector Create /sd Set
#n sd Connect
#
#sd GetStatus info
#
#100 Simulate
#
#2 GetStatus info
#2 GetStatus /events get dup
#  /senders get size == ==
#  /times get ==
#
#(=============================================) =
#
#
#% Check if n_events works and if it can be set to 0 to delete the data
#2 << /n_events 0 >> SetStatus
#
#2 GetStatus info
#2 GetStatus /events get dup
#  /senders get size == ==
#  /times get ==
#
#
#% threading and MPI
#
#
#%
#% 0 GetStatus /recording_backends get
#% -> [/ascii /memory /screen /sionlib]
#%
#% 0 GetStatus /recording_backend_status get
#%
#%  <<
#%     /file_extension    /dat
#%     /name              /ascii
#%     /precision         3
#%  >>
#%
#%  <<
#%     /name              /memory
#%  >>
#%
#%  <<
#%     /name              /screen
#%     /precision         3
#%  >>
#%
#%  <<
#%     /buffer_size       1024
#%     /file_extension    (sion)
#%     /name              /sionlib
#%     /precision         3
#%     /sion_chunksize    1 << 48
#%     /sion_collective   false
#%  >>
#%
#% << /recording_backend /memory >> SetKernelStatus
#%
#
#
#% Conversion guide:
#
#
#% Lost properties in nestio:
#%   /close_after_simulate    always true
#%   /fbuffer_size            always OS default
#%   /flush_after_simulate    always true
#
#
#
#%
#%   /time_in_steps           always in ms
#%
