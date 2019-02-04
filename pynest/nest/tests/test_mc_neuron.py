# -*- coding: utf-8 -*-
#
# test_mc_neuron.py
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
import nest
import numpy as np

HAVE_GSL = nest.sli_func("statusdict/have_gsl ::")


@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
class TestMCNeuron(unittest.TestCase):

    # neuron parameter
    V_th = -60.0           # threshold potential
    V_reset = -65.0        # reset potential
    t_ref = 10.0           # refractory period
    g_sp = 5.0             # somato-proximal coupling conductance
    soma = {'g_L': 12.0}  # somatic leak conductance
    tau_syn_ex = 1.0       # proximal excitatory synaptic time constants
    tau_syn_in = 5.0       # proximal inhibitory synaptic time constants
    proximal = {'tau_syn_ex': tau_syn_ex, 'tau_syn_in': tau_syn_in}
    distal = {'C_m': 90.0}  # distal capacitance
    # paramter for recording time of compartements
    rec_dic_dc_soma = {'start': 250.0, 'stop': 300.0, 'amplitude':  50.0}
    rec_dic_dc_proximal = {'start': 150.0, 'stop': 200.0, 'amplitude': -50.0}
    rec_dic_dc_distal = {'start':  50.0, 'stop': 100.0, 'amplitude': 100.0}
    rec_sp_soma_ex = [600.0, 620.0]  # soma excitatory
    rec_sp_soma_in = [610.0, 630.0]  # soma inhibitory
    rec_sp_prox_ex = [500.0, 520.0]  # proximal excitatory
    rec_sp_prox_in = [510.0, 530.0]  # proximal inhibitory
    rec_sp_dist_ex = [400.0, 420.0]  # distal excitatory
    rec_sp_dist_in = [410.0, 430.0]  # distal inhibitory
    # simulation time
    t0 = 700       # simulation time of initial simulation
    t_stim = 300   # simulation time of with stimulation of soma
    # strength of instrinsic current in soma
    I_e = 150.0
    # test data
    Vm_soma_test = np.asarray([-70.00033871, -70.00033626, -70.00033383,
                               -70.00033141, -70.00032901, -70.00032662,
                               -70.00032426, -70.00032191, -70.00031957,
                               -70.00031725, -70.00030162, -70.00021189,
                               -69.99998874, -69.99958683, -69.99897263,
                               -69.99812256, -69.99702137, -69.99566068,
                               -69.99403779, -69.99215463])
    Vm_prox_test = np.asarray([-70.00041394, -70.00041065, -70.00040738,
                               -70.00040414, -70.00040093, -70.00039774,
                               -70.00039457, -70.00039143, -70.00038832,
                               -70.00038523, -69.98857137, -69.95638129,
                               -69.90815375, -69.84758846, -69.77782368,
                               -69.70150641, -69.62085572, -69.53771917,
                               -69.45362308, -69.36981694])
    Vm_dist_test = np.asarray([-70.0003797,  -70.00037554, -70.00037143,
                               -70.00036737, -70.00036335, -70.00035938,
                               -70.00035545, -70.00035157, -70.00034773,
                               -70.00034393, -70.00033573, -70.00030289,
                               -70.00022565, -70.00008895, -69.99988167,
                               -69.99959601, -69.99922696, -69.9987718,
                               -69.9982297,  -69.99760137])
    gex_soma_test = np.zeros(20)
    gex_prox_test = np.array([0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                              0.24596033, 0.44510821, 0.60412585,
                              0.72884756, 0.82436068, 0.89509487,
                              0.94490122, 0.97712226, 0.99465388,
                              1.00000005])
    gex_dist_test = np.zeros(20)
    # position of test data
    I0, I1 = 5000, 5020

    def setUp(self):
        nest.ResetKernel()
        nest.SetDefaults('iaf_cond_alpha_mc',
                         {'V_th': self.V_th,
                          'V_reset': self.V_reset,
                          't_ref': self.t_ref,
                          'g_sp': self.g_sp,
                          'soma': self.soma,
                          'proximal': self.proximal,
                          'distal':  self.distal
                          })

    def setUpNodes(self):
        self.n = nest.Create('iaf_cond_alpha_mc')
        rqs = nest.GetDefaults('iaf_cond_alpha_mc')['recordables']
        self.mm = nest.Create('multimeter', params={'record_from': rqs,
                                                    'interval': 0.1})
        self.cgs = nest.Create('dc_generator', 3)
        nest.SetStatus(self.cgs, [self.rec_dic_dc_soma,
                                  self.rec_dic_dc_proximal,
                                  self.rec_dic_dc_distal])
        self.sgs = nest.Create('spike_generator', 6)
        nest.SetStatus(self.sgs, [{'spike_times': self.rec_sp_soma_ex},
                                  {'spike_times': self.rec_sp_soma_in},
                                  {'spike_times': self.rec_sp_prox_ex},
                                  {'spike_times': self.rec_sp_prox_in},
                                  {'spike_times': self.rec_sp_dist_ex},
                                  {'spike_times': self.rec_sp_dist_in}])

    def setUpNetwork(self):
        syns = nest.GetDefaults('iaf_cond_alpha_mc')['receptor_types']
        label = ['soma_curr', 'proximal_curr', 'distal_curr', 'soma_exc',
                 'soma_inh', 'proximal_exc', 'proximal_inh', 'distal_exc',
                 'distal_inh']
        nest.Connect(self.mm, self.n)
        for i, l in enumerate(label[:3]):
            nest.Connect([self.cgs[i]], self.n,
                         syn_spec={'receptor_type': syns[l]})
        for i, l in enumerate(label[3:]):
            nest.Connect([self.sgs[i]], self.n,
                         syn_spec={'receptor_type': syns[l]})

    def testNeuron(self):
        self.setUpNodes()
        self.setUpNetwork()
        nest.Simulate(self.t0)
        nest.SetStatus(self.n, {'soma': {'I_e': self.I_e}})
        nest.Simulate(self.t_stim)
        rec = nest.GetStatus(self.mm)[0]['events']
        # test membrane potential recorded in the soma
        self.assertTrue(np.allclose(rec['V_m.s'][self.I0:self.I1],
                                    self.Vm_soma_test))
        # test membrane potential in the proximal compartment
        self.assertTrue(np.allclose(rec['V_m.p'][self.I0:self.I1],
                                    self.Vm_prox_test))
        # test membrane potential in the distal compartment
        self.assertTrue(np.allclose(rec['V_m.d'][self.I0:self.I1],
                                    self.Vm_dist_test))
        # test conductance recorded in the soma
        self.assertTrue(np.allclose(rec['g_ex.s'][self.I0:self.I1],
                                    self.gex_soma_test))
        # test conductance in the proximal compartment
        self.assertTrue(np.allclose(rec['g_ex.p'][self.I0:self.I1],
                                    self.gex_prox_test))
        # test conductance in the distal compartment
        self.assertTrue(np.allclose(rec['g_ex.d'][self.I0:self.I1],
                                    self.gex_dist_test))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestMCNeuron)
    return suite

if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
