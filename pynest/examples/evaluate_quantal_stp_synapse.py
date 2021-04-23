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
import matplotlib.pyplot as plt

nest.ResetKernel()

################################################################################
# On average, the ``quantal_stp_synapse`` converges to the ``tsodyks2_synapse``,
# so we can compare the two by running multiple trials.
#
# First we define the number of trials as well as the number of release sites.

n_syn = 10.0  # number of synapses in a connection
n_trials = 100  # number of measurement trials

# The pre-synaptic neuron is driven by an injected current for part of each
# simulation cycle. We define here the parameters for this stimulation cycle.

I_stim = 376.0   # [pA] stimulation current
T_on = 500.0     # [ms] stimulation is on
T_off = 1000.0   # [ms] stimulation is off

T_cycle = T_on + T_off   # total duration of each stimulation cycle

###############################################################################
# Next, we define parameter sets for facilitation and initial weight

fac_params = {"U": 0.02, 
              "u": 0.02, 
              "tau_fac": 500.,
              "tau_rec": 200., 
              "weight": 1.}

###############################################################################
# Then, we assign the parameter set to the synapse models

tsyn_params = fac_params  # for tsodyks2_synapse
qsyn_params = tsyn_params.copy()  # for quantal_stp_synapse

tsyn_params["x"] = tsyn_params["U"]
qsyn_params["n"] = n_syn

###############################################################################
# To make the responses comparable, we have to scale the weight by the
# number of synapses.

qsyn_params["weight"] = 1. / n_syn

###############################################################################
# We create three different neurons.
# Neuron one is the sender, the two other neurons receive the synapses.
# We exploit Python's unpacking mechanism to assign the neurons to named
# variables directly.

pre_neuron, tsyn_neuron, qsyn_neuron = nest.Create("iaf_psc_exp",
                                                   params={"tau_syn_ex": 3.},
                                                   num=3)

###############################################################################
# We create two voltmeters, one for each of the postsynaptic neurons
# We start recording only after a first cycle used for equilibration

tsyn_voltmeter, qsyn_voltmeter = nest.Create("voltmeter",
                                             params={"start": T_cycle},
                                             num=2)

###############################################################################
# Connect one neuron with the deterministic tsodyks2 synapse and the other
# with the stochastic quantal stp synapse.; connect the voltmeter.

nest.Connect(pre_neuron, tsyn_neuron, 
             syn_spec={"model": "tsodyks2_synapse",
                       "params": tsyn_params})
nest.Connect(pre_neuron, qsyn_neuron, 
             syn_spec={"model": "quantal_stp_synapse",
                       "params": qsyn_params})

nest.Connect(tsyn_voltmeter, tsyn_neuron)
nest.Connect(qsyn_voltmeter, qsyn_neuron)

###############################################################################
# This loop runs over the `n_trials` trials and performs a standard protocol
# of a high-rate response, followed by a pause and then a recovery response.
#
# We actually run over n_trials + 1 rounds, since the first trial is for
# equilibration and is not recorded (see voltmeter parameters above).
#
# We use the NEST `:class:.RunManager` to improve performance and call `:func:.Run`
# inside for each part of the simulation.

with nest.RunManager():
    for t in range(n_trials + 1):
        pre_neuron.I_e = I_stim
        nest.Run(T_on)

        pre_neuron.I_e = 0.0
        nest.Run(T_off)

###############################################################################
# Simulate one additional time step to ensure voltage data is recorded for 
# final time step
nest.Simulate(nest.GetKernelStatus('resolution'))

###############################################################################
# Extract voltage traces and reshape to one column per trial
vm_tsyn = numpy.array(tsyn_voltmeter.get("events", "V_m"))
vm_qsyn = numpy.array(qsyn_voltmeter.get("events", "V_m"))

steps_per_trial = round(T_cycle / vm_tsyn.get('interval'))
vm_tsyn.shape = (n_trials, steps_per_trial)
vm_qsyn.shape = (n_trials, steps_per_trial)

###############################################################################
# Now compute the mean of all trials and plot against trials and references.

vm_mean = numpy.array([numpy.mean(vm[:, i]) for (i, j) in enumerate(vm[0, :])])
vm_ref_mean = numpy.array([numpy.mean(vm_reference[:, i])
                           for (i, j) in enumerate(vm_reference[0, :])])
plt.plot(vm_mean)
plt.plot(vm_ref_mean)
plt.show()

###############################################################################
# Finally, print the mean-suqared error between the trial-average and the
# reference trace. The value should be `< 10^-9`.

print(numpy.mean((vm_ref_mean - vm_mean) ** 2))
