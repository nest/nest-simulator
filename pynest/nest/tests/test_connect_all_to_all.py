# -*- coding: utf-8 -*-
#
# test_connect_all_to_all.py
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


import unittest
import numpy as np
import scipy.stats
import nest
from . import test_connect_helpers as hf
from .test_connect_parameters import TestParams

@nest.check_stack
class TestAllToAll(TestParams):

    # specify connection pattern
    rule = 'all_to_all'
    conn_dict = {'rule': rule}
    # sizes of populations
    N1 = 6
    N2 = 7
            
    #def testErrorMessages(self):
      
    def testConnectivity(self):
        self.setUpNetwork(self.conn_dict)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(self.pop1, self.pop2)
        M_all = np.ones((len(self.pop2), len(self.pop1)))
        self.assertTrue(hf.mpi_assert(M, M_all))
        # make sure no connections were drawn from the target to the source population
        M = hf.get_connectivity_matrix(self.pop2, self.pop1)
        M_none = np.zeros((len(self.pop1), len(self.pop2)))
        self.assertTrue(hf.mpi_assert(M, M_none))
        
    # def testInputArray(self):
    #     # TODO: check what happens if autapses is set to false
    #     # arrays not implemented for multithreading or mpi
    #     if self.comm.Get_size() == 1:
    #         nest.ResetKernel()
    #         syn_params = {}
    #         for label in ['weight', 'delay']:
    #             if label == 'weight':
    #                 self.twodarray = np.random.rand(self.N2,self.N1)
    #             elif label == 'delay':
    #                 self.twodarray = np.random.randint(1,100,size=(self.N2,self.N1))*0.1
    #             syn_params[label] = self.twodarray.flatten()
    #             self.setUpNetwork(self.conn_dict,syn_dict=syn_params)
    #             M_nest = hf.get_weighted_connectivity_matrix(self.pop1,self.pop2,label)
    #             self.assertTrue(np.allclose(M_nest,self.twodarray))

    # test single threaded for now        
    def testRPortDistribution(self):
        n_rport = 100
        pval = 0.05
        nr_neurons = 20
        nest.ResetKernel()
        neuron_model = 'iaf_psc_exp_multisynapse'
        neuron_dict = {'tau_syn': [0.1+i for i in range(n_rport)]}
        self.pop1 = nest.Create(neuron_model, nr_neurons, neuron_dict)
        self.pop2 = nest.Create(neuron_model, nr_neurons, neuron_dict)       
        syn_params = {'model': 'static_synapse'}
        syn_params['receptor_type'] = {'distribution': 'uniform_int', 'low': 1, 'high': n_rport}
        nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
        M = hf.get_weighted_connectivity_matrix(self.pop1, self.pop2, 'receptor')
        params = syn_params
        params['pval'] = self.pval
        M = hf.gather_data(M)
        if M != None:
            M = M.flatten()
            test_dist = np.random.randint(params['receptor_type']['low'], params['receptor_type']['high'], len(M))
            chi, p = scipy.stats.ks_2samp(M, test_dist)
            print("p-value : %.2f" % p)
            if p > self.pval:
                is_dist = True
            else:
                is_dist = False
            self.assertTrue(is_dist)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestAllToAll)
    return suite
        
if __name__ == '__main__':
    unittest.TextTestRunner(verbosity=2).run(suite())
