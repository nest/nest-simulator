# -*- coding: utf-8 -*-
#
# one_neuron.py
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
stdp dopamine synapse example
-----------------------------

script to test stdp_dopamine_synapse model implementing dopamine-dependent spike-timing dependent plasticity 
as defined in [1], based on [2]. Two neurons, which fire poisson like, are connected by a
stdp_dopamine_synapse. Dopamine is release by a volume transmitter, which also fires poisson like.

References
~~~~~~~~~~

.. [1] Potjans W, Morrison A, Diesmann M (2010). Enabling functional neural
       circuit simulations with distributed computing of neuromodulated
       plasticity. Frontiers in Computational Neuroscience, 4:141.
       DOI: https://doi.org/10.3389/fncom.2010.00141
.. [2] Izhikevich EM (2007). Solving the distal reward problem through linkage
       of STDP and dopamine signaling. Cerebral Cortex, 17(10):2443-2452.
       DOI: https://doi.org/10.1093/cercor/bhl152

See Also
~~~~~~~~

:doc:`structural_plasticity`

"""

# import necessary modules.
import nest
import numpy as np
import matplotlib.pyplot as plt

# reset kernel and set random seed
nest.ResetKernel()
nest.SetKernelStatus({'rng_seed': 1})

# parameters
pg_rate = 40.
J = 35.
delay = 1.
J_EX = 4000.
J_DX = 4000.

# create a pre and a postsynaptic neurons
neuron1 = nest.Create("iaf_psc_exp")
neuron2 = nest.Create("iaf_psc_exp")

# create poisson generators
pg1 = nest.Create("poisson_generator", params={"rate": pg_rate})
pg2 = nest.Create("poisson_generator", params={"rate": pg_rate})
pg_dopa = nest.Create("poisson_generator", params={"rate": pg_rate})
parrot_neuron = nest.Create("parrot_neuron")

# create a volume transmitter and weight recorder
vt = nest.Create("volume_transmitter")
wr = nest.Create("weight_recorder")

# connect the neurons using "stdp_dopamine_synapse" 
nest.CopyModel("stdp_dopamine_synapse", "stdp_dopa_wr", {"weight_recorder": wr,
                                                         "vt": vt.tolist()[0],
                                                         "weight": J,
                                                         "delay": delay})

nest.Connect(neuron1, neuron2, syn_spec={"synapse_model": "stdp_dopa_wr"})

# connect poisson generators
syn_spec = {"synapse_model": "static_synapse", "weight": J_EX, "delay": delay}
nest.Connect(pg1, neuron1, syn_spec=syn_spec)
nest.Connect(pg2, neuron2, syn_spec=syn_spec)

syn_spec = {"synapse_model": "static_synapse", "weight": J_DX, "delay": delay}
nest.Connect(pg_dopa, parrot_neuron)
nest.Connect(parrot_neuron, vt, syn_spec=syn_spec)

# create and connect spike recorder
sr1 = nest.Create("spike_recorder")
sr2 = nest.Create("spike_recorder")
sr_dopa = nest.Create("spike_recorder")
nest.Connect(neuron1, sr1)
nest.Connect(neuron2, sr2)
nest.Connect(parrot_neuron, sr_dopa)

# simulate network
T = 100.
nest.Simulate(T)

# retrieve spike time and weights
spike_times1 = nest.GetStatus(sr1, keys='events')[0]['times']
spike_times2 = nest.GetStatus(sr2, keys='events')[0]['times']

spike_times_dopa = nest.GetStatus(sr_dopa, keys='events')[0]['times']

t_log = nest.GetStatus(wr, "events")[0]["times"]
w_log = nest.GetStatus(wr, "events")[0]["weights"]

# plot settings 
fig_size = (5.2, 5.7)
plt.rcParams["font.size"] = 8
plt.rcParams["legend.fontsize"] = 6
plt.rcParams["figure.figsize"] = fig_size
plt.rcParams["font.family"] = "sans-serif"
plt.rcParams["savefig.dpi"] = 300
plt.rcParams["text.usetex"] = True
mew = 2
ms = 10
ms_w = 2
alpha_spikes = 0.4
alpha_grid = 0.4

# plot data
fig, ax = plt.subplots(nrows=4)

ax[0].plot(spike_times1, np.ones(len(spike_times1)), '|', ms=ms, mew=mew, color="blue", alpha=alpha_spikes)
ax[0].set_ylabel("Pre spikes")

ax[1].plot(spike_times2, np.ones(len(spike_times2)), '|', ms=ms, mew=mew, color="blue", alpha=alpha_spikes)
ax[1].set_ylabel("Post spikes")

ax[2].plot(spike_times_dopa, np.ones(len(spike_times_dopa)), '|', ms=ms, mew=mew, color='red')
ax[2].set_ylabel("dopa spikes")

ax[3].plot(t_log, w_log, marker="o", ms=ms_w)
ax[3].set_ylabel("weight (pA)")
ax[3].set_xlabel("Time [ms]")

for i, _ax in enumerate(ax):
    _ax.grid(which="major", axis="both")
    _ax.grid(which="minor", axis="x", linestyle=":", alpha=alpha_grid)
    _ax.minorticks_on()
    _ax.set_xlim(0., T)
    if i != len(ax) - 1:
        _ax.set_xticklabels([])

# save plot
fname = "example_stdp_dopa_synapse"
print("save ./%s.pdf ..." % fname)
print("save ./%s.png ..." % fname)
plt.savefig("./%s.pdf" % fname)
plt.savefig("./%s.png" % fname)
