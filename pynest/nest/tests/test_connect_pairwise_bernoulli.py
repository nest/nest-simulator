# -*- coding: utf-8 -*-
#
# test_connect_pairwise_bernoulli.py
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
import nest
from . import test_connect_helpers as hf
from .test_connect_parameters import TestParams

class TestPairwiseBernoulli(TestParams):

    # specify connection pattern and specific params
    rule = 'pairwise_bernoulli'
    p = 0.5
    conn_dict = {'rule': rule, 'p': p}
    # sizes of source-, target-population and connection probability for statistical test
    N_s = 50
    N_t = 50
    # Critical values and number of iterations of two level test
    stat_dict = {'alpha2': 0.05, 'n_runs': 20}

    def testStatistics(self):
        for fan in ['in', 'out']:
            expected = hf.get_expected_degrees_bernoulli(self.p, fan, self.N_s, self.N_t)

            pvalues = []
            for i in range(self.stat_dict['n_runs']):
                hf.reset_seed(i, self.nr_threads)
                self.setUpNetwork(conn_dict=self.conn_dict,N1=self.N_s,N2=self.N_t)
                degrees = hf.get_degrees(fan, self.pop1, self.pop2)
                degrees = hf.gather_data(degrees)
                #degrees = self.comm.gather(degrees, root=0)
                #if self.rank == 0:
                if degrees != None:
                    chi, p = hf.chi_squared_check(degrees, expected, self.rule)
                    pvalues.append(p)
                #self.comm.Barrier()
                hf.mpi_barrier()
            #if self.rank == 0:
            if degrees != None:
                ks, p = scipy.stats.kstest(pvalues, 'uniform', alternative='two_sided')
                self.assertTrue( p > self.stat_dict['alpha2'] )

    def testAutapses(self):
        conn_params = self.conn_dict.copy()
        N = 10
        conn_params['multapses'] = False

        # test that autapses exist
        conn_params['p'] = 1.
        conn_params['autapses'] = True
        pop = nest.Create('iaf_neuron', N)
        nest.Connect(pop, pop, conn_params)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(pop, pop)
        self.assertTrue(hf.mpi_assert(M, np.ones(N), 'diagonal'))
        nest.ResetKernel()

        # test that autapses were excluded
        conn_params['p'] = 1.
        conn_params['autapses'] = False        
        pop = nest.Create('iaf_neuron', N)
        nest.Connect(pop, pop, conn_params)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(pop, pop)
        self.assertTrue(hf.mpi_assert(M, np.zeros(N), 'diagonal'))

    
def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestPairwiseBernoulli)
    return suite

def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
    

if __name__ == '__main__':
    run()
