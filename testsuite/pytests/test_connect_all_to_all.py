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

import connect_test_base
import nest
import numpy as np
import scipy.stats

HAVE_MPI = nest.ll_api.sli_func("statusdict/have_mpi ::")
HAVE_OPENMP = nest.ll_api.sli_func("is_threaded")


@unittest.skipIf(not HAVE_MPI, "NEST was compiled without MPI")
@unittest.skipIf(not HAVE_OPENMP, "NEST was compiled without multi-threading")
@nest.ll_api.check_stack
class TestAllToAll(connect_test_base.ConnectTestBase):
    # specify connection pattern
    rule = "all_to_all"
    conn_dict = {"rule": rule}
    # sizes of populations
    N1 = 6
    N2 = 7
    N1_array = 500
    N2_array = 10

    def testConnectivity(self):
        self.setUpNetwork(self.conn_dict)
        # make sure all connections do exist
        M = connect_test_base.get_connectivity_matrix(self.pop1, self.pop2)
        M_all = np.ones((len(self.pop2), len(self.pop1)))
        connect_test_base.mpi_assert(M, M_all, self)
        # make sure no connections were drawn from the target to the source
        # population
        M = connect_test_base.get_connectivity_matrix(self.pop2, self.pop1)
        M_none = np.zeros((len(self.pop1), len(self.pop2)))
        connect_test_base.mpi_assert(M, M_none, self)

    def testInputArray(self):
        for label in ["weight", "delay"]:
            syn_params = {}
            if label == "weight":
                self.param_array = np.arange(self.N1_array * self.N2_array, dtype=float).reshape(
                    self.N2_array, self.N1_array
                )
            elif label == "delay":
                self.param_array = (
                    np.arange(1, self.N1_array * self.N2_array + 1).reshape(self.N2_array, self.N1_array) * 0.1
                )
            syn_params[label] = self.param_array
            nest.ResetKernel()
            self.setUpNetwork(self.conn_dict, syn_params, N1=self.N1_array, N2=self.N2_array)
            M_nest = connect_test_base.get_weighted_connectivity_matrix(self.pop1, self.pop2, label)
            connect_test_base.mpi_assert(M_nest, self.param_array, self)

    def testInputArrayWithoutAutapses(self):
        self.conn_dict["allow_autapses"] = False
        for label in ["weight", "delay"]:
            syn_params = {}
            if label == "weight":
                self.param_array = np.arange(self.N1 * self.N1, dtype=float).reshape(self.N1, self.N1)
            elif label == "delay":
                self.param_array = np.arange(1, self.N1 * self.N1 + 1).reshape(self.N1, self.N1) * 0.1
            syn_params[label] = self.param_array
            self.setUpNetworkOnePop(self.conn_dict, syn_params)
            M_nest = connect_test_base.get_weighted_connectivity_matrix(self.pop, self.pop, label)
            np.fill_diagonal(self.param_array, 0)
            connect_test_base.mpi_assert(M_nest, self.param_array, self)

    def testInputArrayRPort(self):
        syn_params = {}
        neuron_model = "iaf_psc_exp_multisynapse"
        neuron_dict = {"tau_syn": [0.1 + i for i in range(self.N2)]}
        self.pop1 = nest.Create(neuron_model, self.N1)
        self.pop2 = nest.Create(neuron_model, self.N2, neuron_dict)
        self.param_array = np.transpose(np.asarray([np.arange(1, self.N2 + 1) for i in range(self.N1)]))
        syn_params["receptor_type"] = self.param_array
        nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
        M = connect_test_base.get_weighted_connectivity_matrix(self.pop1, self.pop2, "receptor")
        connect_test_base.mpi_assert(M, self.param_array, self)

    def testInputArrayToStdpSynapse(self):
        params = ["Wmax", "alpha", "lambda", "mu_minus", "mu_plus", "tau_plus"]
        syn_params = {"synapse_model": "stdp_synapse"}
        values = [np.arange(self.N1 * self.N2, dtype=float).reshape(self.N2, self.N1) for i in range(6)]
        for i, param in enumerate(params):
            syn_params[param] = values[i]
        self.setUpNetwork(self.conn_dict, syn_params)
        for i, param in enumerate(params):
            a = connect_test_base.get_weighted_connectivity_matrix(self.pop1, self.pop2, param)
            connect_test_base.mpi_assert(a, values[i], self)

    # test single threaded for now
    def testRPortDistribution(self):
        n_rport = 10
        nr_neurons = 100
        nest.ResetKernel()  # To reset local_num_threads
        neuron_model = "iaf_psc_exp_multisynapse"
        neuron_dict = {"tau_syn": [0.1 + i for i in range(n_rport)]}
        self.pop1 = nest.Create(neuron_model, nr_neurons, neuron_dict)
        self.pop2 = nest.Create(neuron_model, nr_neurons, neuron_dict)
        syn_params = {"synapse_model": "static_synapse"}
        syn_params["receptor_type"] = 1 + nest.random.uniform_int(n_rport)
        nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
        M = connect_test_base.get_weighted_connectivity_matrix(self.pop1, self.pop2, "receptor")
        M = connect_test_base.gather_data(M)
        if M is not None:
            M = M.flatten()
            unique, counts = np.unique(M, return_counts=True)
            frequencies = np.asarray((unique, counts)).T
            self.assertTrue(np.array_equal(frequencies[:, 0], np.arange(1, n_rport + 1)), "Missing or invalid rports")
            chi, p = scipy.stats.chisquare(frequencies[:, 1])
            self.assertGreater(p, self.pval)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestAllToAll)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
