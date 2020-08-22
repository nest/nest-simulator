# -*- coding: utf-8 -*-
#
# test_connect_fixed_outdegree.py
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

import numpy as np
import unittest
import scipy.stats
from . import test_connect_helpers as hf
from .test_connect_parameters import TestParams


class TestFixedOutDegree(TestParams):

    # specify connection pattern and specific params
    rule = 'fixed_outdegree'
    conn_dict = {'rule': rule}
    # sizes of source-, target-population and outdegree for connection test
    N1 = 50
    N2 = 70
    Nout = 10
    conn_dict['outdegree'] = Nout
    # sizes of source-, target-population and outdegree for statistical test
    N_s = 10
    N_t = 10
    C = 10
    # Critical values and number of iterations of two level test
    stat_dict = {'alpha2': 0.05, 'n_runs': 100}

    # tested on each mpi process separately
    def testErrorMessages(self):
        got_error = False
        conn_params = self.conn_dict.copy()
        conn_params['allow_autapses'] = True
        conn_params['allow_multapses'] = False
        conn_params['outdegree'] = self.N2 + 1
        try:
            self.setUpNetwork(conn_params)
        except hf.nest.kernel.NESTError:
            got_error = True
        self.assertTrue(got_error)

    def testOutDegree(self):
        conn_params = self.conn_dict.copy()
        conn_params['allow_autapses'] = False
        conn_params['allow_multapses'] = False
        self.setUpNetwork(conn_params)
        # make sure the outdegree is right
        M = hf.get_connectivity_matrix(self.pop1, self.pop2)
        outds = np.sum(M, axis=0)
        hf.mpi_assert(outds, self.Nout * np.ones(self.N1), self)
        # make sure no connections were drawn from the target to the source
        # population
        M = hf.get_connectivity_matrix(self.pop2, self.pop1)
        M_none = np.zeros((len(self.pop1), len(self.pop2)))
        hf.mpi_assert(M, M_none, self)

    def testStatistics(self):
        conn_params = self.conn_dict.copy()
        conn_params['allow_autapses'] = True
        conn_params['allow_multapses'] = True
        conn_params['outdegree'] = self.C
        expected = hf.get_expected_degrees_fixedDegrees(
            self.C, 'out', self.N_s, self.N_t)
        pvalues = []
        for i in range(self.stat_dict['n_runs']):
            hf.reset_seed(i, self.nr_threads)
            self.setUpNetwork(conn_dict=conn_params, N1=self.N_s, N2=self.N_t)
            degrees = hf.get_degrees('in', self.pop1, self.pop2)
            degrees = hf.gather_data(degrees)
            if degrees is not None:
                chi, p = hf.chi_squared_check(degrees, expected)
                pvalues.append(p)
            hf.mpi_barrier()
        if degrees is not None:
            ks, p = scipy.stats.kstest(pvalues, 'uniform')
            self.assertTrue(p > self.stat_dict['alpha2'])

    def testAutapsesTrue(self):
        conn_params = self.conn_dict.copy()
        N = 10
        conn_params['allow_multapses'] = False

        # test that autapses exist
        conn_params['outdegree'] = N
        conn_params['allow_autapses'] = True
        pop = hf.nest.Create('iaf_psc_alpha', N)
        hf.nest.Connect(pop, pop, conn_params)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(pop, pop)
        hf.mpi_assert(np.diag(M), np.ones(N), self)

    def testAutapsesFalse(self):
        conn_params = self.conn_dict.copy()
        N = 10
        conn_params['allow_multapses'] = False

        # test that autapses were excluded
        conn_params['outdegree'] = N - 1
        conn_params['allow_autapses'] = False
        pop = hf.nest.Create('iaf_psc_alpha', N)
        hf.nest.Connect(pop, pop, conn_params)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(pop, pop)
        hf.mpi_assert(np.diag(M), np.zeros(N), self)

    def testMultapsesTrue(self):
        conn_params = self.conn_dict.copy()
        N = 3
        conn_params['allow_autapses'] = True

        # test that multapses were drawn
        conn_params['outdegree'] = N + 1
        conn_params['allow_multapses'] = True
        pop = hf.nest.Create('iaf_psc_alpha', N)
        hf.nest.Connect(pop, pop, conn_params)
        nr_conns = len(hf.nest.GetConnections(pop, pop))
        hf.mpi_assert(nr_conns, conn_params['outdegree'] * N, self)

    def testMultapsesFalse(self):
        conn_params = self.conn_dict.copy()
        N = 3
        conn_params['allow_autapses'] = True

        # test that no multapses exist
        conn_params['outdegree'] = N
        conn_params['allow_multapses'] = False
        pop = hf.nest.Create('iaf_psc_alpha', N)
        hf.nest.Connect(pop, pop, conn_params)
        M = hf.get_connectivity_matrix(pop, pop)
        M = hf.gather_data(M)
        if M is not None:
            self.assertTrue(M.flatten, np.ones(N * N))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestFixedOutDegree)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
