# -*- coding: utf-8 -*-
#
# test_parameters_thread_invariance.py
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
import ast
import unittest
import nest

HAVE_OPENMP = nest.ll_api.sli_func('is_threaded')
HAVE_MPI = nest.ll_api.sli_func('statusdict/have_mpi ::')


class TestParamThreadInvariance(unittest.TestCase):

    @unittest.skipIf(not HAVE_OPENMP, 'NEST was compiled without multi-threading')
    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    def test_exponential_thread_invariance(self):
        self.param_thread_invariance('exponential')

    @unittest.skipIf(not HAVE_OPENMP, 'NEST was compiled without multi-threading')
    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    def test_lognormal_thread_invariance(self):
        self.param_thread_invariance('lognormal')

    @unittest.skipIf(not HAVE_OPENMP, 'NEST was compiled without multi-threading')
    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    def test_normal_thread_invariance(self):
        self.param_thread_invariance('normal')

    @unittest.skipIf(not HAVE_OPENMP, 'NEST was compiled without multi-threading')
    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    def test_uniform_thread_invariance(self):
        self.param_thread_invariance('uniform')

    @unittest.skipIf(not HAVE_OPENMP, 'NEST was compiled without multi-threading')
    @unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
    def test_uniform_int_thread_invariance(self):
        self.param_thread_invariance('uniform_int')

    def param_thread_invariance(self, parameter):
        merged_results = []
        path = os.path.dirname(os.path.realpath(__file__))
        test = os.path.join(path, 'mpitest_test_parameters_thread_invariance.py')
        NUM_EXPECTED_VALUES = 10

        # The test runs with total_num_virtual_procs=4, so we keep the number of VPs constant while varying
        # the number of MPI processes.
        for num_procs in [1, 2, 4]:
            mpirun_cmd = ['sli', '-c', f'{num_procs} (python) ({test} {parameter}) mpirun =only']
            command = sp.check_output(mpirun_cmd).decode('utf-8')
            print('Executing test with command: ' + command)
            command = command.split()
            my_env = os.environ.copy()
            my_env['PYNEST_QUIET'] = ''  # to suppress welcome message
            result = sp.run(command, env=my_env, capture_output=True)
            # A nonzero returncode means the script crashed somehow
            self.assertEqual(result.returncode, 0, result.stderr.decode('utf-8'))
            # Convert the output string to a list of tuples, one for each MPI process
            result_values = [ast.literal_eval(r) for r in result.stdout.decode('utf-8').strip('\n').split('\n')]
            # Each process prints a tuple for all nodes, with values for non-local nodes set to None.
            # Therefore we need to merge the results from all processes to get a comparable set of values.
            merged_result = list(result_values[0])
            self.assertEqual(len(merged_result), NUM_EXPECTED_VALUES)
            for result_value in result_values[1:]:
                self.assertEqual(len(result_value), NUM_EXPECTED_VALUES)
                for i, value in enumerate(result_value):
                    if value is not None:
                        self.assertIsNone(merged_result[i])  # fails if there are conflicting values from processes
                        merged_result[i] = value
            self.assertNotIn(None, merged_result)  # fails if not all values are filled
            merged_results.append(merged_result)

        print(merged_results)
        # Compare that merged results from all MPI configurations are the same
        for merged_result in merged_results[1:]:
            self.assertEqual(merged_result, merged_results[0])


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestParamThreadInvariance)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
