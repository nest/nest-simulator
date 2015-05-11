# -*- coding: utf-8 -*-
#
# test_connect_one_to_one.py
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
import nest
from . import test_connect_helpers as hf
from .test_connect_parameters import TestParams

class TestOneToOne(TestParams):

    # specify connection pattern
    rule = 'one_to_one'
    conn_dict = {'rule': rule}
    # sizes of populations
    N = 6
    N1 = N
    N2 = N
    
    #def testErrorMessages(self):
        
    def testConnectivity(self):
        self.setUpNetwork(self.conn_dict)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(self.pop1, self.pop2)
        self.assertTrue(hf.mpi_assert(M, np.identity(self.N)))
        # make sure no connections were drawn from the target to the source population
        M = hf.get_connectivity_matrix(self.pop2, self.pop1)
        self.assertTrue(hf.mpi_assert(M, np.zeros((self.N, self.N))))
                        
    # def testInputArray(self):
    #     # TODO: check for autapses
    #     # arrays not implemented for multithreading or mpi
    #     if self.comm.Get_size() == 1:
    #         nest.ResetKernel()
    #         syn_params = {}
    #         for label in ['weight', 'delay']:
    #             if label == 'weight':
    #                 self.onedarray = np.random.rand(self.N1)
    #             elif label == 'delay':
    #                 self.onedarray = np.random.randint(1,100,size=self.N1)*0.1
    #             syn_params[label] = self.onedarray
    #             self.setUpNetwork(syn_dict=syn_params)
    #             M_nest = hf.get_weighted_connectivity_matrix(self.pop1,self.pop2,label)
    #             self.assertTrue(np.allclose(np.diag(M_nest),self.onedarray))

    # def testInputArrayRPort(self):
    #     syn_params = {}
    #     nest.ResetKernel()
    #     neuron_model = 'iaf_psc_exp_multisynapse'
    #     neuron_dict = {'tau_syn': [0.1+i for i in range(self.N1)]}
    #     self.pop1 = nest.Create(neuron_model, self.N1, neuron_dict)
    #     self.pop2 = nest.Create(neuron_model, self.N1, neuron_dict)
    #     self.onedarray = np.arange(1, self.N1+1, dtype=int)
    #     syn_params['receptor_type'] = self.onedarray
    #     nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
    #     M = hf.get_weighted_connectivity_matrix(self.pop1, self.pop2, 'receptor')
    #     M = self.comm.gather(M, root=0)
    #     if self.rank == 0:
    #         M = sum(M)
    #         M = M.flatten()
    #         self.assertTrue(np.allclose(np.diag(M),self.onedarray))

def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestOneToOne)
    return suite

def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
    
        
if __name__ == '__main__':
    run()
