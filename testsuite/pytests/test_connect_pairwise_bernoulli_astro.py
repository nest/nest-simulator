# -*- coding: utf-8 -*-
#
# test_connect_pairwise_bernoulli_astro.py
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
import connect_test_base
import nest


class TestPairwiseBernoulliAstro(connect_test_base.ConnectTestBase):

    # specify connection pattern and specific params
    rule = 'pairwise_bernoulli_astro'
    p = 0.5
    p_syn_astro = 1.0
    max_astro_per_target = 1
    astro_pool_by_index = True
    conn_dict = {
        'rule': rule, 'p': p, 'p_syn_astro': p_syn_astro,
        'max_astro_per_target': max_astro_per_target,
        'astro_pool_by_index': astro_pool_by_index}
    # sizes of source-, target-population and connection probability for
    # statistical test
    N_s = 50
    N_t = 50
    # Critical values and number of iterations of two level test
    stat_dict = {'alpha2': 0.05, 'n_runs': 20}

    def setUpNetwork(self, conn_dict=None, syn_dict=None, N1=None, N2=None,
        neuron_model='aeif_cond_alpha_astro'):
        if N1 is None:
            N1 = self.N_s
        if N2 is None:
            N2 = self.N_t
        self.pop1 = nest.Create(neuron_model, N1)
        self.pop2 = nest.Create(neuron_model, N2)
        self.pop_astro = nest.Create('astrocyte', N2)
        conn_dict['astrocyte'] = self.pop_astro
        nest.set_verbosity('M_FATAL')
        nest.Connect(self.pop1, self.pop2, conn_dict, syn_dict)

    def testStatistics(self):
        for fan in ['in', 'out']:
            expected = connect_test_base.get_expected_degrees_bernoulli(
                self.p, fan, self.N_s, self.N_t)

            pvalues = []
            for i in range(self.stat_dict['n_runs']):
                connect_test_base.reset_seed(i+1, self.nr_threads)
                self.setUpNetwork(conn_dict=self.conn_dict)
                degrees = connect_test_base.get_degrees(fan, self.pop1, self.pop2)
                degrees = connect_test_base.gather_data(degrees)
                if degrees is not None:
                    chi, p = connect_test_base.chi_squared_check(degrees, expected, 'pairwise_bernoulli')
                    pvalues.append(p)
                connect_test_base.mpi_barrier()
            if degrees is not None:
                ks, p = scipy.stats.kstest(pvalues, 'uniform')
                self.assertTrue(p > self.stat_dict['alpha2'])

    def testAutapsesTrue(self):
        conn_params = self.conn_dict.copy()
        N = 10
        conn_params['allow_multapses'] = False

        # test that autapses exist
        conn_params['p'] = 1.
        conn_params['allow_autapses'] = True
        pop = nest.Create('aeif_cond_alpha_astro', N)
        astro_pop = nest.Create('astrocyte', N)
        conn_params['astrocyte'] = astro_pop
        nest.Connect(pop, pop, conn_params)
        # make sure all connections do exist
        M = connect_test_base.get_connectivity_matrix(pop, pop)
        connect_test_base.mpi_assert(np.diag(M), np.ones(N), self)

    def testAutapsesFalse(self):
        conn_params = self.conn_dict.copy()
        N = 10

        # test that autapses were excluded
        conn_params['p'] = 1.
        conn_params['allow_autapses'] = False
        pop = nest.Create('aeif_cond_alpha_astro', N)
        astro_pop = nest.Create('astrocyte', N)
        conn_params['astrocyte'] = astro_pop
        nest.Connect(pop, pop, conn_params)
        # make sure all connections do exist
        M = connect_test_base.get_connectivity_matrix(pop, pop)
        connect_test_base.mpi_assert(np.diag(M), np.zeros(N), self)

    def testRPortSetting(self):
        neuron_model = 'iaf_psc_exp_multisynapse'
        neuron_dict = {'tau_syn': [0.5, 0.7]}
        rtype = 2
        syn_params = {'synapse_model': 'static_synapse',
                      'receptor_type': rtype}
        self.pop1 = nest.Create(neuron_model, self.N_s, neuron_dict)
        self.pop2 = nest.Create(neuron_model, self.N_t, neuron_dict)
        self.pop_astro = nest.Create('astrocyte', self.N_t)
        self.conn_dict['astrocyte'] = self.pop_astro
        conn_spec = self.conn_dict.copy()
        # exclude astrocyte because it is not compatible
        conn_spec['p_syn_astro'] = 0.0
        nest.Connect(self.pop1, self.pop2, conn_spec, syn_params)
        conns = nest.GetConnections(self.pop1, self.pop2)
        ports = conns.get('receptor')
        self.assertTrue(connect_test_base.all_equal(ports))
        self.assertTrue(ports[0] == rtype)

    def testRPortAllSynapses(self):
        syns = ['cont_delay_synapse', 'ht_synapse', 'quantal_stp_synapse',
                'static_synapse_hom_w', 'stdp_dopamine_synapse',
                'stdp_facetshw_synapse_hom', 'stdp_pl_synapse_hom',
                'stdp_synapse_hom', 'stdp_synapse', 'tsodyks2_synapse',
                'tsodyks_synapse'
                ]
        syn_params = {'receptor_type': 1}

        for i, syn in enumerate(syns):
            if syn == 'stdp_dopamine_synapse':
                vol = nest.Create('volume_transmitter')
                nest.SetDefaults('stdp_dopamine_synapse', {'vt': vol.get('global_id')})
            syn_params['synapse_model'] = syn
            self.pop1 = nest.Create('iaf_psc_exp_multisynapse', self.N_s, {
                                       'tau_syn': [0.2, 0.5]})
            self.pop2 = nest.Create('iaf_psc_exp_multisynapse', self.N_t, {
                                       'tau_syn': [0.2, 0.5]})
            self.pop_astro = nest.Create('astrocyte', self.N_t)
            self.conn_dict['astrocyte'] = self.pop_astro
            conn_spec = self.conn_dict.copy()
            # exclude astrocyte because it is not compatible
            conn_spec['p_syn_astro'] = 0.0
            nest.Connect(self.pop1, self.pop2, conn_spec, syn_params)
            conns = nest.GetConnections(self.pop1, self.pop2)
            conn_params = conns.get('receptor')
            self.assertTrue(connect_test_base.all_equal(conn_params))
            self.assertTrue(conn_params[0] == syn_params['receptor_type'])
            self.setUp()

    def testDelayAllSynapses(self):
        syns = ['cont_delay_synapse',
                'ht_synapse', 'quantal_stp_synapse',
                'static_synapse_hom_w',
                'stdp_dopamine_synapse',
                'stdp_facetshw_synapse_hom', 'stdp_pl_synapse_hom',
                'stdp_synapse_hom', 'stdp_synapse', 'tsodyks2_synapse',
                'tsodyks_synapse'
                ]
        syn_params = {'delay': 0.4}

        for syn in syns:
            if syn == 'stdp_dopamine_synapse':
                vol = nest.Create('volume_transmitter')
                nest.SetDefaults('stdp_dopamine_synapse', {'vt': vol.get('global_id')})
            syn_params['synapse_model'] = syn
            # exclude astrocyte because it is not compatible with
            # static_synapse_hom_w connection
            tmp = self.conn_dict['p_syn_astro']
            self.conn_dict['p_syn_astro'] = 0.0
            connect_test_base.check_synapse(
                ['delay'], [syn_params['delay']], syn_params, self)
            self.setUp()
            self.conn_dict['p_syn_astro'] = tmp

def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestPairwiseBernoulliAstro)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
