# -*- coding: utf-8 -*-
#
# test_clopath_stdp_synapse.py
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
Test functionality of the Clopath stdp synapse
"""

import unittest
import nest
import numpy as np

HAVE_GSL = nest.sli_func("statusdict/have_gsl ::")


@nest.check_stack
@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
class ClopathSynapseTestCase(unittest.TestCase):
    """Test Clopath stdp synapse"""

    def test_ConnectNeuronsWithClopathSynapse(self):
        """Ensures that the restriction to supported neuron models works."""

        # Specify supported models
        supported_models = [
            'aeif_cbvg_2010',
            'hh_psc_alpha_clopath',
        ]

        # Connect supported models with clopath synapse
        for nm in supported_models:
            nest.ResetKernel()

            n = nest.Create(nm, 2)

            nest.Connect(n, n, {"rule": "all_to_all"},
                         {"model": "clopath_stdp_synapse"})

        # Compute not supported models
        not_supported_models = [n for n in nest.Models(mtype='nodes')
                                if n not in supported_models]

        # Ensure that connecting not supported models fails
        for nm in not_supported_models:
            nest.ResetKernel()

            n = nest.Create(nm, 2)

            # try to connect with clopath_rule
            with self.assertRaises(nest.NESTError):
                nest.Connect(n, n, {"rule": "all_to_all"},
                             {"model": "clopath_stdp_synapse"})

    def test_SynapseDepressionFacilitation(self):
        """Ensure that depression and facilitation work correctly"""

        # This is done using the spike pairing experiment of
        # Clopath et al. 2010. First we specify the parameters
        resolution = 0.1
        nrn_params = {'V_m': -70.6,
                      'E_L': -70.6,
                      'C_m': 281.0,
                      'theta_minus': -70.6,
                      'theta_plus': -45.3,
                      'A_LTD': 14.0e-5,
                      'A_LTP': 8.0e-5,
                      'tau_minus': 10.0,
                      'tau_plus': 7.0,
                      'delay_u_bars': 4.0,
                      'a': 4.0,
                      'b': 0.0805,
                      'V_reset': -70.6 + 21.0,
                      'V_clamp': 33.0,
                      't_clamp': 2.0,
                      't_ref': 0.0, }
        spike_times_pre = [
            [29.,  129.,  229.,  329.,  429.],
            [29.,   62.3,   95.7,  129.,  162.3],
            [29.,   49.,   69.,   89.,  109.],
            [129.,  229.,  329.,  429.,  529.,  629.],
            [62.3,   95.6,  129.,  162.3,  195.6,  229.],
            [49.,   69.,   89.,  109.,  129.,  149.]]
        spike_times_post = [
            [19.,  119.,  219.,  319.,  419.],
            [19.,   52.3,   85.7,  119.,  152.3],
            [19.,  39.,  59.,  79.,  99.],
            [139.,  239.,  339.,  439.,  539.,  639.],
            [72.3,  105.6,  139.,  172.3,  205.6,  239.],
            [59.,   79.,   99.,  119.,  139.,  159.]]
        init_w = 0.5
        syn_weights = []

        # Loop over pairs of spike trains
        for (s_t_pre, s_t_post) in zip(spike_times_pre, spike_times_post):
            nest.ResetKernel()
            nest.SetKernelStatus({"resolution": resolution})

            # Create one neuron
            nrn = nest.Create("aeif_cbvg_2010", 1, nrn_params)
            prrt_nrn = nest.Create("parrot_neuron", 1)

            # Create and connect spike generator
            spike_gen_pre = nest.Create("spike_generator", 1, {
                                        "spike_times": s_t_pre})

            nest.Connect(spike_gen_pre, prrt_nrn,
                         syn_spec={"delay": resolution})

            spike_gen_params_post = {"spike_times": s_t_post}
            spike_gen_post = nest.Create("spike_generator", 1, {
                                         "spike_times": s_t_post})

            nest.Connect(spike_gen_post, nrn, syn_spec={
                         "delay": resolution, "weight": 80.0})

            # Create weight recorder
            wr = nest.Create('weight_recorder', 1)

            # Create Clopath-STDP synapse with weight recorder
            nest.CopyModel("clopath_stdp_synapse", "clopath_stdp_synapse_rec",
                           {"weight_recorder": wr[0]})
            syn_dict = {"model": "clopath_stdp_synapse_rec",
                        "weight": init_w, "delay": resolution}
            nest.Connect(prrt_nrn, nrn, syn_spec=syn_dict)

            # Simulation
            simulation_time = (10.0 + max(s_t_pre[-1], s_t_post[-1]))
            nest.Simulate(simulation_time)

            # Evaluation
            w_events = nest.GetStatus(wr)[0]["events"]
            weights = w_events["weights"]
            syn_weights.append(weights[-1])

        # Compare to expected result
        syn_weights = np.array(syn_weights)
        syn_weights = 100.0*15.0*(syn_weights - init_w)/init_w + 100.0
        correct_weights = [60.27717273, 72.83202162, 141.91383624,
                           102.74121415, 120.01918347, 148.76674224]

        self.assertTrue(np.allclose(syn_weights, correct_weights, rtol=1e-7))

    def test_SynapseFunctionWithAeifModel(self):
        """Ensure that spikes are properly processed"""

        nest.ResetKernel()

        # Create neurons and devices
        nrns = nest.Create('aeif_cbvg_2010', 2, {'V_m': -70.6})
        prrt_nrn = nest.Create('parrot_neuron', 1)

        spike_times = [10.0]
        sg = nest.Create('spike_generator', 1, {'spike_times': spike_times})

        mm = nest.Create('multimeter', params={
                         'record_from': ['V_m'], 'interval': 1.0})

        nest.Connect(sg, prrt_nrn)
        nest.Connect(mm, nrns)

        # Connect one neuron with static connection
        conn_dict = {'rule': 'all_to_all'}
        static_syn_dict = {'model': 'static_synapse',
                           'weight': 2.0, 'delay': 1.0}
        nest.Connect(prrt_nrn, nrns[0:1], conn_dict, static_syn_dict)

        # Connect one neuron with Clopath stdp connection
        cl_stdp_syn_dict = {'model': 'clopath_stdp_synapse',
                            'weight': 2.0, 'delay': 1.0}
        nest.Connect(prrt_nrn, nrns[1:2], conn_dict, cl_stdp_syn_dict)

        # Simulation
        nest.Simulate(20.)

        # Evaluation
        data = nest.GetStatus(mm)
        senders = data[0]['events']['senders']
        voltages = data[0]['events']['V_m']

        vm1 = voltages[np.where(senders == 1)]
        vm2 = voltages[np.where(senders == 2)]

        # Compare results for static synapse and Clopath stdp synapse
        self.assertTrue(np.allclose(vm1, vm2, rtol=1e-5))
        # Check that a spike with weight 2.0 is processes properly
        # in the aeif_cbvg_2010 model
        self.assertTrue(np.isclose(vm2[11]-vm2[10], 2.0, rtol=1e-5))


def suite():
    suite = unittest.makeSuite(ClopathSynapseTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
