# -*- coding: utf-8 -*-
#
# CampbellSiegert.py
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

"""Campbell & Siegert approximation example
----------------------------------------------

Example script that applies Campbell's theorem and Siegert's rate
approximation to and integrate-and-fire neuron.

This script calculates the firing rate of an integrate-and-fire neuron
in response to a series of Poisson generators, each specified with a
rate and a synaptic weight. The calculated rate is compared with a
simulation using the ``iaf_psc_alpha`` model



References:
~~~~~~~~~~~~

 .. [1] Papoulis A (1991). Probability, Random Variables, and
        Stochastic Processes, McGraw-Hill
 .. [2] Siegert AJ (1951). On the first passage time probability problem,
        Phys Rev 81: 617-623

Authors
~~~~~~~~

S. Schrader, Siegert implentation by T. Tetzlaff
"""

###############################################################################
# First, we import all necessary modules for simulation and analysis. Scipy
# should be imported before nest.

from scipy.special import erf
from scipy.optimize import fmin

import numpy as np

import nest

###############################################################################
# We first set the parameters of neurons, noise and the simulation. First
# settings are with a single Poisson source, second is with two Poisson
# sources with half the rate of the single source. Both should lead to the
# same results.

weights = [0.1]    # (mV) psp amplitudes
rates = [10000.]   # (1/s) rate of Poisson sources
# weights = [0.1, 0.1]    # (mV) psp amplitudes
# rates = [5000., 5000.]  # (1/s) rate of Poisson sources

C_m = 250.0       # (pF) capacitance
E_L = -70.0       # (mV) resting potential
I_e = 0.0         # (nA) external current
V_reset = -70.0   # (mV) reset potential
V_th = -55.0      # (mV) firing threshold
t_ref = 2.0       # (ms) refractory period
tau_m = 10.0      # (ms) membrane time constant
tau_syn_ex = .5   # (ms) excitatory synaptic time constant
tau_syn_in = 2.0  # (ms) inhibitory synaptic time constant

simtime = 20000  # (ms) duration of simulation
n_neurons = 10   # number of simulated neurons

###############################################################################
# For convenience we define some units.

pF = 1e-12
ms = 1e-3
pA = 1e-12
mV = 1e-3

mu = 0.0
sigma2 = 0.0
J = []

assert(len(weights) == len(rates))

###############################################################################
# In the following we analytically compute the firing rate of the neuron
# based on Campbell's theorem [1]_ and Siegerts approximation [2]_.

for rate, weight in zip(rates, weights):

    if weight > 0:
        tau_syn = tau_syn_ex
    else:
        tau_syn = tau_syn_in

    t_psp = np.arange(0., 10. * (tau_m * ms + tau_syn * ms), 0.0001)

    # We define the form of a single PSP, which allows us to match the
    # maximal value to or chosen weight.

    def psp(x):
        return - ((C_m * pF) / (tau_syn * ms) * (1 / (C_m * pF)) *
                  (np.exp(1) / (tau_syn * ms)) *
                  (((-x * np.exp(-x / (tau_syn * ms))) /
                    (1 / (tau_syn * ms) - 1 / (tau_m * ms))) +
                   (np.exp(-x / (tau_m * ms)) - np.exp(-x / (tau_syn * ms))) /
                   ((1 / (tau_syn * ms) - 1 / (tau_m * ms)) ** 2)))

    min_result = fmin(psp, [0], full_output=1, disp=0)

    # We need to calculate the PSC amplitude (i.e., the weight we set in NEST)
    # from the PSP amplitude, that we have specified above.

    fudge = -1. / min_result[1]
    J.append(C_m * weight / (tau_syn) * fudge)

    # We now use Campbell's theorem to calculate mean and variance of
    # the input due to the Poisson sources. The mean and variance add up
    # for each Poisson source.

    mu += (rate * (J[-1] * pA) * (tau_syn * ms) *
           np.exp(1) * (tau_m * ms) / (C_m * pF))

    sigma2 += rate * (2 * tau_m * ms + tau_syn * ms) * \
        (J[-1] * pA * tau_syn * ms * np.exp(1) * tau_m * ms /
         (2 * (C_m * pF) * (tau_m * ms + tau_syn * ms))) ** 2

mu += (E_L * mV)
sigma = np.sqrt(sigma2)

###############################################################################
# Having calculate mean and variance of the input, we can now employ
# Siegert's rate approximation.

num_iterations = 100
upper = (V_th * mV - mu) / (sigma * np.sqrt(2))
lower = (E_L * mV - mu) / (sigma * np.sqrt(2))
interval = (upper - lower) / num_iterations
tmpsum = 0.0
for cu in range(0, num_iterations + 1):
    u = lower + cu * interval
    f = np.exp(u ** 2) * (1 + erf(u))
    tmpsum += interval * np.sqrt(np.pi) * f
r = 1. / (t_ref * ms + tau_m * ms * tmpsum)

###############################################################################
# We now simulate neurons receiving Poisson spike trains as input,
# and compare the theoretical result to the empirical value.

nest.ResetKernel()

nest.set_verbosity('M_WARNING')
neurondict = {'V_th': V_th,
              'tau_m': tau_m,
              'tau_syn_ex': tau_syn_ex,
              'tau_syn_in': tau_syn_in,
              'C_m': C_m,
              'E_L': E_L,
              't_ref': t_ref,
              'V_m': E_L,
              'V_reset': E_L}

###############################################################################
# Neurons and devices are instantiated. We set a high threshold as we want
# free membrane potential. In addition we choose a small resolution for
# recording the membrane to collect good statistics.

nest.SetDefaults('iaf_psc_alpha', neurondict)
n = nest.Create('iaf_psc_alpha', n_neurons)
n_free = nest.Create('iaf_psc_alpha', 1, {'V_th': 1e12})
pg = nest.Create('poisson_generator', len(rates),
                 [{'rate': float(rate_i)} for rate_i in rates])
vm = nest.Create('voltmeter', 1, {'interval': .1})
sd = nest.Create('spike_detector')

###############################################################################
# We connect devices and neurons and start the simulation.

for indx in range(len(pg)):
    nest.Connect(pg[indx], n,
                 syn_spec={'weight': float(J[indx]), 'delay': 0.1})
    nest.Connect(pg[indx], n_free, syn_spec={'weight': J[indx]})

nest.Connect(vm, n_free)
nest.Connect(n, sd)

nest.Simulate(simtime)

###############################################################################
# Here we read out the recorded membrane potential. The first 500 steps are
# omitted so initial transients do not perturb our results. We then print the
# results from theory and simulation.

v_free = vm.get('events', 'V_m')[500:-1]
print('mean membrane potential (actual / calculated): {0} / {1}'
      .format(np.mean(v_free), mu * 1000))
print('variance (actual / calculated): {0} / {1}'
      .format(np.var(v_free), sigma2 * 1e6))
print('firing rate (actual / calculated): {0} / {1}'
      .format(nest.GetStatus(sd, 'n_events')[0] /
              (n_neurons * simtime * ms), r))
