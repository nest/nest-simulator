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

import numpy as np
import matplotlib.pyplot as plt
import nest

##############################################################################
# First we specify the neuron parameters. To enable voltage dependent
# prefactor ``A_LTD(u_bar_bar)`` add ``A_LTD_const: False`` to the dictionary.

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
              't_ref': 0.0,
              }


##############################################################################
# Hardcoded spike times of presynaptic spike generator

spike_times_pre = [
    # Presynaptic spike before the postsynaptic
    [20.,  120.,  220.,  320.,  420.],
    [20.,   70.,  120.,  170.,  220.],
    [20.,   53.3,   86.7,  120.,  153.3],
    [20.,   45.,   70.,   95.,  120.],
    [20.,   40.,   60.,   80.,  100.],
    # Presynaptic spike after the postsynaptic
    [120.,  220.,  320.,  420.,  520.,  620.],
    [70.,  120.,  170.,  220.,  270.,  320.],
    [53.3,   86.6,  120.,  153.3,  186.6,  220.],
    [45.,   70.,   95.,  120.,  145.,  170.],
    [40.,   60.,   80.,  100.,  120.,  140.]]

##############################################################################
# Hardcoded spike times of postsynaptic spike generator

spike_times_post = [
    [10.,  110.,  210.,  310.,  410.],
    [10.,   60.,  110.,  160.,  210.],
    [10.,   43.3,   76.7,  110.,  143.3],
    [10.,   35.,   60.,   85.,  110.],
    [10.,  30.,  50.,  70.,  90.],
    [130.,  230.,  330.,  430.,  530.,  630.],
    [80.,  130.,  180.,  230.,  280.,  330.],
    [63.3,   96.6,  130.,  163.3,  196.6,  230.],
    [55.,   80.,  105.,  130.,  155.,  180.],
    [50.,   70.,   90.,  110.,  130.,  150.]]
init_w = 0.5
syn_weights = []
resolution = 0.1

##############################################################################
# Loop over pairs of spike trains

for s_t_pre, s_t_post in zip(spike_times_pre, spike_times_post):
    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": resolution})

    # Create one neuron
    nrn = nest.Create("aeif_psc_delta_clopath", 1, nrn_params)

    # We need a parrot neuron since spike generators can only
    # be connected with static connections
    prrt_nrn = nest.Create("parrot_neuron", 1)

    # Create and connect spike generators
    spike_gen_pre = nest.Create("spike_generator", {"spike_times": s_t_pre})

    nest.Connect(spike_gen_pre, prrt_nrn,
                 syn_spec={"delay": resolution})

    spike_gen_post = nest.Create("spike_generator", {"spike_times": s_t_post})

    nest.Connect(spike_gen_post, nrn, syn_spec={"delay": resolution, "weight": 80.0})

    # Create weight recorder
    wr = nest.Create('weight_recorder')

    # Create Clopath connection with weight recorder
    nest.CopyModel("clopath_synapse", "clopath_synapse_rec",
                   {"weight_recorder": wr})
    syn_dict = {"synapse_model": "clopath_synapse_rec",
                "weight": init_w, "delay": resolution}
    nest.Connect(prrt_nrn, nrn, syn_spec=syn_dict)

    # Simulation
    simulation_time = (10.0 + max(s_t_pre[-1], s_t_post[-1]))
    nest.Simulate(simulation_time)

    # Extract and save synaptic weights
    weights = wr.get("events", "weights")
    syn_weights.append(weights[-1])

syn_weights = np.array(syn_weights)
# scaling of the weights so that they are comparable to [1]
syn_weights = 100.0*15.0*(syn_weights - init_w)/init_w + 100.0

# Plot results
fig, ax = plt.subplots(1, sharex=False)
ax.plot([10., 20., 30., 40., 50.], syn_weights[5:], color='b', lw=2.5, ls='-',
        label="pre-post pairing")
ax.plot([10., 20., 30., 40., 50.], syn_weights[:5], color='g', lw=2.5, ls='-',
        label="post-pre pairing")
ax.set_ylabel("normalized weight change")
ax.set_xlabel("rho (Hz)")
ax.legend()
ax.set_title("synaptic weight")

plt.show()
