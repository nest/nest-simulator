# -*- coding: utf-8 -*-
#
# cuba.py
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
Benchmark 2 of the simulator review (CUBA)
-------------------------------------------

This script creates a sparsely coupled network of excitatory and inhibitory
neurons which exhibits self-sustained activity after an initial stimulus.
Connections within and across both populations are created at random. Both
neuron populations receive Poissonian background input. The spike output of
500 neurons from each population are recorded. Neurons are modeled as leaky
integrate-and-fire neurons with current-based synapses (exponential functions).
The model is based on the Vogels & Abbott network model [1]_.

This is Benchmark 2 of the FACETS simulator review (Brette et al. 2007):
- Neuron model: integrate-and-fire (i&f)
- Synapse model: current-based (CUBA)
- Synapse time course: exponential
- Spike times: grid-constrained

Note that the simulation time is set 10 times longer than for benchmark 1
(coba.py) or benchmark 3 (hh_coba.py). This is necessary as the computational
load here is much lower, so a longer simulation time is necessary to make
reasonable measurements.

References
~~~~~~~~~~

.. [1] Vogels TP, Abbott LF. 2005. Signal propagation and logic gating in
       networks of integrate-and-fire neurons. Journal of Neuroscience.
       25(46):10786-10795.
       https://doi.org/10.1523/JNEUROSCI.3508-05.2005

.. [2] Brette R, Rudolph M, Carnevale T, Hines M, Beeman D, Bower JM, et al.
       2007. Simulation of networks of spiking neurons: a review of tools and
       strategies. Journal of Computational Neuroscience. 23(3):349-398.
       https://doi.org/10.1007/s10827-007-0038-6

"""

import nest
from brette_et_al_2007_benchmark import run_simulation

###############################################################################
# Set benchmark parameters

params = {
    "model": "iaf_psc_exp",  # neuron model
    "model_params": {
        "E_L": -49.0,  # resting membrane potential [mV]
        # see Brette et al, J Comput Neurosci 23:349 (2007), p 393
        "V_m": -49.0,  # initial membrane potential [mV]
        "V_th": -50.0,  # threshold [mV]
        "V_reset": -60.0,  # reset potential [mV]
        "C_m": 200.0,  # capacity of the membrane [pF]
        "tau_m": 20.0,  # membrane time constant [ms]
        "tau_syn_ex": 5.0,  # time const. postsynaptic excitatory currents [ms]
        "tau_syn_in": 10.0,  # time const. postsynaptic inhibitory currents [ms]
        "t_ref": 5.0,  # duration of refractory period [ms]
    },
    "delay": 0.1,  # synaptic delay [ms]
    "E_synapse_params": {
        "weight": 16.2,  # excitatory PSC amplitude [pA]
    },
    "I_synapse_params": {
        "weight": -139.5,  # inhibitory PSC amplitude [pA]
    },
    "stimulus": "poisson_generator",
    "stimulus_params": {
        "rate": 300.0,  # rate of initial poisson stimulus [spikes/s]
        "start": 1.0,  # start of Poisson generator [ms]
        "stop": 51.0,  # stop of Poisson generator [ms]
        "origin": 0.0,  # origin of time [ms]
    },
    "recorder": "spike_recorder",
    "recorder_params": {
        "record_to": "ascii",
        "label": "cuba",
    },
    "Nrec": 500,  # number of neurons per population to record from
    "Nstim": 50,  # number of neurons to stimulate
    "simtime": 10000.0,  # simulated time [ms] (10x longer than coba/hh_coba)
    "dt": 0.1,  # simulation step [ms]
    "NE": 3200,  # number of excitatory neurons
    "NI": 800,  # number of inhibitory neurons
    "epsilon": 0.02,  # connection probability
    "virtual_processes": 1,  # number of virtual processes to use
}

###############################################################################
# Run the simulation

if __name__ == "__main__":
    nest.set_verbosity("M_WARNING")
    results = run_simulation(params)
