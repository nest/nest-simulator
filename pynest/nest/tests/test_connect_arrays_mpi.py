# -*- coding: utf-8 -*-
#
# test_connect_arrays_mpi.py
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

"""
This file contains two TestCases, and must be run from nosetests or pytest.
It requires NEST to be built with MPI and the Python package mpi4py.

The file can be run in two modes: with a single process, or with multiple MPI
processes. If run with multiple processes, the ConnectArraysMPICase TestCase
is run, which tests connecting with arrays when using multiple MPI processes.
If run with a single process, the TestConnectArraysMPI TestCase is run,
which runs ConnectArraysMPICase in a subprocess with multiple MPI processes,
and fails if any of the tests in ConnectArraysMPICase fail.
"""

import os
import subprocess as sp
import unittest
import nest
import numpy as np

try:
    from mpi4py import MPI
    HAVE_MPI4PY = True
except ImportError:
    HAVE_MPI4PY = False

HAVE_MPI = nest.ll_api.sli_func("statusdict/have_mpi ::")
MULTIPLE_PROCESSES = nest.NumProcesses() > 1


@unittest.skipIf(not HAVE_MPI4PY, 'mpi4py is not available')
class ConnectArraysMPICase(unittest.TestCase):
    """
    This TestCase uses mpi4py to collect and assert results from all
    processes, and is supposed to only be run with multiple processes.
    If running with nosetests or pytest, this TestCase is ignored when
    run with a single process, and called from TestConnectArraysMPI using
    multiple processes.
    """
    non_unique = np.array([1, 1, 3, 5, 4, 5, 9, 7, 2, 8], dtype=np.uint64)

    # The test class is instantiated by the unittest framework regardless of the value of
    # HAVE_MPI4PY, even though all tests will be skipped in case it is False. In this
    # situation, we have to manually prevent calls to MPI in order to avoid errors during
    # the execution.
    if HAVE_MPI4PY:
        comm = MPI.COMM_WORLD.Clone()

    # With pytest or nosetests, only run these tests if using multiple processes
    __test__ = MULTIPLE_PROCESSES

    def assert_connections(self, expected_sources, expected_targets, expected_weights, expected_delays, rule):
        """Gather connections from all processes and assert against expected connections"""
        conns = nest.GetConnections()
        projections = [[s, t] for s, t in zip(conns.source, conns.target)]
        weights = conns.weight
        delays = conns.delay
        if rule == 'one_to_one':
            expected_projections = np.array([[s, t] for s, t in zip(expected_sources, expected_targets)])
        elif rule == 'all_to_all':
            expected_projections = np.array([[s, t] for s in expected_sources for t in expected_targets])
        else:
            self.assertFalse(True, 'rule={} is not valid'.format(rule))

        expected_weights = (expected_weights if type(expected_weights) is np.ndarray
                            else expected_weights*np.ones(len(expected_projections)))
        expected_delays = (expected_delays if type(expected_delays) is np.ndarray
                           else expected_delays*np.ones(len(expected_projections)))

        recv_projections = self.comm.gather(projections, root=0)
        recv_weights = self.comm.gather(weights, root=0)
        recv_delays = self.comm.gather(delays, root=0)
        if self.comm.Get_rank() == 0:
            # Flatten the projection lists to a single list of projections
            recv_projections = np.array([proj for part in recv_projections for proj in part])
            recv_weights = np.array([w for part in recv_weights for w in part])
            recv_delays = np.array([proj for part in recv_delays for proj in part])
            # Results must be sorted to make comparison possible
            np.testing.assert_array_equal(np.sort(recv_projections, axis=0), np.sort(expected_projections, axis=0))
            np.testing.assert_array_almost_equal(np.sort(recv_weights, axis=0), np.sort(expected_weights, axis=0))
            np.testing.assert_array_almost_equal(np.sort(recv_delays, axis=0), np.sort(expected_delays, axis=0))
        else:
            self.assertIsNone(recv_projections)
            self.assertIsNone(recv_weights)
            self.assertIsNone(recv_delays)

    def setUp(self):
        nest.ResetKernel()

    def test_connect_arrays_unique(self):
        """Connecting NumPy arrays of unique node IDs with MPI"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = np.arange(1, n+1, dtype=np.uint64)
        weight = 1.5
        delay = 1.4

        nest.Connect(sources, targets, syn_spec={'weight': weight, 'delay': delay})

        self.assert_connections(sources, targets, weight, delay, 'all_to_all')

    def test_connect_arrays_nonunique(self):
        """Connecting NumPy arrays with non-unique node IDs with MPI"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique
        weights = np.ones(n)
        delays = np.ones(n)
        nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays},
                     conn_spec='one_to_one')

        self.assert_connections(sources, targets, weights, delays, 'one_to_one')

    def test_connect_arrays_threaded(self):
        """Connecting NumPy arrays, threaded with MPI"""
        nest.SetKernelStatus({'local_num_threads': 2})
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique
        syn_model = 'static_synapse'
        weights = np.linspace(0.6, 1.5, len(sources))  # Interval endpoints are carefully selected to get nice values,
        delays = np.linspace(0.4, 1.3, len(sources))   # that is, a step of 0.1 between values.

        nest.Connect(sources, targets, conn_spec='one_to_one',
                     syn_spec={'weight': weights,
                               'delay': delays,
                               'synapse_model': syn_model})

        self.assert_connections(sources, targets, weights, delays, 'one_to_one')


@unittest.skipIf(not HAVE_MPI, 'NEST was compiled without MPI')
@unittest.skipIf(not HAVE_MPI4PY, 'mpi4py is not available')
class TestConnectArraysMPI(unittest.TestCase):
    """
    When run with nosetests or pytest, this TestCase runs the ConnectArraysMPICase
    with multiple MPI processes in a subprocess. The test fails if any of the tests in
    ConnectArraysMPICase fail.
    """

    # With nosetests, only run this test if using a single process
    __test__ = not MULTIPLE_PROCESSES

    def testWithMPI(self):
        """Connect NumPy arrays with MPI"""
        directory = os.path.dirname(os.path.realpath(__file__))
        script = os.path.realpath(__file__)
        test_script = os.path.join(directory, script)
        command = nest.ll_api.sli_func("mpirun", 2, "nosetests", test_script)
        command = command.split()

        my_env = os.environ.copy()
        retcode = sp.call(command, env=my_env)

        self.assertEqual(retcode, 0, 'Test failed when run with "mpirun -np 2 nosetests [script]"')


if __name__ == '__main__':
    raise RuntimeError('This test must be run with nosetests or pytest')
