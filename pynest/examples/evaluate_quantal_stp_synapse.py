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

"""
Example for the quantal_stp_synapse
-----------------------------------

The ``quantal_stp_synapse`` is a stochastic version of the Tsodys-Markram model
for synaptic short term plasticity (STP).
This script compares the two variants of the Tsodyks/Markram synapse in NEST.

This synapse model implements synaptic short-term depression and short-term
facilitation according to the quantal release model described by Fuhrmann et
al. [1]_ and Loebel et al. [2]_.

Each presynaptic spike will stochastically activate a fraction of the
available release sites.  This fraction is binomially distributed and the
release probability per site is governed by the Fuhrmann et al. (2002) model.
The solution of the differential equations is taken from Maass and Markram
2002 [3]_.

The connection weight is interpreted as the maximal weight that can be
obtained if all n release sites are activated.

Parameters
~~~~~~~~~~

The following parameters can be set in the status dictionary:

* U         - Maximal fraction of available resources [0,1], default=0.5
* u         - available fraction of resources [0,1], default=0.5
* p         - probability that a vesicle is available, default = 1.0
* n         - total number of release sites, default = 1
* a         - number of available release sites, default = n
* tau_rec   - time constant for depression in ms, default=800 ms
* tau_rec   - time constant for facilitation in ms, default=0 (off)


References
~~~~~~~~~~

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
import numpy
import matplotlib.pyplot as plt

################################################################################
# On average, the ``quantal_stp_synapse`` converges to the ``tsodyks2_synapse``,
# so we can compare the two by running multiple trials.
#
# First we define simulation time step and random seed

resolution = 0.1  # [ms]
seed = 12345

# We define the number of trials as well as the number of release sites.

n_sites = 10.0  # number of synaptic release sites
n_trials = 500  # number of measurement trials

# The pre-synaptic neuron is driven by an injected current for a part of each
# simulation cycle. We define here the parameters for this stimulation cycle.

I_stim = 376.0   # [pA] stimulation current
T_on = 500.0     # [ms] stimulation is on
T_off = 1000.0   # [ms] stimulation is off

T_cycle = T_on + T_off   # total duration of each stimulation cycle

###############################################################################
# Next, we define parameter sets for facilitation and initial weight.

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
qsyn_params["n"] = n_sites

###############################################################################
# To make the responses comparable, we have to scale the weight by the
# number of release sites.

qsyn_params["weight"] = 1. / n_sites

###############################################################################
# We reset NEST to have a well-defined starting point,
# make NEST less verbose, and set some kernel parameters.
nest.ResetKernel()
nest.set_verbosity("M_ERROR")
nest.SetKernelStatus({"resolution": resolution,
                      "rng_seed": seed})

###############################################################################
# We create three different neurons.
# Neuron one is the sender, the two other neurons receive the synapses.
# We exploit Python's unpacking mechanism to assign the neurons to named
# variables directly.

pre_neuron, tsyn_neuron, qsyn_neuron = nest.Create("iaf_psc_exp",
                                                   params={"tau_syn_ex": 3.},
                                                   n=3)

###############################################################################
# We create two voltmeters, one for each of the postsynaptic neurons.
# We start recording only after a first cycle, which is used for equilibration.

tsyn_voltmeter, qsyn_voltmeter = nest.Create("voltmeter",
                                             params={"start": T_cycle,
                                                     "interval": resolution},
                                             n=2)

###############################################################################
# Connect one neuron with the deterministic tsodyks2 synapse and the other neuron
# with the stochastic quantal stp synapse; then, connect a voltmeter to each neuron.
# Here, ``**tsyn_params`` inserts the content of the ``tsyn_params`` dict into the
# dict passed to ``syn_spec``.

nest.Connect(pre_neuron, tsyn_neuron,
             syn_spec={"synapse_model": "tsodyks2_synapse", **tsyn_params})

# For technical reasons, we currently must set the parameters of the
# quantal_stp_synapse via default values. This will change in a future version
# of NEST.
nest.SetDefaults("quantal_stp_synapse", qsyn_params)
nest.Connect(pre_neuron, qsyn_neuron, syn_spec={"synapse_model": "quantal_stp_synapse"})

nest.Connect(tsyn_voltmeter, tsyn_neuron)
nest.Connect(qsyn_voltmeter, qsyn_neuron)

###############################################################################
# This loop runs over the `n_trials` trials and performs a standard protocol
# of a high-rate response, followed by a pause and then a recovery response.
#
# We actually run over ``n_trials + 1`` rounds, since the first trial is for
# equilibration and is not recorded (see voltmeter parameters above).
#
# We use the NEST ``:class:.RunManager`` to improve performance and call ``:func:.Run``
# inside for each part of the simulation.
#
# We print a line of breadcrumbs to indicate progress.

print(f"Simulating {n_trials} times ", end="", flush=True)
with nest.RunManager():
    for t in range(n_trials + 1):
        pre_neuron.I_e = I_stim
        nest.Run(T_on)

        pre_neuron.I_e = 0.0
        nest.Run(T_off)

        if t % 10 == 0:
            print(".", end="", flush=True)
print()

###############################################################################
# Simulate one additional time step. This ensures that the
# voltage traces for all trials, including the last, have the full length, so we
# can easily transform them into a matrix below.
nest.Simulate(nest.GetKernelStatus('resolution'))

###############################################################################
# Extract voltage traces and reshape the matrix with one column per trial
# and one row per time step. NEST returns results as NumPy arrays.
# We extract times only once and keep only times for a single trial.
vm_tsyn = tsyn_voltmeter.get("events", "V_m")
vm_qsyn = qsyn_voltmeter.get("events", "V_m")

steps_per_trial = round(T_cycle / tsyn_voltmeter.get("interval"))
vm_tsyn.shape = (n_trials, steps_per_trial)
vm_qsyn.shape = (n_trials, steps_per_trial)

t_vm = tsyn_voltmeter.get("events", "times")
t_trial = t_vm[:steps_per_trial]

###############################################################################
# Now compute the mean of all trials and plot against trials and references.

vm_tsyn_mean = vm_tsyn.mean(axis=0)
vm_qsyn_mean = vm_qsyn.mean(axis=0)
rms_error = ((vm_tsyn_mean - vm_qsyn_mean) ** 2).mean() ** 0.5

plt.plot(t_trial, vm_tsyn_mean, lw=2, alpha=0.7,
         label="Tsodyks-2 synapse (deterministic)")
plt.plot(t_trial, vm_qsyn_mean, lw=2, alpha=0.7,
         label="Quantal STP synapse (stochastic)")
plt.xlabel("Time [ms]")
plt.ylabel("Membrane potential [mV]")
plt.title("Comparison of deterministic and stochastic plasicity rules")
plt.text(0.95, 0.05, f"RMS error: {rms_error:.3g}",
         horizontalalignment="right", verticalalignment="bottom",
         transform=plt.gca().transAxes)  # relative coordinates for text placement
plt.legend()
plt.show()
