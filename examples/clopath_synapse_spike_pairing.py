# -*- coding: utf-8 -*-
#
# clopath_synapse_spike_pairing.py
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
Clopath Rule: Spike pairing experiment
--------------------------------------

This script simulates one ``aeif_psc_delta_clopath`` neuron that is connected with
a Clopath connection [1]_. The synapse receives pairs of a pre- and a postsynaptic
spikes that are separated by either 10 ms (pre before post) or -10 ms (post
before pre). The change of the synaptic weight is measured after five of such
pairs. This experiment is repeated five times with different rates of the
sequence of the spike pairs: 10Hz, 20Hz, 30Hz, 40Hz, and 50Hz.

References
~~~~~~~~~~

.. [1] Clopath C, Büsing L, Vasilaki E, Gerstner W (2010). Connectivity reflects coding:
       a model of voltage-based STDP with homeostasis.
       Nature Neuroscience 13:3, 344--352
"""

import matplotlib.pyplot as plt
import nest
import numpy as np

##############################################################################
# First we specify the neuron parameters. To enable voltage dependent
# prefactor ``A_LTD(u_bar_bar)`` add ``A_LTD_const: False`` to the dictionary.

nrn_params = {
    "V_m": -70.6,
    "E_L": -70.6,
    "C_m": 281.0,
    "theta_minus": -70.6,
    "theta_plus": -45.3,
    "A_LTD": 14.0e-5,
    "A_LTP": 8.0e-5,
    "tau_u_bar_minus": 10.0,
    "tau_u_bar_plus": 7.0,
    "delay_u_bars": 4.0,
    "a": 4.0,
    "b": 0.0805,
    "V_reset": -70.6 + 21.0,
    "V_clamp": 33.0,
    "t_clamp": 2.0,
    "t_ref": 0.0,
}


##############################################################################
# Hardcoded spike times of presynaptic spike generator

spike_times_pre = [
    # Presynaptic spike before the postsynaptic
    [20.0, 120.0, 220.0, 320.0, 420.0],  # noqa
    [20.0, 70.0, 120.0, 170.0, 220.0],  # noqa
    [20.0, 53.3, 86.7, 120.0, 153.3],  # noqa
    [20.0, 45.0, 70.0, 95.0, 120.0],  # noqa
    [20.0, 40.0, 60.0, 80.0, 100.0],  # noqa
    # Presynaptic spike after the postsynaptic
    [120.0, 220.0, 320.0, 420.0, 520.0, 620.0],  # noqa
    [70.0, 120.0, 170.0, 220.0, 270.0, 320.0],  # noqa
    [53.3, 86.6, 120.0, 153.3, 186.6, 220.0],  # noqa
    [45.0, 70.0, 95.0, 120.0, 145.0, 170.0],  # noqa
    [40.0, 60.0, 80.0, 100.0, 120.0, 140.0],
]  # noqa

##############################################################################
# Hardcoded spike times of postsynaptic spike generator

spike_times_post = [
    [10.0, 110.0, 210.0, 310.0, 410.0],  # noqa
    [10.0, 60.0, 110.0, 160.0, 210.0],  # noqa
    [10.0, 43.3, 76.7, 110.0, 143.3],  # noqa
    [10.0, 35.0, 60.0, 85.0, 110.0],  # noqa
    [10.0, 30.0, 50.0, 70.0, 90.0],  # noqa
    [130.0, 230.0, 330.0, 430.0, 530.0, 630.0],  # noqa
    [80.0, 130.0, 180.0, 230.0, 280.0, 330.0],  # noqa
    [63.3, 96.6, 130.0, 163.3, 196.6, 230.0],  # noqa
    [55.0, 80.0, 105.0, 130.0, 155.0, 180.0],  # noqa
    [50.0, 70.0, 90.0, 110.0, 130.0, 150.0],
]  # noqa
init_w = 0.5
syn_weights = []
resolution = 0.1

##############################################################################
# Loop over pairs of spike trains

for s_t_pre, s_t_post in zip(spike_times_pre, spike_times_post):
    nest.ResetKernel()
    nest.resolution = resolution

    # Create one neuron
    nrn = nest.Create("aeif_psc_delta_clopath", params=nrn_params)

    # We need a parrot neuron since spike generators can only
    # be connected with static connections
    prrt_nrn = nest.Create("parrot_neuron")

    # Create and connect spike generators
    spike_gen_pre = nest.Create("spike_generator", params={"spike_times": s_t_pre})

    nest.Connect(spike_gen_pre, prrt_nrn, syn_spec={"delay": resolution})

    spike_gen_post = nest.Create("spike_generator", params={"spike_times": s_t_post})

    nest.Connect(spike_gen_post, nrn, syn_spec={"delay": resolution, "weight": 80.0})

    # Create weight recorder
    wr = nest.Create("weight_recorder")

    # Create Clopath connection with weight recorder
    nest.CopyModel("clopath_synapse", "clopath_synapse_rec", {"weight_recorder": wr})
    syn_dict = {"synapse_model": "clopath_synapse_rec", "weight": init_w, "delay": resolution}
    nest.Connect(prrt_nrn, nrn, syn_spec=syn_dict)

    # Simulation
    simulation_time = 10.0 + max(s_t_pre[-1], s_t_post[-1])
    nest.Simulate(simulation_time)

    # Extract and save synaptic weights
    weights = wr.get("events", "weights")
    syn_weights.append(weights[-1])

syn_weights = np.array(syn_weights)
# scaling of the weights so that they are comparable to [1]
syn_weights = 100.0 * 15.0 * (syn_weights - init_w) / init_w + 100.0

# Plot results
fig, ax = plt.subplots(1, sharex=False)
ax.plot([10.0, 20.0, 30.0, 40.0, 50.0], syn_weights[5:], color="b", lw=2.5, ls="-", label="pre-post pairing")
ax.plot([10.0, 20.0, 30.0, 40.0, 50.0], syn_weights[:5], color="g", lw=2.5, ls="-", label="post-pre pairing")
ax.set_ylabel("normalized weight change")
ax.set_xlabel("rho (Hz)")
ax.legend()
ax.set_title("synaptic weight")

plt.show()
