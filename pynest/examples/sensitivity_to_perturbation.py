# -*- coding: utf-8 -*-
#
# sensitivity_to_perturbation.py
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
Sensitivity to perturbation
---------------------------

This script simulates a network in two successive trials, which are identical
except for one extra input spike in the second realisation (a small
perturbation). The network consists of recurrent, randomly connected excitatory
and inhibitory neurons. Its activity is driven by an external Poisson input
provided to all neurons independently. In order to ensure that the network is
reset appropriately between the trials, we do the following steps:

- resetting the network
- resetting the random network generator
- resetting the internal clock
- deleting all entries in the spike recorder
- introducing a hyperpolarisation phase between the trials
  (in order to avoid that spikes remaining in the NEST memory
  after the first simulation are fed into the second simulation)

"""


###############################################################################
# Importing all necessary modules for simulation, analysis and plotting.


import numpy
import matplotlib.pyplot as plt
import nest


###############################################################################
# Here we define all parameters necessary for building and simulating the
# network.

# We start with the global network parameters.


NE = 1000      # number of excitatory neurons
NI = 250       # number of inhibitory neurons
N = NE + NI    # total number of neurons
KE = 100       # excitatory in-degree
KI = 25        # inhibitory in-degree


###############################################################################
# Parameters specific for the neurons in the network. The  default values of
# the reset potential ``E_L`` and the spiking threshold ``V_th`` are used to set
# the limits of the initial potential of the neurons.


neuron_model = 'iaf_psc_delta'
neuron_params = nest.GetDefaults(neuron_model)
Vmin = neuron_params['E_L']   # minimum of initial potential distribution (mV)
Vmax = neuron_params['V_th']  # maximum of initial potential distribution (mV)


###############################################################################
# Synapse parameters. Changing the weights `J` in the network can lead to
# qualitatively different behaviors. If `J` is small (e.g. ``J = 0.1``), we
# are likely to observe a non-chaotic network behavior (after perturbation
# the network returns to its original activity). Increasing `J`
# (e.g ``J = 5.5``) leads to rather chaotic activity. Given that in this
# example the transition to chaos is probabilistic, we sometimes observe
# chaotic behavior for small weights (e.g. ``J = 0.5``) and non-chaotic
# behavior for strong weights (e.g. ``J = 5.4``).


J = 0.5                   # excitatory synaptic weight (mV)
g = 6.                    # relative inhibitory weight
delay = 0.1               # spike transmission delay (ms)


# External input parameters.


Jext = 0.2                # PSP amplitude for external Poisson input (mV)
rate_ext = 6500.          # rate of the external Poisson input


# Perturbation parameters.


t_stim = 400.             # perturbation time (time of the extra spike)
Jstim = Jext              # perturbation amplitude (mV)


# Simulation parameters.


T = 1000.                 # simulation time per trial (ms)
fade_out = 2. * delay     # fade out time (ms)
dt = 0.01                 # simulation time resolution (ms)
seed_NEST = 30            # seed of random number generator in Nest
seed_numpy = 30           # seed of random number generator in numpy

senders = []
spiketimes = []

###############################################################################

# we run the two simulations successively. After each simulation the
# sender ids and spiketimes are stored in a list (``senders``, ``spiketimes``).

for trial in [0, 1]:

    # Before we build the network, we reset the simulation kernel to ensure
    # that previous NEST simulations in the Python shell will not disturb this
    # simulation and set the simulation resolution (later defined
    # synaptic delays cannot be smaller than the simulation resolution).
    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": dt})

    ###############################################################################
    # Now we start building the network and create excitatory and inhibitory nodes
    # and connect them. According to the connectivity specification, each neuron
    # is assigned random KE synapses from the excitatory population and random KI
    # synapses from the inhibitory population.

    nodes_ex = nest.Create(neuron_model, NE)
    nodes_in = nest.Create(neuron_model, NI)
    allnodes = nodes_ex + nodes_in

    nest.Connect(nodes_ex, allnodes,
                 conn_spec={'rule': 'fixed_indegree', 'indegree': KE},
                 syn_spec={'weight': J, 'delay': dt})
    nest.Connect(nodes_in, allnodes,
                 conn_spec={'rule': 'fixed_indegree', 'indegree': KI},
                 syn_spec={'weight': -g * J, 'delay': dt})

    ###############################################################################
    # Afterwards we create a ``poisson_generator`` that provides spikes (the external
    # input) to the neurons until time ``T`` is reached.
    # Afterwards a ``dc_generator``, which is also connected to the whole population,
    # provides a stong hyperpolarisation step for a short time period ``fade_out``.
    #
    # The ``fade_out`` period has to last at least twice as long as the simulation
    # resolution to supress the neurons from firing.

    ext = nest.Create("poisson_generator",
                      params={'rate': rate_ext, 'stop': T})
    nest.Connect(ext, allnodes,
                 syn_spec={'weight': Jext, 'delay': dt})

    suppr = nest.Create("dc_generator",
                        params={'amplitude': -1e16, 'start': T,
                                'stop': T + fade_out})
    nest.Connect(suppr, allnodes)

    spikerecorder = nest.Create("spike_recorder")
    nest.Connect(allnodes, spikerecorder)

    ###############################################################################
    # We then create the ``spike_generator``, which provides the extra spike
    # (perturbation).

    stimulus = nest.Create("spike_generator")
    stimulus.spike_times = []

    ###############################################################################
    # We need to reset the random number generator and the clock of
    # the simulation Kernel. In addition, we ensure that there is no spike left in
    # the spike recorder.

    nest.SetKernelStatus({"rng_seeds": [seed_NEST], 'time': 0.0})
    spikerecorder.n_events = 0

    # We assign random initial membrane potentials to all neurons

    numpy.random.seed(seed_numpy)
    Vms = Vmin + (Vmax - Vmin) * numpy.random.rand(N)
    allnodes.V_m = Vms

    ##############################################################################
    # In the second trial, we add an extra input spike at time ``t_stim`` to the
    # neuron that fires first after perturbation time ``t_stim``. Thus, we make sure
    # that the perturbation is transmitted to the network before it fades away in
    # the perturbed neuron. (Single IAF-neurons are not chaotic.)

    if trial == 1:
        id_stim = [senders[0][spiketimes[0] > t_stim][0]]
        nest.Connect(stimulus, nest.NodeCollection(id_stim),
                     syn_spec={'weight': Jstim, 'delay': dt})
        stimulus.spike_times = [t_stim]

    # Now we simulate the network and add a fade out period to discard
    # remaining spikes.

    nest.Simulate(T)
    nest.Simulate(fade_out)

    # Storing the data.

    senders += [spikerecorder.get('events', 'senders')]
    spiketimes += [spikerecorder.get('events', 'times')]

###############################################################################
# We plot the spiking activity of the network (first trial in red, second trial
# in black).

plt.figure(1)
plt.clf()
plt.plot(spiketimes[0], senders[0], 'ro', ms=4.)
plt.plot(spiketimes[1], senders[1], 'ko', ms=2.)
plt.xlabel('time (ms)')
plt.ylabel('neuron id')
plt.xlim((0, T))
plt.ylim((0, N))
plt.show()
