# -*- coding: utf-8 -*-
#
# evaluate_quantal_stp_synapse.py
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

"""Example for the quantal_stp_synapse
-----------------------------------------

The ``quantal_stp_synapse`` is a stochastic version of the Tsodys-Markram model
for synaptic short term plasticity (STP).
This script compares the two variants of the Tsodyks/Markram synapse in NEST.

This synapse model implements synaptic short-term depression and short-term
facilitation according to the quantal release model described by Fuhrmann et
al. [1]_ and Loebel et al. [2]_.

Each presynaptic spike will stochastically activate a fraction of the
available release sites.  This fraction is binomialy distributed and the
release probability per site is governed by the Fuhrmann et al. (2002) model.
The solution of the differential equations is taken from Maass and Markram
2002 [3]_.

The connection weight is interpreted as the maximal weight that can be
obtained if all n release sites are activated.

Parameters
~~~~~~~~~~~~~

The following parameters can be set in the status dictionary:

* U         - Maximal fraction of available resources [0,1], default=0.5
* u         - available fraction of resources [0,1], default=0.5
* p         - probability that a vesicle is available, default = 1.0
* n         - total number of release sites, default = 1
* a         - number of available release sites, default = n
* tau_rec   - time constant for depression in ms, default=800 ms
* tau_rec   - time constant for facilitation in ms, default=0 (off)


References
~~~~~~~~~~~~~

.. [1] Fuhrmann G, Segev I, Markram H, and Tsodyks MV. (2002). Coding of
       temporal information by activity-dependent synapses. Journal of
       Neurophysiology, 8. https://doi.org/10.1152/jn.00258.2001
.. [2] Loebel, A., Silberberg, G., Helbig, D., Markram, H., Tsodyks,
       M. V, & Richardson, M. J. E. (2009). Multiquantal release underlies
       the distribution of synaptic efficacies in the neocortex. Frontiers
       in Computational Neuroscience, 3:27. doi:10.3389/neuro.10.027.
.. [3] Maass W, and Markram H. (2002). Synapses as dynamic memory buffers.
       Neural Networks, 15(2), 155-161.
       http://dx.doi.org/10.1016/S0893-6080(01)00144-7

"""
import nest
import nest.voltage_trace
import numpy
import pylab

nest.ResetKernel()

################################################################################
# On average, the ``quantal_stp_synapse`` converges to the ``tsodyks2_synapse``,
# so we can compare the two by running multiple trials.
#
# First we define the number of trials as well as the number of release sites.

n_syn = 10.0  # number of synapses in a connection
n_trials = 100  # number of measurement trials

###############################################################################
# Next, we define parameter sets for facilitation

fac_params = {"U": 0.02, "u": 0.02, "tau_fac": 500.,
              "tau_rec": 200., "weight": 1.}

###############################################################################
# Then, we assign the parameter set to the synapse models

t1_params = fac_params  # for tsodyks2_synapse
t2_params = t1_params.copy()  # for quantal_stp_synapse

t1_params['x'] = t1_params['U']
t2_params['n'] = n_syn

###############################################################################
# To make the responses comparable, we have to scale the weight by the
# number of synapses.

t2_params['weight'] = 1. / n_syn

###############################################################################
# Next, we chage the defaults of the various models to our parameters.

nest.SetDefaults("tsodyks2_synapse", t1_params)
nest.SetDefaults("quantal_stp_synapse", t2_params)
nest.SetDefaults("iaf_psc_exp", {"tau_syn_ex": 3.})

###############################################################################
# We create three different neurons.
# Neuron one is the sender, the two other neurons receive the synapses.

neuron = nest.Create("iaf_psc_exp", 3)

###############################################################################
# The connection from neuron 1 to neuron 2 is a deterministic synapse.

nest.Connect(neuron[0], neuron[1], syn_spec="tsodyks2_synapse")

###############################################################################
# The connection from neuron 1 to neuron 3 has a stochastic
# ``quantal_stp_synapse``.

nest.Connect(neuron[0], neuron[2], syn_spec="quantal_stp_synapse")

###############################################################################
# The voltmeter will show us the synaptic responses in neurons 2 and 3.

voltmeter = nest.Create("voltmeter", 2)

###############################################################################
# One dry run to bring all synapses into their rest state.
# The default initialization does not achieve this. In large network
# simulations this problem does not show, but in small simulations like
# this, we would see it.

nest.SetStatus(neuron[0], "I_e", 376.0)

nest.Simulate(500.0)
nest.SetStatus(neuron[0], "I_e", 0.0)
nest.Simulate(1000.0)

###############################################################################
# Only now do we connect the ``voltmeter`` to the neurons.

nest.Connect(voltmeter[0], neuron[1])
nest.Connect(voltmeter[1], neuron[2])

###############################################################################
# This loop runs over the `n_trials` trials and performs a standard protocol
# of a high-rate response, followed by a pause and then a recovery response.

for t in range(n_trials):
    nest.SetStatus(neuron[0], "I_e", 376.0)
    nest.Simulate(500.0)
    nest.SetStatus(neuron[0], "I_e", 0.0)
    nest.Simulate(1000.0)

###############################################################################
# Flush the last voltmeter events from the queue by simulating one time-step.

nest.Simulate(.1)

###############################################################################
# Extract the reference trace.
vm = numpy.array(nest.GetStatus(voltmeter[1], 'events')[0]['V_m'])
vm_reference = numpy.array(nest.GetStatus(voltmeter[0], 'events')[0]['V_m'])

vm.shape = (n_trials, 1500)
vm_reference.shape = (n_trials, 1500)

###############################################################################
# Now compute the mean of all trials and plot against trials and references.

vm_mean = numpy.array([numpy.mean(vm[:, i]) for (i, j) in enumerate(vm[0, :])])
vm_ref_mean = numpy.array([numpy.mean(vm_reference[:, i])
                           for (i, j) in enumerate(vm_reference[0, :])])
pylab.plot(vm_mean)
pylab.plot(vm_ref_mean)

###############################################################################
# Finally, print the mean-suqared error between the trial-average and the
# reference trace. The value should be `< 10^-9`.

print(numpy.mean((vm_ref_mean - vm_mean) ** 2))
