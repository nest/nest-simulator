# -*- coding: utf-8 -*-
#
# gap_junctions_inhibitory_network.py
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
Gap Junctions: Inhibitory network example
-----------------------------------------

This script simulates an inhibitory network of 500 Hodgkin-Huxley neurons.
Without the gap junctions (meaning for ``gap_weight = 0.0``) the network shows
an asynchronous irregular state that is caused by the external excitatory
Poissonian drive being balanced by the inhibitory feedback within the
network. With increasing `gap_weight` the network synchronizes:

For a lower gap weight of 0.3 nS the network remains in an asynchronous
state. With a weight of 0.54 nS the network switches randomly between the
asynchronous to the synchronous state, while for a gap weight of 0.7 nS a
stable synchronous state is reached.

This example is also used as test case 2 (see Figure 9 and 10)
in [1]_.

References
~~~~~~~~~~

.. [1] Hahne et al. (2015) A unified framework for spiking and gap-junction
       interactions in distributed neuronal network simulations, Front.
       Neuroinform. http://dx.doi.org/10.3389/neuro.11.012.2008
"""

import matplotlib.pyplot as plt
import nest
import numpy

n_neuron = 500
gap_per_neuron = 60
inh_per_neuron = 50
delay = 1.0
j_exc = 300.0
j_inh = -50.0
threads = 8
stepsize = 0.05
simtime = 501.0
gap_weight = 0.3

nest.ResetKernel()

###############################################################################
# First we set the random seed, adjust the kernel settings and create
# ``hh_psc_alpha_gap`` neurons, ``spike_recorder`` and ``poisson_generator``.

numpy.random.seed(1)

nest.resolution = 0.05
nest.total_num_virtual_procs = threads
nest.print_time = True

# Settings for waveform relaxation. If 'use_wfr' is set to False,
# communication takes place in every step instead of using an
# iterative solution
nest.use_wfr = True
nest.wfr_comm_interval = 1.0
nest.wfr_tol = 0.0001
nest.wfr_max_iterations = 15
nest.wfr_interpolation_order = 3

neurons = nest.Create("hh_psc_alpha_gap", n_neuron)

sr = nest.Create("spike_recorder")
pg = nest.Create("poisson_generator", params={"rate": 500.0})

###############################################################################
# Each neuron shall receive ``inh_per_neuron = 50`` inhibitory synaptic inputs
# that are randomly selected from all other neurons, each with synaptic
# weight ``j_inh = -50.0`` pA and a synaptic delay of 1.0 ms. Furthermore each
# neuron shall receive an excitatory external Poissonian input of 500.0 Hz
# with synaptic weight ``j_exc = 300.0`` pA and the same delay.
# The desired connections are created with the following commands:

syn_spec = nest.synapsemodels.static(weight=j_inh, delay=delay)

nest.Connect(
    nest.FixedIndegree(
        neurons, neurons, indegree=inh_per_neuron, allow_autapses=False, allow_multapses=True, syn_spec=syn_spec
    )
)

nest.Connect(nest.AllToAll(pg, neurons, syn_spec=nest.synapsemodels.static(weight=j_exc, delay=delay)))

###############################################################################
# Then the neurons are connected to the ``spike_recorder`` and the initial
# membrane potential of each neuron is set randomly between -40 and -80 mV.

nest.Connect(nest.AllToAll(neurons, sr))

neurons.V_m = nest.random.uniform(min=-80.0, max=-40.0)

#######################################################################################
# Finally gap junctions are added to the network. :math:`(60*500)/2` ``gap_junction``
# connections are added randomly resulting in an average of 60 gap-junction
# connections per neuron. We must not use the ``fixed_indegree`` or the
# ``fixed_outdegree`` functionality of ``nest.Connect()`` to create the
# connections, as ``gap_junction`` connections are bidirectional connections
# and we need to make sure that the same neurons are connected in both ways.
# This is achieved by creating the connections on the Python level with the
# `random` module of the Python Standard Library and connecting the neurons
# using the ``make_symmetric`` flag for ``one_to_one`` connections.

n_connection = int(n_neuron * gap_per_neuron / 2)
neuron_list = neurons.tolist()
connections = numpy.random.choice(neuron_list, [n_connection, 2])

for source_node_id, target_node_id in connections:
    nest.Connect(
        nest.OneToOne(
            nest.NodeCollection([source_node_id]),
            nest.NodeCollection([target_node_id]),
            make_symmetric=True,
            syn_spec=nest.synapsemodels.gap_junction(weight=gap_weight),
        )
    )

###############################################################################
# In the end we start the simulation and plot the spike pattern.

nest.Simulate(simtime)

events = sr.events
times = events["times"]
spikes = events["senders"]
n_spikes = sr.n_events

hz_rate = (1000.0 * n_spikes / simtime) / n_neuron

plt.figure(1)
plt.plot(times, spikes, "o")
plt.title(f"Average spike rate (Hz): {hz_rate:.2f}")
plt.xlabel("time (ms)")
plt.ylabel("neuron no")
plt.show()
