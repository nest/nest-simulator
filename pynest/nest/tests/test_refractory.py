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
Assert that all neuronal models implement refractory period correctly.

Details
-------
Submit the neuron to a constant excitatory current so that it spikes in the
[0, 50] ms.
A ``spike_detector`` is used to detect the time at which the neuron spikes and
a ``voltmeter`` is then used to make sure the voltage is clamped to ``V_reset``
during exactly ``t_ref``.
"""


# --------------------------------------------------------------------------- #
#  Models, specific parameters
# -------------------------
#

ignore_model = [
    "aeif_cond_alpha_RK5",
    "aeif_cond_alpha_multisynapse",
    "aeif_cond_beta_multisynapse",
    "amat2_psc_exp",
    "ginzburg_neuron",
    "hh_cond_exp_traub",
    "hh_psc_alpha",
    "hh_psc_alpha_gap",
    "ht_neuron",
    "iaf_chs_2007",
    "iaf_chxk_2008",
    "iaf_psc_alpha_canon",
    "iaf_psc_alpha_presc",
    "iaf_psc_delta_canon",
    "iaf_psc_exp_ps",
    "iaf_tum_2000",
    "izhikevich",
    "mat2_psc_exp",
    "mcculloch_pitts_neuron",
    "parrot_neuron",
    "parrot_neuron_ps",
    "pp_pop_psc_delta",
    "pp_psc_delta",
    "sli_neuron",
]

nodes = nest.Models("nodes")
# list of all neuronal models to be tested
neurons = [m for m in nodes if nest.GetDefaults(m, "element_type") == "neuron"
           and m not in ignore_model]

# additional parameters for the connector
add_connect_param = {
    "iaf_cond_alpha_mc": {"receptor_type": 7},
}


# --------------------------------------------------------------------------- #
#  Simulation and refractory time
# -------------------------
#

simtime = 100
resolution = 0.1
min_steps = 1
max_steps = 200


# --------------------------------------------------------------------------- #
#  Test class
# -------------------------
#

def foreach_neuron(func):
    '''
    Decorator that automatically does the test for all neurons.
    '''       
    def wrapper(*args, **kwargs):
        self = args[0]
        msd = 123456
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        pyrngs = [np.random.RandomState(s) for s in range(msd, msd + N_vp)]
        for name in neurons:
            nest.ResetKernel()
            nest.SetKernelStatus({
                'resolution': resolution, 'grng_seed': msd + N_vp,
                'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})
            func(self, name, **kwargs)
    return wrapper


class RefractoryTestCase(unittest.TestCase):
    """
    Check the correct implementation of refractory time in all neuronal models.
    """

    def compute_reftime(self, model, sd, vm, Vr):
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
        Vr : double
            Value of the reset potential.

        Returns
        -------
        t_ref_sim : double
            Value of the simulated refractory period.
        '''
        spike_times = nest.GetStatus(sd, "events")[0]["times"]
        times = nest.GetStatus(vm, "events")[0]["times"]
        idx_max = (np.argwhere(np.isclose(times, spike_times[1]))[0]
                   if len(spike_times) > 1 else -1)
        name_Vm = "V_m.s" if model == "iaf_cond_alpha_mc" else "V_m"
        Vs = nest.GetStatus(vm, "events")[0][name_Vm]
        # get the index at which the spike occured
        idx_spike = np.argwhere(times == spike_times[0])[0]
        idx_end = np.where(np.isclose(Vs[idx_spike:idx_max], Vr, 1e-6))[0][-1]
        t_ref_sim = idx_end * resolution
        return t_ref_sim


    @foreach_neuron
    def test_refractory_time(self, model):
        '''
        Check that refractory time implementation is correct.
        '''
        # randomly set a refractory period
        t_ref = resolution * np.random.randint(min_steps, max_steps)
        # create the neuron and devices
        nparams = {"t_ref": t_ref}
        neuron = nest.Create(model, params=nparams)
        name_Vm = "V_m.s" if model == "iaf_cond_alpha_mc" else "V_m"
        vm_params = {"interval": resolution, "record_from": [name_Vm]}
        vm = nest.Create("voltmeter", params=vm_params)
        sd = nest.Create("spike_detector")
        cg = nest.Create("dc_generator", params={"amplitude": 600.})
        # connect them and simulate
        nest.Connect(vm, neuron)
        nest.Connect(cg, neuron, syn_spec=add_connect_param.get(model, {}))
        nest.Connect(neuron, sd)
        
        nest.Simulate(simtime)

        # get and compare t_ref
        Vr = nest.GetStatus(neuron, "V_reset")[0]
        t_ref_sim = self.compute_reftime(model, sd, vm, Vr)
        
        self.assertEqual(t_ref, t_ref_sim, '''Error in model {}:
                         {} != {}'''.format(model, t_ref, t_ref_sim))


# --------------------------------------------------------------------------- #
#  Run the comparisons
# ------------------------
#

def suite():
    return unittest.makeSuite(RefractoryTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
