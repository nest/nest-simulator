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
    N_array = 1000

    def testConnectivity(self):
        self.setUpNetwork(self.conn_dict)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(self.pop1, self.pop2)
        hf.mpi_assert(M, np.identity(self.N), self)
        # make sure no connections were drawn from the target to the source
        # population
        M = hf.get_connectivity_matrix(self.pop2, self.pop1)
        hf.mpi_assert(M, np.zeros((self.N, self.N)), self)

    def testSymmetricFlag(self):
        conn_dict_symmetric = self.conn_dict.copy()
        conn_dict_symmetric['make_symmetric'] = True
        self.setUpNetwork(conn_dict_symmetric)
        M1 = hf.get_connectivity_matrix(self.pop1, self.pop2)
        M2 = hf.get_connectivity_matrix(self.pop2, self.pop1)
        # test that connections were created in both directions
        hf.mpi_assert(M1, np.transpose(hf.gather_data(M2)), self)
        # test that no other connections were created
        hf.mpi_assert(M1, np.zeros_like(M1) + np.identity(self.N), self)

    def testInputArray(self):
        syn_params = {}
        for label in ['weight', 'delay']:
            if label == 'weight':
                self.param_array = np.arange(self.N_array, dtype=float)
            elif label == 'delay':
                self.param_array = np.arange(1, self.N_array + 1) * 0.1
            syn_params[label] = self.param_array
            hf.nest.ResetKernel()
            self.setUpNetwork(self.conn_dict, syn_params,
                              N1=self.N_array, N2=self.N_array)
            M_nest = hf.get_weighted_connectivity_matrix(
                self.pop1, self.pop2, label)
            hf.mpi_assert(M_nest, np.diag(self.param_array), self)

    def testInputArrayRPort(self):
        syn_params = {}
        neuron_model = 'iaf_psc_exp_multisynapse'
        neuron_dict = {'tau_syn': [0.1 + i for i in range(self.N1)]}
        self.pop1 = hf.nest.Create(neuron_model, self.N1, neuron_dict)
        self.pop2 = hf.nest.Create(neuron_model, self.N1, neuron_dict)
        self.param_array = np.arange(1, self.N1 + 1, dtype=int)
        syn_params['receptor_type'] = self.param_array
        hf.nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
        M = hf.get_weighted_connectivity_matrix(
            self.pop1, self.pop2, 'receptor')
        hf.mpi_assert(M, np.diag(self.param_array), self)

    def testInputArrayToStdpSynapse(self):
        params = ['Wmax', 'alpha', 'lambda', 'mu_minus', 'mu_plus', 'tau_plus']
        syn_params = {'synapse_model': 'stdp_synapse'}
        values = [np.arange(self.N1, dtype=float) for i in range(6)]
        for i, param in enumerate(params):
            syn_params[param] = values[i]
        self.setUpNetwork(self.conn_dict, syn_params)
        for i, param in enumerate(params):
            a = hf.get_weighted_connectivity_matrix(
                self.pop1, self.pop2, param)
            hf.mpi_assert(np.diag(a), values[i], self)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestOneToOne)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
