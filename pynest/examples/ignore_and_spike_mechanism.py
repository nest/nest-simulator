# -*- coding: utf-8 -*-
#
# ignore_and_spike_mechanism.py
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
Ignore-and-spike mechanism example
-----------------------------------

This script demonstrates the ignore-and-spike mechanism, which causes
a neuron to spike at defined intervals with a specified initial offset,
ignoring spikes from its internal dynamics.

Two neurons are simulated: one with the ignore-and-spike mechanism enabled
and one without. Both receive the same external current, and their spike
times are recorded and compared.
"""

###############################################################################
# First, we import all necessary modules for simulation, analysis and plotting.
# Additionally, we set the verbosity to suppress info messages and reset
# the kernel.

import matplotlib.pyplot as plt
import nest

nest.verbosity = nest.VerbosityLevel.WARNING
nest.ResetKernel()

###############################################################################
# Second, we set the simulation resolution.

nest.set(resolution=0.1)

###############################################################################
# Third, we create two neurons and a spike recorder.
# The first neuron uses the ignore-and-spike mechanism to fire at regular
# intervals (every 10 ms, starting at 5 ms). The second neuron behaves
# normally, responding to its internal dynamics.

nrn_regular = nest.Create("iaf_psc_delta")
nrn_ignore_spike = nest.Create(
    "iaf_psc_delta",
    params=dict(
        ignore_and_spike=True,
        ignore_and_spike_offset=5.0,
        ignore_and_spike_interval=10.0,
    ),
)

spike_recorder = nest.Create("spike_recorder")

###############################################################################
# Fourth, we set the same external current for both neurons.
# Without the ignore-and-spike mechanism, this would cause irregular spiking
# based on the neuron's membrane dynamics.

nrn_regular.I_e = 400.0
nrn_ignore_spike.I_e = 400.0

###############################################################################
# Fifth, we connect both neurons to the spike recorder.

nest.Connect(nrn_regular, spike_recorder)
nest.Connect(nrn_ignore_spike, spike_recorder)

###############################################################################
# Now we simulate the network for 100 ms.

nest.Simulate(100.0)

###############################################################################
# Finally, we retrieve the spike times and plot them as a raster plot.
# The neuron with ignore-and-spike enabled should show perfectly regular
# spikes at 5, 15, 25, 35, ... ms, regardless of the external input.

events = spike_recorder.get("events")
senders = events["senders"]
times = events["times"]

fig, ax = plt.subplots(figsize=(6, 2))
fig.subplots_adjust(left=0.3, right=0.95)

ax.scatter(times, senders, marker="|", color="#3455B4", s=300)
ax.set_xlabel("Time (ms)")
ax.set_yticks([nrn_regular.global_id, nrn_ignore_spike.global_id])
ax.set_yticklabels(["Regular neuron", "Ignore-and-spike\nneuron"], ha="center")
ax.tick_params(axis="y", pad=50)
ax.set_xticks(range(0, 105, 5))
ax.set_xlim(0, 100)
ax.set_ylim(0.5, 2.5)
ax.spines[["top", "right"]].set_visible(False)
ax.grid(True, alpha=0.3)

fig.tight_layout()

plt.show()
