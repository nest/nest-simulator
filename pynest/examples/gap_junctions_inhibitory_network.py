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

"""Gap Junctions: Inhibitory network example
-----------------------------------------------

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
~~~~~~~~~~~

.. [1] Hahne et al. (2015) A unified framework for spiking and gap-junction
       interactions in distributed neuronal network simulations, Front.
       Neuroinform. http://dx.doi.org/10.3389/neuro.11.012.2008
"""

import nest
import pylab as pl
import numpy
import random

n_neuron = 500
gap_per_neuron = 60
inh_per_neuron = 50
delay = 1.0
j_exc = 300.
j_inh = -50.
threads = 8
stepsize = 0.05
simtime = 501.
gap_weight = 0.3

nest.ResetKernel()

###############################################################################
# First we set the random seed, adjust the kernel settings and create
# ``hh_psc_alpha_gap`` neurons, ``spike_detector`` and ``poisson_generator``.

random.seed(1)

nest.SetKernelStatus({'resolution': 0.05,
                      'total_num_virtual_procs': threads,
                      'print_time': True,
                      # Settings for waveform relaxation
                      # 'use_wfr': False uses communication in every step
                      # instead of an iterative solution
                      'use_wfr': True,
                      'wfr_comm_interval': 1.0,
                      'wfr_tol': 0.0001,
                      'wfr_max_iterations': 15,
                      'wfr_interpolation_order': 3})

neurons = nest.Create('hh_psc_alpha_gap', n_neuron)

sd = nest.Create("spike_detector")
pg = nest.Create("poisson_generator", params={'rate': 500.0})

###############################################################################
# Each neuron shall receive ``inh_per_neuron = 50`` inhibitory synaptic inputs
# that are randomly selected from all other neurons, each with synaptic
# weight ``j_inh = -50.0`` pA and a synaptic delay of 1.0 ms. Furthermore each
# neuron shall receive an excitatory external Poissonian input of 500.0 Hz
# with synaptic weight ``j_exc = 300.0`` pA and the same delay.
# The desired connections are created with the following commands:

conn_dict = {'rule': 'fixed_indegree',
             'indegree': inh_per_neuron,
             'allow_autapses': False,
             'allow_multapses': True}

syn_dict = {'synapse_model': 'static_synapse',
            'weight': j_inh,
            'delay': delay}

nest.Connect(neurons, neurons, conn_dict, syn_dict)

nest.Connect(pg, neurons, 'all_to_all',
             syn_spec={'synapse_model': 'static_synapse',
                       'weight': j_exc,
                       'delay': delay})

###############################################################################
# Then the neurons are connected to the ``spike_detector`` and the initial
# membrane potential of each neuron is set randomly between -40 and -80 mV.

nest.Connect(neurons, sd)

for i in range(n_neuron):
    nest.SetStatus(neurons[i], {'V_m': (-40. - 40. * random.random())})

#######################################################################################
# Finally gap junctions are added to the network. :math:`(60*500)/2` ``gap_junction``
# connections are added randomly resulting in an average of 60 gap-junction
# connections per neuron. We must not use the ``fixed_indegree`` oder
# ``fixed_outdegree`` functionality of ``nest.Connect()`` to create the
# connections, as ``gap_junction`` connections are bidirectional connections
# and we need to make sure that the same neurons are connected in both ways.
# This is achieved by creating the connections on the Python level with the
# `random` module of the Python Standard Library and connecting the neurons
# using the ``make_symmetric`` flag for ``one_to_one`` connections.

n_connection = int(n_neuron * gap_per_neuron / 2)
neuron_list = [n for n in neurons]
connections = numpy.transpose(
    [random.sample(neuron_list, 2) for _ in range(n_connection)])

for indx in range(n_connection):
    nest.Connect(nest.GIDCollection([connections[0][indx]]),
                 nest.GIDCollection([connections[1][indx]]),
                 {'rule': 'one_to_one', 'make_symmetric': True},
                 {'synapse_model': 'gap_junction', 'weight': gap_weight})

###############################################################################
# In the end we start the simulation and plot the spike pattern.

nest.Simulate(simtime)

times = nest.GetStatus(sd, 'events')[0]['times']
spikes = nest.GetStatus(sd, 'events')[0]['senders']
n_spikes = nest.GetStatus(sd, 'n_events')[0]

hz_rate = (1000.0 * n_spikes / simtime) / n_neuron

pl.figure(1)
pl.plot(times, spikes, 'o')
pl.title('Average spike rate (Hz): %.2f' % hz_rate)
pl.xlabel('time (ms)')
pl.ylabel('neuron no')
pl.show()
