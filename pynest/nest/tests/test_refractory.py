# -*- coding: utf-8 -*-
#
# test_refractory.py
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

import nest

"""
Assert that all neuronal models that have a refractory period implement it
correctly (except for Hodgkin-Huxley models which cannot be tested).

Details
-------
Submit the neuron to a constant excitatory current so that it spikes in the
[0, 50] ms.
A ``spike_detector`` is used to detect the time at which the neuron spikes and
a ``voltmeter`` is then used to make sure the voltage is clamped to ``V_reset``
during exactly ``t_ref``.

For neurons that do not clamp the potential, use a very large current to
trigger immediate spiking

Untested models
---------------
* ``aeif_cond_alpha_RK5``
* ``amat2_psc_exp``
* ``erfc_neuron``
* ``gauss_rate_ipn``
* ``gif_pop_psc_exp``
* ``ginzburg_neuron``
* ``hh_cond_exp_traub``
* ``hh_psc_alpha``
* ``hh_psc_alpha_gap``
* ``iaf_chs_2007``
* ``iaf_chxk_2008``
* ``iaf_psc_exp_ps_lossless``
* ``iaf_tum_2000``
* ``izhikevich``
* ``lin_rate_ipn``
* ``lin_rate_opn``
* ``mat2_psc_exp``
* ``mcculloch_pitts_neuron``
* ``music_cont_in_proxy``
* ``music_cont_out_proxy``
* ``music_event_in_proxy``
* ``music_event_out_proxy``
* ``music_message_in_proxy``
* ``parrot_neuron``
* ``parrot_neuron_ps``
* ``pp_pop_psc_delta``
* ``pp_psc_delta``
* ``rate_transformer_gauss``
* ``rate_transformer_lin``
* ``rate_transformer_sigmoid``
* ``rate_transformer_sigmoid_gg_1998``
* ``rate_transformer_tanh``
* ``rate_transformer_threshold_lin``
* ``sli_neuron``
* ``siegert_neuron``
* ``sigmoid_rate_gg_1998_ipn``
* ``sigmoid_rate_ipn``
* ``tanh_rate_ipn``
* ``tanh_rate_opn``
* ``threshold_lin_rate_ipn``
* ``threshold_lin_rate_opn``
"""


# --------------------------------------------------------------------------- #
#  Models, specific parameters
# --------------------------------------------------------------------------- #

# list of all neuronal models that can be tested by looking at clamped V
neurons_V_clamped = [
    'aeif_cond_alpha',
    'aeif_cond_alpha_multisynapse',
    'aeif_cond_beta_multisynapse',
    'aeif_cond_exp',
    'aeif_psc_alpha',
    'aeif_psc_exp',
    'gif_cond_exp',
    'gif_cond_exp_multisynapse',
    'gif_psc_exp',
    'gif_psc_exp_multisynapse',
    'iaf_cond_alpha',
    'iaf_cond_alpha_mc',
    'iaf_cond_exp',
    'iaf_cond_exp_sfa_rr',
    'iaf_psc_alpha',
    'iaf_psc_alpha_multisynapse',
    'iaf_psc_delta',
    'iaf_psc_exp',
    'iaf_psc_exp_multisynapse',
]

# neurons that must be tested through a high current to spike immediately
# (t_ref = interspike)
neurons_interspike = [
    "ht_neuron",
]

neurons_interspike_ps = [
    "iaf_psc_alpha_canon",
    "iaf_psc_alpha_presc",
    "iaf_psc_delta_canon",
    "iaf_psc_exp_ps",
]

# models that cannot be tested
ignore_model = [
    "aeif_cond_alpha_RK5",  # this one is faulty and will be removed
    "amat2_psc_exp",
    "erfc_neuron",
    "gauss_rate_ipn",
    "gif_pop_psc_exp",
    "ginzburg_neuron",
    "hh_cond_exp_traub",
    "hh_psc_alpha",
    "hh_psc_alpha_gap",
    "iaf_chs_2007",
    "iaf_chxk_2008",
    "iaf_psc_exp_ps_lossless",
    "iaf_tum_2000",
    "izhikevich",
    "lin_rate_ipn",
    "lin_rate_opn",
    "mat2_psc_exp",
    "mcculloch_pitts_neuron",
    "music_cont_in_proxy",
    "music_cont_out_proxy",
    "music_event_in_proxy",
    "music_event_out_proxy",
    "music_message_in_proxy",
    "parrot_neuron",
    "parrot_neuron_ps",
    "pp_pop_psc_delta",
    "pp_psc_delta",
    "rate_transformer_gauss",
    "rate_transformer_lin",
    "rate_transformer_sigmoid",
    "rate_transformer_sigmoid_gg_1998",
    "rate_transformer_tanh",
    "rate_transformer_threshold_lin",
    "sli_neuron",
    "siegert_neuron",
    "sigmoid_rate_gg_1998_ipn",
    "sigmoid_rate_ipn",
    "tanh_rate_ipn",
    "tanh_rate_opn",
    "threshold_lin_rate_ipn",
    "threshold_lin_rate_opn",
]

tested_models = [m for m in nest.Models("nodes") if (nest.GetDefaults(
                 m, "element_type") == "neuron" and m not in ignore_model)]

# additional parameters for the connector
add_connect_param = {
    "iaf_cond_alpha_mc": {"receptor_type": 7},
}


# --------------------------------------------------------------------------- #
#  Simulation time and refractory time limits
# --------------------------------------------------------------------------- #

simtime = 100
resolution = 0.1
min_steps = 1    # minimal number of refractory steps (t_ref = resolution)
max_steps = 200  # maximal number of steps (t_ref = 200 * resolution)


# --------------------------------------------------------------------------- #
#  Test class
# --------------------------------------------------------------------------- #


class TestRefractoryCase(unittest.TestCase):
    """
    Check the correct implementation of refractory time in all neuronal models.
    """

    def reset(self):
        nest.ResetKernel()

        msd = 123456
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        pyrngs = [np.random.RandomState(s) for s in range(msd, msd + N_vp)]

        nest.SetKernelStatus({
            'resolution': resolution, 'grng_seed': msd + N_vp,
            'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

    def compute_reftime(self, model, sd, vm, neuron):
        '''
        Compute the refractory time of the neuron.

        Parameters
        ----------
        model : str
          Name of the neuronal model.
        sd : tuple
            GID of the spike detector.
        vm : tuple
            GID of the voltmeter.
        neuron : tuple
            GID of the recorded neuron.

        Returns
        -------
        t_ref_sim : double
            Value of the simulated refractory period.
        '''
        spike_times = nest.GetStatus(sd, "events")[0]["times"]
        if model in neurons_interspike:
            # spike emitted at next timestep so substract resolution
            return spike_times[1]-spike_times[0]-resolution
        elif model in neurons_interspike_ps:
            return spike_times[1]-spike_times[0]
        else:
            Vr = nest.GetStatus(neuron, "V_reset")[0]
            times = nest.GetStatus(vm, "events")[0]["times"]
            # index of the 2nd spike
            idx_max = np.argwhere(times == spike_times[1])[0][0]
            name_Vm = "V_m.s" if model == "iaf_cond_alpha_mc" else "V_m"
            Vs = nest.GetStatus(vm, "events")[0][name_Vm]
            # get the index at which the spike occured
            idx_spike = np.argwhere(times == spike_times[0])[0][0]
            # find end of refractory period between 1st and 2nd spike
            idx_end = np.where(
                np.isclose(Vs[idx_spike:idx_max], Vr, 1e-6))[0][-1]
            t_ref_sim = idx_end * resolution
            return t_ref_sim

    def test_refractory_time(self):
        '''
        Check that refractory time implementation is correct.
        '''
        for model in tested_models:
            self.reset()
            # randomly set a refractory period
            t_ref = resolution * np.random.randint(min_steps, max_steps)
            # create the neuron and devices
            nparams = {"t_ref": t_ref}
            neuron = nest.Create(model, params=nparams)
            name_Vm = "V_m.s" if model == "iaf_cond_alpha_mc" else "V_m"
            vm_params = {"interval": resolution, "record_from": [name_Vm]}
            vm = nest.Create("voltmeter", params=vm_params)
            sd = nest.Create("spike_detector", params={'precise_times': True})
            cg = nest.Create("dc_generator", params={"amplitude": 900.})
            # for models that do not clamp V_m, use very large current to
            # trigger almost immediate spiking => t_ref almost equals
            # interspike
            if model in neurons_interspike_ps:
                nest.SetStatus(cg, "amplitude", 10000000.)
            elif model in neurons_interspike:
                nest.SetStatus(cg, "amplitude", 2000.)
            # connect them and simulate
            nest.Connect(vm, neuron)
            nest.Connect(cg, neuron, syn_spec=add_connect_param.get(model, {}))
            nest.Connect(neuron, sd)

            nest.Simulate(simtime)

            # get and compare t_ref
            t_ref_sim = self.compute_reftime(model, sd, vm, neuron)

            # approximate result for precise spikes (interpolation error)
            if model in neurons_interspike_ps:
                self.assertAlmostEqual(t_ref, t_ref_sim, places=3,
                                       msg='''Error in model {}:
                                       {} != {}'''.format(
                                           model, t_ref, t_ref_sim))
            else:
                self.assertAlmostEqual(t_ref, t_ref_sim,
                                       msg='''Error in model {}:
                                       {} != {}'''.format(
                                           model, t_ref, t_ref_sim))


# --------------------------------------------------------------------------- #
#  Run the comparisons
# --------------------------------------------------------------------------- #

def suite():
    return unittest.makeSuite(TestRefractoryCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
