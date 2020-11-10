# -*- coding: utf-8 -*-
#
# lin_rate_ipn_network.py
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

"""Network of linear rate neurons
-----------------------------------

This script simulates an excitatory and an inhibitory population
of ``lin_rate_ipn`` neurons with delayed excitatory and instantaneous
inhibitory connections. The rate of all neurons is recorded using
a multimeter. The resulting rate for one excitatory and one
inhibitory neuron is plotted.

"""

import nest
import numpy
import matplotlib.pyplot as plt

###############################################################################
# Assigning the simulation parameters to variables.

dt = 0.1  # the resolution in ms
T = 100.0  # Simulation time in ms

###############################################################################
# Definition of the number of neurons

order = 50
NE = int(4 * order)  # number of excitatory neurons
NI = int(1 * order)  # number of inhibitory neurons
N = int(NE+NI)       # total number of neurons

###############################################################################
# Definition of the connections


d_e = 5.   # delay of excitatory connections in ms
g = 5.0  # ratio inhibitory weight/excitatory weight
epsilon = 0.1  # connection probability
w = 0.1/numpy.sqrt(N)  # excitatory connection strength

KE = int(epsilon * NE)  # number of excitatory synapses per neuron (outdegree)
KI = int(epsilon * NI)  # number of inhibitory synapses per neuron (outdegree)
K_tot = int(KI + KE)  # total number of synapses per neuron
connection_rule = 'fixed_outdegree'  # connection rule

###############################################################################
# Definition of the neuron model and its neuron parameters

neuron_model = 'lin_rate_ipn'  # neuron model
neuron_params = {'linear_summation': True,
                 # type of non-linearity (not affecting linear rate models)
                 'tau': 10.0,
                 # time constant of neuronal dynamics in ms
                 'mu': 2.0,
                 # mean input
                 'sigma': 5.
                 # noise parameter
                 }


###############################################################################
# Configuration of the simulation kernel by the previously defined time
# resolution used in the simulation. Setting ``print_time`` to True prints
# the already processed simulation time as well as its percentage of the
# total simulation time.

nest.ResetKernel()
nest.SetKernelStatus({"resolution": dt, "use_wfr": False,
                      "print_time": True,
                      "overwrite_files": True})

print("Building network")

###############################################################################
# Configuration of the neuron model using ``SetDefaults``.

nest.SetDefaults(neuron_model, neuron_params)

###############################################################################
# Creation of the nodes using ``Create``.

n_e = nest.Create(neuron_model, NE)
n_i = nest.Create(neuron_model, NI)


################################################################################
# To record from the rate neurons a ``multimeter`` is created and the parameter
# ``record_from`` is set to `rate` as well as the recording interval to `dt`

mm = nest.Create('multimeter', params={'record_from': ['rate'],
                                       'interval': dt})

###############################################################################
# Specify synapse and connection dictionaries:
# Connections originating from excitatory neurons are associated
# with a delay `d` (``rate_connection_delayed``).
# Connections originating from inhibitory neurons are not associated
# with a delay (``rate_connection_instantaneous``).

syn_e = {'weight': w, 'delay': d_e, 'synapse_model': 'rate_connection_delayed'}
syn_i = {'weight': -g*w, 'synapse_model': 'rate_connection_instantaneous'}
conn_e = {'rule': connection_rule, 'outdegree': KE}
conn_i = {'rule': connection_rule, 'outdegree': KI}

###############################################################################
# Connect rate units

nest.Connect(n_e, n_e, conn_e, syn_e)
nest.Connect(n_i, n_i, conn_i, syn_i)
nest.Connect(n_e, n_i, conn_i, syn_e)
nest.Connect(n_i, n_e, conn_e, syn_i)

###############################################################################
# Connect recording device to rate units

nest.Connect(mm, n_e+n_i)

###############################################################################
# Simulate the network

nest.Simulate(T)

###############################################################################
# Plot rates of one excitatory and one inhibitory neuron

data = mm.events
rate_ex = data['rate'][numpy.where(data['senders'] == n_e[0].global_id)]
rate_in = data['rate'][numpy.where(data['senders'] == n_i[0].global_id)]
times = data['times'][numpy.where(data['senders'] == n_e[0].global_id)]

plt.figure()
plt.plot(times, rate_ex, label='excitatory')
plt.plot(times, rate_in, label='inhibitory')
plt.xlabel('time (ms)')
plt.ylabel('rate (a.u.)')
plt.show()
