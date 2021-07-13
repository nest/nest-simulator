# -*- coding: utf-8 -*-
#
# test_urbanczik_synapse.py
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

"""
Test functionality of the Urbanczik synapse
"""

import unittest
import nest
import numpy as np

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")


@nest.ll_api.check_stack
@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
class UrbanczikSynapseTestCase(unittest.TestCase):
    """Test Urbanczik synapse"""

    def test_ConnectNeuronsWithUrbanczikSynapse(self):
        """Ensures that the restriction to supported neuron models works."""

        nest.set_verbosity('M_WARNING')

        # Multi-compartment models
        mc_models = [
            "iaf_cond_alpha_mc",
            "pp_cond_exp_mc_urbanczik",
        ]

        # Supported models
        supported_models = [
            "pp_cond_exp_mc_urbanczik",
        ]

        # Compute not supported models
        not_supported_models = [n for n in nest.Models(mtype='nodes')
                                if n not in supported_models]

        # Connect supported models with Urbanczik synapse
        for nm in supported_models:
            nest.ResetKernel()

            r_type = 0
            if nm in mc_models:
                syns = nest.GetDefaults(nm)["receptor_types"]
                r_type = syns["soma_exc"]

            n = nest.Create(nm, 2)

            nest.Connect(n, n, {"rule": "all_to_all"},
                         {"synapse_model": "urbanczik_synapse", "receptor_type": r_type})

        # Ensure that connecting not supported models fails
        for nm in not_supported_models:
            nest.ResetKernel()

            r_type = 0
            if nm in mc_models:
                syns = nest.GetDefaults(nm)["receptor_types"]
                r_type = syns["soma_exc"]

            n = nest.Create(nm, 2)

            # try to connect with urbanczik synapse
            with self.assertRaises(nest.kernel.NESTError):
                nest.Connect(n, n, {"rule": "all_to_all"},
                             {"synapse_model": "urbanczik_synapse", "receptor_type": r_type})

    def test_SynapseDepressionFacilitation(self):
        """Ensure that depression and facilitation work correctly"""

        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()

        resolution = 0.1
        nest.SetKernelStatus({'resolution': resolution})

        '''
        neuron parameters
        '''
        nrn_model = 'pp_cond_exp_mc_urbanczik'
        nrn_params = {
            't_ref': 3.0,        # refractory period
            'g_sp': 600.0,       # somato-dendritic coupling conductance
            'soma': {
                'V_m': -70.0,    # initial value of V_m
                'C_m': 300.0,    # capacitance of membrane
                'E_L': -70.0,    # resting potential
                'g_L': 30.0,     # somatic leak conductance
                'E_ex': 0.0,     # resting potential for exc input
                'E_in': -75.0,   # resting potential for inh input
                'tau_syn_ex': 3.0,  # time constant of exc conductance
                'tau_syn_in': 3.0,  # time constant of inh conductance
            },
            'dendritic': {
                'V_m': -70.0,    # initial value of V_m
                'C_m': 300.0,    # capacitance of membrane
                'E_L': -70.0,    # resting potential
                'g_L': 30.0,     # dendritic leak conductance
                'tau_syn_ex': 3.0,  # time constant of exc input current
                'tau_syn_in': 3.0,  # time constant of inh input current
            },
            # parameters of rate function
            'phi_max': 0.15,     # max rate
            'rate_slope': 0.5,   # called 'k' in the paper
            'beta': 1.0 / 3.0,
            'theta': -55.0,
        }

        '''
        synapse params
        '''
        syns = nest.GetDefaults(nrn_model)['receptor_types']
        init_w = 100.0
        syn_params = {
            'synapse_model': 'urbanczik_synapse_wr',
            'receptor_type': syns['dendritic_exc'],
            'tau_Delta': 100.0,  # time constant of low pass filtering of the weight change
            'eta': 0.75,         # learning rate
            'weight': init_w,
            'Wmax': 4.5*nrn_params['dendritic']['C_m'],
            'delay': resolution,
        }

        '''
        neuron and devices
        '''
        nest.SetDefaults(nrn_model, nrn_params)
        nrn = nest.Create(nrn_model)

        # spike generator is connected to a parrot neuron which is connected to the mc neuron
        prrt_nrn = nest.Create('parrot_neuron')

        # excitatory input to the dendrite
        pre_syn_spike_times = np.array([1.0, 98.0])
        sg_prox = nest.Create('spike_generator', params={
                              'spike_times': pre_syn_spike_times})

        # excitatory input to the soma
        spike_times_soma_inp = np.arange(10.0, 50.0, resolution)
        spike_weights_soma = 10.0*np.ones_like(spike_times_soma_inp)
        sg_soma_exc = nest.Create('spike_generator',
                                  params={'spike_times': spike_times_soma_inp, 'spike_weights': spike_weights_soma})

        # for recording all parameters of the Urbanczik neuron
        rqs = nest.GetDefaults(nrn_model)['recordables']
        mm = nest.Create('multimeter', params={
                         'record_from': rqs, 'interval': 0.1})

        # for recoding the synaptic weights of the Urbanczik synapses
        wr = nest.Create('weight_recorder')

        # for recording the spiking of the soma
        sr_soma = nest.Create('spike_recorder')

        '''
        create connections
        '''
        nest.Connect(sg_prox, prrt_nrn, syn_spec={'delay': resolution})
        nest.CopyModel('urbanczik_synapse', 'urbanczik_synapse_wr',
                       {'weight_recorder': wr[0]})
        nest.Connect(prrt_nrn, nrn, syn_spec=syn_params)
        nest.Connect(sg_soma_exc, nrn,
                     syn_spec={'receptor_type': syns['soma_exc'], 'weight': 10.0*resolution, 'delay': resolution})
        nest.Connect(mm, nrn, syn_spec={'delay': resolution})
        nest.Connect(nrn, sr_soma, syn_spec={'delay': resolution})

        '''
        simulation
        '''
        nest.Simulate(100.0)

        '''
        read out devices
        '''
        # multimeter
        rec = nest.GetStatus(mm)[0]['events']
        t = rec['times']
        V_w = rec['V_m.p']

        # compute dendritic prediction of somatic membrane potential
        g_D = nrn_params['g_sp']
        g_L = nrn_params['soma']['g_L']
        E_L = nrn_params['soma']['E_L']
        V_w_star = (g_L*E_L + g_D*V_w) / (g_L + g_D)

        # weight recorder
        data = nest.GetStatus(wr)
        senders = data[0]['events']['senders']
        targets = data[0]['events']['targets']
        weights = data[0]['events']['weights']
        times = data[0]['events']['times']

        # spike recorder
        data = nest.GetStatus(sr_soma)[0]['events']
        spike_times_soma = data['times']

        # compute predicted rate
        phi_max = nrn_params['phi_max']
        k = nrn_params['rate_slope']
        beta = nrn_params['beta']
        theta = nrn_params['theta']
        rate = (phi_max / (1.0 + k*np.exp(beta*(theta - V_w_star))))

        # compute h(V_w_star)
        h = (15.0*beta / (1.0 + np.exp(-beta*(theta - V_w_star)) / k))

        # compute alpha response kernel
        tau_s = nrn_params['dendritic']['tau_syn_ex']
        g_L_prox = nrn_params['dendritic']['g_L']
        C_m_prox = nrn_params['dendritic']['C_m']
        tau_L = C_m_prox / g_L_prox
        E_L_prox = nrn_params['dendritic']['E_L']
        t0 = 1.2
        alpha_response = (np.heaviside(t - t0, 0.5)*tau_s*(np.exp(-(t - t0) / tau_L) - np.exp(-(t - t0) / tau_s)) /
                          (g_L_prox*(tau_L - tau_s)))

        # compute PI(t)
        if len(spike_times_soma) > 0:
            t = np.around(t, 4)
            spike_times_soma = np.around(spike_times_soma + 0.2, 4)
            idx = np.nonzero(np.in1d(t, spike_times_soma))[0]
            rate[idx] -= 1.0 / resolution

        w_change_raw = -15.0*C_m_prox*rate*h*alpha_response

        # compute low pass filtered version of PI
        tau_Delta = syn_params['tau_Delta']
        eta = syn_params['eta']
        w_change_low_pass = eta * np.exp(-t / tau_Delta)*np.cumsum(
            np.exp(t / tau_Delta)*w_change_raw)*resolution / tau_Delta
        integrated_w_change = np.cumsum(w_change_low_pass)*resolution
        syn_weight_comp = init_w + integrated_w_change

        '''
        comparison between Nest and python implementation
        '''
        # extract the weight computed in python at the times of the presynaptic spikes
        idx = np.nonzero(np.in1d(np.around(t, 4), np.around(pre_syn_spike_times + resolution, 4)))[0]
        syn_w_comp_at_spike_times = syn_weight_comp[idx]
        realtive_error = (
            (weights[-1] - syn_w_comp_at_spike_times[-1]) / (weights[-1] - init_w))

        self.assertTrue(abs(realtive_error) < 0.001)


def suite():
    suite = unittest.makeSuite(UrbanczikSynapseTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
