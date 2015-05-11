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
import nest
from . import test_connect_helpers as hf

class TestParams(unittest.TestCase):

    ## Setting default parameter. These parameter might be overwritten by the classes
    ## testing one specific rule.
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
    pval = 0.05 # minimum p-value to pass kolmogorov smirnov test
    # number of threads
    nr_threads = 2

    # for now only tests if a multi-thread connect is successfull, not whether the the threading is actually used
    def setUp(self):
        nest.ResetKernel()
        nest.SetKernelStatus({'local_num_threads': self.nr_threads})
        #pass
       
    def setUpNetwork(self, conn_dict=None, syn_dict=None, N1=None, N2=None):
        if N1 == None:
            N1 = self.N1
        if N2 == None:
            N2 = self.N2
        self.pop1 = nest.Create('iaf_neuron', N1)
        self.pop2 = nest.Create('iaf_neuron', N2)
        nest.set_verbosity('M_FATAL')
        nest.Connect(self.pop1, self.pop2, conn_dict, syn_dict)
                
    def testWeightSetting(self):
        ## test if weights are set correctly
        
        # one weight for all connections
        w0 = 0.351
        syn_params = {'weight': w0}
        self.setUpNetwork(conn_dict=self.conn_dict,syn_dict=syn_params)
        connections =  nest.GetStatus(nest.GetConnections(self.pop1, self.pop2))
        nest_weights = [connection['weight'] for connection in connections]
        self.assertTrue(hf.mpi_assert(nest_weights, w0))
        
        # TODO: write test for insertion of generell weight iterator
        
    def testDelaySetting(self):
        ## test if delays are set correctly
        
        # one delay for all connections
        d0 = 0.275
        syn_params = {'delay': d0}
        self.setUpNetwork(self.conn_dict, syn_params)
        connections =  nest.GetStatus(nest.GetConnections(self.pop1, self.pop2))
        nest_delays = [connection['delay'] for connection in connections]
        # all delays need to be equal
        self.assertTrue(hf.mpi_assert(nest_delays, nest_delays[0]))
        # delay (rounded) needs to equal the delay that was put in
        self.assertTrue(abs(d0-nest_delays[0]) < self.dt)

    def testRPortSetting(self):
        neuron_model = 'iaf_psc_exp_multisynapse'
        neuron_dict = {'tau_syn': [0.5, 0.7]}
        rtype = 2
        self.pop1 = nest.Create(neuron_model, self.N1, neuron_dict)
        self.pop2 = nest.Create(neuron_model, self.N2, neuron_dict)       
        syn_params = {'model': 'static_synapse', 'receptor_type': rtype}
        nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
        conns = nest.GetStatus(nest.GetConnections(self.pop1, self.pop2))
        ports = [conn['receptor'] for conn in conns]
        self.assertTrue(hf.mpi_assert(ports, rtype))

    def testSynapseSetting(self):
        nest.CopyModel("static_synapse", 'test_syn', {'receptor_type': 0})
        syn_params = {'model': 'test_syn'}
        self.setUpNetwork(self.conn_dict, syn_params)
        conns = nest.GetStatus(nest.GetConnections(self.pop1, self.pop2))
        syns = [str(conn['synapse_model']) for conn in conns]
        self.assertTrue(hf.mpi_assert(syns, syn_params['model']))

    # tested on each mpi process separatly
    def testDefaultParams(self):
        self.setUpNetwork(self.conn_dict)
        conns = nest.GetStatus(nest.GetConnections(self.pop1, self.pop2))
        self.assertTrue(all(x['weight'] == self.w0 for x in conns))
        self.assertTrue(all(x['delay'] == self.d0 for x in conns))
        self.assertTrue(all(x['receptor'] == self.r0 for x in conns))
        self.assertTrue(all(x['synapse_model'] == self.syn0 for x in conns))

    def testAutapses(self):
        conn_params = self.conn_dict.copy()
        
        # test that autapses exist
        conn_params['autapses'] = True        
        self.pop1 = nest.Create('iaf_neuron', self.N1)
        nest.Connect(self.pop1, self.pop1, conn_params)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(self.pop1, self.pop1)
        self.assertTrue(hf.mpi_assert(M, np.ones(self.N1), 'diagonal')) 
        nest.ResetKernel()

        # test that autapses were excluded
        conn_params['autapses'] = False        
        self.pop1 = nest.Create('iaf_neuron', self.N1)
        nest.Connect(self.pop1, self.pop1, conn_params)
        # make sure all connections do exist
        M = hf.get_connectivity_matrix(self.pop1, self.pop1)
        self.assertTrue(hf.mpi_assert(M, np.zeros(self.N1), 'diagonal'))

    def testHtSynapse(self):
        params = ['P', 'delta_P']
        values = [0.987, 0.362]
        for i, param in enumerate(params):
            syn_params = {'model': 'ht_synapse'}
            syn_params[param] = values[i]
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, param)
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*values[i]))

    def testQuantalStpSynapse(self):
        # params a and n are not tested since Connect cannot handle integer parameter yet
        # Connect will throw an error is a or n are set in syn_spec
        params = ['U', 'tau_fac', 'tau_rec', 'u']
        values = [0.679, 8.45, 746.2, 0.498]
        for i, param in enumerate(params):
            syn_params = {'model': 'quantal_stp_synapse'}
            syn_params[param] = values[i]
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, param)
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*values[i])) 

    def testStdpFacetshwSynapseHom(self):
        params = [ 'a_acausal', 'a_causal', 'a_thresh_th', 'a_thresh_tl',
                   'next_readout_time'
                 ]
        values = [0.162, 0.263, 20.46, 19.83, 0.1]
        for i, param in enumerate(params):
            syn_params = {'model': 'stdp_facetshw_synapse_hom'}
            syn_params[param] = values[i]
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, param)
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*values[i])) 

    def testStdpPlSynapseHom(self):
        params = ['Kplus']
        values = [0.173]
        for i, param in enumerate(params):
            syn_params = {'model': 'stdp_pl_synapse_hom'}
            syn_params[param] = values[i]
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, param)
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*values[i]))
            
    def testStdpSynapseHom(self):
        params = ['Kplus']
        values = [0.382]
        for i, param in enumerate(params):
            syn_params = {'model': 'stdp_synapse_hom'}
            syn_params[param] = values[i]
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, param)
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*values[i]))
            
    def testStdpSynapse(self):
        params = ['Wmax', 'alpha', 'lambda', 'mu_minus', 'mu_plus', 'tau_plus']
        values = [98.34, 0.945, 0.02, 0.945, 1.26, 19.73]
        for i, param in enumerate(params):
            syn_params = {'model': 'stdp_synapse'}
            syn_params[param] = values[i]
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, param)
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*values[i]))
            
    def testTsodyks2Synapse(self):
        params = ['U', 'tau_fac', 'tau_rec', 'u', 'x']
        values = [0.362, 0.152, 789.2, 0.683, 0.945]
        for i, param in enumerate(params):
            syn_params = {'model': 'tsodyks2_synapse'}
            syn_params[param] = values[i]
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, param)
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*values[i]))
            
    def testTsodyksSynapse(self):
        params = ['U', 'tau_fac', 'tau_psc', 'tau_rec', 'x', 'y', 'u']
        values = [0.452, 0.263, 2.56, 801.34, 0.567, 0.376, 0.102]
        for i, param in enumerate(params):
            syn_params = {'model': 'tsodyks_synapse'}
            if param == 'y':
                syn_params['x'] = 0.98 - values[i] 
            syn_params[param] = values[i]
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, param)
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*values[i]))
            
    def testStdpDopamineSynapse(self):
        # ResetKernel() since parameter setting not thread save for this synapse type
        nest.ResetKernel()
        vol = nest.Create('volume_transmitter')
        nest.SetDefaults('stdp_dopamine_synapse',{'vt':vol[0]})
        params = ['c', 'n']
        values = [0.153, 0.365]
        for i, param in enumerate(params):
            syn_params = {'model': 'stdp_dopamine_synapse'}
            syn_params[param] = values[i]
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, param)
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*values[i]))
            
    def testRPortAllSynapses(self):
        syns = [ 'cont_delay_synapse', 'ht_synapse', 'quantal_stp_synapse',
                 'static_synapse_hom_w', 'stdp_dopamine_synapse',
                 'stdp_facetshw_synapse_hom', 'stdp_pl_synapse_hom',
                 'stdp_synapse_hom', 'stdp_synapse', 'tsodyks2_synapse',
                 'tsodyks_synapse'
               ]
        syn_params = {'receptor_type': 1}
            
        for i, syn in enumerate(syns):
            if syn == 'stdp_dopamine_synapse':
                vol = nest.Create('volume_transmitter')
                nest.SetDefaults('stdp_dopamine_synapse',{'vt':vol[0]})
            syn_params['model'] = syn
            self.pop1 = nest.Create('iaf_psc_exp_multisynapse', self.N1, {'tau_syn': [0.2,0.5]})
            self.pop2 = nest.Create('iaf_psc_exp_multisynapse', self.N2, {'tau_syn': [0.2,0.5]})
            nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, 'receptor')
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))))
            nest.ResetKernel()
            self.setUp

    def testWeightAllSynapses(self):
        # test all synapses apart from static_synapse_hom_w where weight is not settable
        syns = [ 'cont_delay_synapse', 'ht_synapse', 'quantal_stp_synapse',
                 'stdp_dopamine_synapse',
                 'stdp_facetshw_synapse_hom',
                 'stdp_pl_synapse_hom',
                 'stdp_synapse_hom', 'stdp_synapse', 'tsodyks2_synapse',
                 'tsodyks_synapse'
               ]
        syn_params = {'weight': 0.372}

        for i, syn in enumerate(syns):
            if syn == 'stdp_dopamine_synapse':
                vol = nest.Create('volume_transmitter')
                nest.SetDefaults('stdp_dopamine_synapse',{'vt':vol[0]})
            syn_params['model'] = syn
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, 'weight')
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*syn_params['weight']))
            self.setUp()

    def testDelayAllSynapses(self):
        syns = [ 'cont_delay_synapse',
                 'ht_synapse', 'quantal_stp_synapse',
                 'static_synapse_hom_w',
                 'stdp_dopamine_synapse',
                 'stdp_facetshw_synapse_hom', 'stdp_pl_synapse_hom',
                 'stdp_synapse_hom', 'stdp_synapse', 'tsodyks2_synapse',
                 'tsodyks_synapse'
               ]
        syn_params = {'delay': 0.4}

        for i, syn in enumerate(syns):
            if syn == 'stdp_dopamine_synapse':
                vol = nest.Create('volume_transmitter')
                nest.SetDefaults('stdp_dopamine_synapse',{'vt':vol[0]})
            syn_params['model'] = syn
            self.setUpNetwork(self.conn_dict, syn_params)
            a = hf.get_weighted_connection_array(self.pop1, self.pop2, 'delay')
            self.assertTrue(hf.mpi_assert(a, np.ones(len(a))*syn_params['delay']))
            self.setUp()

    
if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestParams)
    unittest.TextTestRunner(verbosity=2).run(suite)

