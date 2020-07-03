# -*- coding: utf-8 -*-
#
# test_connect_symmetric_pairwise_bernoulli.py
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


import collections
import numpy as np
import unittest
import scipy.stats
from . import test_connect_helpers as hf
from .test_connect_parameters import TestParams


class TestSymmetricPairwiseBernoulli(TestParams):

    # sizes of source-, target-population and connection probability for
    # statistical test
    N_s = 60
    N_t = 60
    # specify connection pattern and specific params
    rule = 'symmetric_pairwise_bernoulli'
    p = 0.5
    conn_dict = {'rule': rule, 'p': p, 'allow_multapses': True,
                 'allow_autapses': False, 'make_symmetric': True}
    # Critical values and number of iterations of two level test
    stat_dict = {'alpha2': 0.05, 'n_runs': 20}

    def testStatistics(self):
        for fan in ['in', 'out']:
            expected = hf.get_expected_degrees_bernoulli(
                self.p, fan, self.N_s, self.N_t)

            pvalues = []
            for i in range(self.stat_dict['n_runs']):
                hf.reset_seed(i, self.nr_threads)
                self.setUpNetwork(conn_dict=self.conn_dict,
                                  N1=self.N_s, N2=self.N_t)
                degrees = hf.get_degrees(fan, self.pop1, self.pop2)
                degrees = hf.gather_data(degrees)
                # degrees = self.comm.gather(degrees, root=0)
                # if self.rank == 0:
                if degrees is not None:
                    chi, p = hf.chi_squared_check(degrees, expected, self.rule)
                    pvalues.append(p)
                hf.mpi_barrier()
            if degrees is not None:
                ks, p = scipy.stats.kstest(pvalues, 'uniform')
                self.assertTrue(p > self.stat_dict['alpha2'])

    def testAutapsesTrue(self):
        conn_params = self.conn_dict.copy()
        conn_params['allow_autapses'] = True
        N = 10

        # test that autapses are not permitted
        pop = hf.nest.Create('iaf_psc_alpha', N)
        with self.assertRaises(hf.nest.kernel.NESTError):
            hf.nest.Connect(pop, pop, conn_params)

    def testAutapsesFalse(self):
        conn_params = self.conn_dict.copy()
        N = 10

        # test that autapses were excluded
        conn_params['p'] = 1. - 1. / N
        conn_params['allow_autapses'] = False
        pop = hf.nest.Create('iaf_psc_alpha', N)
        hf.nest.Connect(pop, pop, conn_params)
        M = hf.get_connectivity_matrix(pop, pop)
        hf.mpi_assert(np.diag(M), np.zeros(N), self)

    def testMultapses(self):
        conn_params = self.conn_dict.copy()
        conn_params['allow_multapses'] = False
        N = 10

        # test that multapses must be permitted
        hf.nest.ResetKernel()
        pop = hf.nest.Create('iaf_psc_alpha', N)
        with self.assertRaises(hf.nest.kernel.NESTError):
            hf.nest.Connect(pop, pop, conn_params)

        # test that multapses can only arise from symmetric
        # connectivity
        conn_params['p'] = 1. - 1. / N
        conn_params['allow_multapses'] = True
        hf.nest.ResetKernel()
        pop = hf.nest.Create('iaf_psc_alpha', N)
        hf.nest.Connect(pop, pop, conn_params)

        conn_dict = collections.defaultdict(int)
        conn = hf.nest.GetConnections()
        for s_t_key in zip(conn.sources(), conn.targets()):
            conn_dict[s_t_key] += 1
            self.assertTrue(conn_dict[s_t_key] <= 2)

    def testMakeSymmetric(self):
        conn_params = self.conn_dict.copy()
        N = 100

        # test that make_symmetric must be enabled
        conn_params['make_symmetric'] = False
        hf.nest.ResetKernel()
        pop = hf.nest.Create('iaf_psc_alpha', N)
        with self.assertRaises(hf.nest.kernel.NESTError):
            hf.nest.Connect(pop, pop, conn_params)

        # test that all connections are symmetric
        conn_params['make_symmetric'] = True
        hf.nest.ResetKernel()
        pop = hf.nest.Create('iaf_psc_alpha', N)
        hf.nest.Connect(pop, pop, conn_params)

        conns = set()
        conn = hf.nest.GetConnections()
        for s_t_key in zip(conn.sources(), conn.targets()):
            conns.add(s_t_key)

        for s_t_key in zip(conn.sources(), conn.targets()):
            self.assertTrue(s_t_key[::-1] in conns)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(
        TestSymmetricPairwiseBernoulli)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
