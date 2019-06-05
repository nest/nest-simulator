# -*- coding: utf-8 -*-
#
# test_connect_parameters.py
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

'''
Tests of non pattern specific parameter.
The tests for all rules will run these tests separately.
'''


import unittest
import numpy as np
from . import test_connect_helpers as hf


class TestParams(unittest.TestCase):

    # Setting default parameter. These parameter might be overwritten
    # by the classes
    # testing one specific rule.
    # specify connection pattern
    rule = 'one_to_one'
    conn_dict = {'rule': rule}
    # sizes of populations
    N1 = 6
    N2 = 6
    # time step
    dt = 0.1
    # default params
    w0 = 1.0
    d0 = 1.0
    r0 = 0
    syn0 = 'static_synapse'
    # parameter for test of distributed parameter
    pval = 0.05  # minimum p-value to pass kolmogorov smirnov test
    # number of threads
    nr_threads = 2

    # for now only tests if a multi-thread connect is successfull, not whether
    # the the threading is actually used
    def setUp(self):
        hf.nest.ResetKernel()
        hf.nest.SetKernelStatus({'local_num_threads': self.nr_threads})

    def setUpNetwork(self, conn_dict=None, syn_dict=None, N1=None, N2=None):
        if N1 is None:
            N1 = self.N1
        if N2 is None:
            N2 = self.N2
        self.pop1 = hf.nest.Create('iaf_psc_alpha', N1)
        self.pop2 = hf.nest.Create('iaf_psc_alpha', N2)
        hf.nest.set_verbosity('M_FATAL')
        hf.nest.Connect(self.pop1, self.pop2, conn_dict, syn_dict)

    def setUpNetworkOnePop(self, conn_dict=None, syn_dict=None, N=None):
        if N is None:
            N = self.N1
        self.pop = hf.nest.Create('iaf_psc_alpha', N)
        hf.nest.set_verbosity('M_FATAL')
        hf.nest.Connect(self.pop, self.pop, conn_dict, syn_dict)

    def testWeightSetting(self):
        # test if weights are set correctly

        # one weight for all connections
        w0 = 0.351
        label = 'weight'
        syn_params = {label: w0}
        hf.check_synapse([label], [syn_params['weight']], syn_params, self)

    def testDelaySetting(self):
        # test if delays are set correctly

        # one delay for all connections
        d0 = 0.275
        syn_params = {'delay': d0}
        self.setUpNetwork(self.conn_dict, syn_params)
        connections = hf.nest.GetStatus(
            hf.nest.GetConnections(self.pop1, self.pop2))
        nest_delays = [connection['delay'] for connection in connections]
        # all delays need to be equal
        self.assertTrue(hf.all_equal(nest_delays))
        # delay (rounded) needs to equal the delay that was put in
        self.assertTrue(abs(d0 - nest_delays[0]) < self.dt)

    def testRPortSetting(self):
        neuron_model = 'iaf_psc_exp_multisynapse'
        neuron_dict = {'tau_syn': [0.5, 0.7]}
        rtype = 2
        self.pop1 = hf.nest.Create(neuron_model, self.N1, neuron_dict)
        self.pop2 = hf.nest.Create(neuron_model, self.N2, neuron_dict)
        syn_params = {'model': 'static_synapse', 'receptor_type': rtype}
        hf.nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
        conns = hf.nest.GetStatus(hf.nest.GetConnections(self.pop1, self.pop2))
        ports = [conn['receptor'] for conn in conns]
        self.assertTrue(hf.all_equal(ports))
        self.assertTrue(ports[0] == rtype)

    def testSynapseSetting(self):
        hf.nest.CopyModel("static_synapse", 'test_syn', {'receptor_type': 0})
        syn_params = {'model': 'test_syn'}
        self.setUpNetwork(self.conn_dict, syn_params)
        conns = hf.nest.GetStatus(hf.nest.GetConnections(self.pop1, self.pop2))
        syns = [str(conn['synapse_model']) for conn in conns]
        self.assertTrue(hf.all_equal(syns))
        self.assertTrue(syns[0] == syn_params['model'])

    # tested on each mpi process separatly
    def testDefaultParams(self):
        self.setUpNetwork(self.conn_dict)
        conns = hf.nest.GetStatus(hf.nest.GetConnections(self.pop1, self.pop2))
        self.assertTrue(all(x['weight'] == self.w0 for x in conns))
        self.assertTrue(all(x['delay'] == self.d0 for x in conns))
        self.assertTrue(all(x['receptor'] == self.r0 for x in conns))
        self.assertTrue(all(x['synapse_model'] == self.syn0 for x in conns))

    def testAutapsesTrue(self):
        conn_params = self.conn_dict.copy()

        # test that autapses exist
        conn_params['autapses'] = True
        self.pop1 = hf.nest.Create('iaf_psc_alpha', self.N1)
        hf.nest.Connect(self.pop1, self.pop1, conn_params)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(self.pop1, self.pop1)
        hf.mpi_assert(np.diag(M), np.ones(self.N1), self)

    def testAutapsesFalse(self):
        conn_params = self.conn_dict.copy()

        # test that autapses were excluded
        conn_params['autapses'] = False
        self.pop1 = hf.nest.Create('iaf_psc_alpha', self.N1)
        hf.nest.Connect(self.pop1, self.pop1, conn_params)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(self.pop1, self.pop1)
        hf.mpi_assert(np.diag(M), np.zeros(self.N1), self)

    def testHtSynapse(self):
        params = ['P', 'delta_P']
        values = [0.987, 0.362]
        syn_params = {'model': 'ht_synapse'}
        hf.check_synapse(params, values, syn_params, self)

    def testQuantalStpSynapse(self):
        # params a and n are not tested since Connect cannot handle
        # integer parameter yet
        # Connect will throw an error is a or n are set in syn_spec
        params = ['U', 'tau_fac', 'tau_rec', 'u']
        values = [0.679, 8.45, 746.2, 0.498]
        syn_params = {'model': 'quantal_stp_synapse'}
        hf.check_synapse(params, values, syn_params, self)

    def testStdpFacetshwSynapseHom(self):
        params = ['a_acausal', 'a_causal', 'a_thresh_th', 'a_thresh_tl',
                  'next_readout_time'
                  ]
        values = [0.162, 0.263, 20.46, 19.83, 0.1]
        syn_params = {'model': 'stdp_facetshw_synapse_hom'}
        hf.check_synapse(params, values, syn_params, self)

    def testStdpPlSynapseHom(self):
        params = ['Kplus']
        values = [0.173]
        syn_params = {'model': 'stdp_pl_synapse_hom'}
        hf.check_synapse(params, values, syn_params, self)

    def testStdpSynapseHom(self):
        params = ['Kplus']
        values = [0.382]
        syn_params = {'model': 'stdp_synapse_hom'}
        hf.check_synapse(params, values, syn_params, self)

    def testStdpSynapse(self):
        params = ['Wmax', 'alpha', 'lambda', 'mu_minus', 'mu_plus', 'tau_plus']
        values = [98.34, 0.945, 0.02, 0.945, 1.26, 19.73]
        syn_params = {'model': 'stdp_synapse'}
        hf.check_synapse(params, values, syn_params, self)

    def testTsodyks2Synapse(self):
        params = ['U', 'tau_fac', 'tau_rec', 'u', 'x']
        values = [0.362, 0.152, 789.2, 0.683, 0.945]
        syn_params = {'model': 'tsodyks2_synapse'}
        hf.check_synapse(params, values, syn_params, self)

    def testTsodyksSynapse(self):
        params = ['U', 'tau_fac', 'tau_psc', 'tau_rec', 'x', 'y', 'u']
        values = [0.452, 0.263, 2.56, 801.34, 0.567, 0.376, 0.102]
        syn_params = {'model': 'tsodyks_synapse'}
        hf.check_synapse(params, values, syn_params, self)

    def testStdpDopamineSynapse(self):
        # ResetKernel() since parameter setting not thread save for this
        # synapse type
        hf.nest.ResetKernel()
        vol = hf.nest.Create('volume_transmitter')
        hf.nest.SetDefaults('stdp_dopamine_synapse', {'vt': vol[0]})
        params = ['c', 'n']
        values = [0.153, 0.365]
        syn_params = {'model': 'stdp_dopamine_synapse'}
        hf.check_synapse(params, values, syn_params, self)

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
                vol = hf.nest.Create('volume_transmitter')
                hf.nest.SetDefaults('stdp_dopamine_synapse', {'vt': vol[0]})
            syn_params['model'] = syn
            self.pop1 = hf.nest.Create('iaf_psc_exp_multisynapse', self.N1, {
                                       'tau_syn': [0.2, 0.5]})
            self.pop2 = hf.nest.Create('iaf_psc_exp_multisynapse', self.N2, {
                                       'tau_syn': [0.2, 0.5]})
            hf.nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
            conns = hf.nest.GetStatus(
                hf.nest.GetConnections(self.pop1, self.pop2))
            conn_params = [conn['receptor'] for conn in conns]
            self.assertTrue(hf.all_equal(conn_params))
            self.assertTrue(conn_params[0] == syn_params['receptor_type'])
            self.setUp()

    def testWeightAllSynapses(self):
        # test all synapses apart from static_synapse_hom_w where weight is not
        # settable
        syns = ['cont_delay_synapse', 'ht_synapse', 'quantal_stp_synapse',
                'stdp_dopamine_synapse',
                'stdp_facetshw_synapse_hom',
                'stdp_pl_synapse_hom',
                'stdp_synapse_hom', 'stdp_synapse', 'tsodyks2_synapse',
                'tsodyks_synapse'
                ]
        syn_params = {'weight': 0.372}

        for syn in syns:
            if syn == 'stdp_dopamine_synapse':
                vol = hf.nest.Create('volume_transmitter')
                hf.nest.SetDefaults('stdp_dopamine_synapse', {'vt': vol[0]})
            syn_params['model'] = syn
            hf.check_synapse(
                ['weight'], [syn_params['weight']], syn_params, self)
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
                vol = hf.nest.Create('volume_transmitter')
                hf.nest.SetDefaults('stdp_dopamine_synapse', {'vt': vol[0]})
            syn_params['model'] = syn
            hf.check_synapse(
                ['delay'], [syn_params['delay']], syn_params, self)
            self.setUp()


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestParams)
    return suite

if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
