# -*- coding: utf-8 -*-
#
# glif_psc_neuron.py
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
Current-based generalized leaky integrate and fire (GLIF) neuron example
--------------------------------

Simple example of how to use the ``glif_psc`` neuron model for
five different levels of GLIF neurons.

Four stimulation paradigms are illustrated for the GLIF model
with externally applied current and spikes impinging

Voltage traces, current traces, threshold traces, and spikes are shown.

KEYWORDS: glif_psc
"""

##############################################################################
# First, we import all necessary modules to simulate, analyze and plot this
# example.

import nest
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

##############################################################################
# We initialize the nest and set the simulation resolution.

nest.ResetKernel()
resolution = 0.05
nest.SetKernelStatus({"resolution": resolution})

##############################################################################
# We define functions for creation of the five GLIF models
# using the GLIF parameters of cell 490626718 in Allen Cell Type Database
# (ACTD). The parameters is available at
# <https://celltypes.brain-map.org/mouse/experiment/electrophysiology/490626718>
# with units being converted from SI units( i.e., V, S( 1 / Ohm ), F, s, A ) to
# NEST used units( i.e., mV, nS( 1 / GOhm ), pF, ms, pA ) and values being
# rounded to appropriate digits for simplification.
# Note that the rounded values of the Level 5 GLIF model of such a cell are
# the default parameter values of ``glif_psc`` neuron.
# Each function includes effected parameters for each level of GLIF model.
# Also, the initial membrane potential V_m is set to E_L.


def create_lif_psc(mechs, syn_tau):
    """Creates a nest glif_psc object for GLIF Level 1 (LIF) neuron"""
    return nest.Create("glif_psc",
                       params={"C_m": 58.72,
                               "E_L": -78.85,
                               "V_m": -78.85,
                               "V_reset": -78.85,
                               "V_th": -37.88,
                               "g": 9.12,
                               "t_ref": 3.75,
                               "tau_syn": syn_tau,
                               "spike_dependent_threshold": mechs[0],  # False
                               "after_spike_currents": mechs[1],  # False
                               "adapting_threshold": mechs[2]})  # False


def create_lif_r_psc(mechs, syn_tau):
    """Creates a nest glif_psc object for GLIF Level 2 (LIF-R) neuron"""
    return nest.Create("glif_psc",
                       params={"C_m": 58.72,
                               "E_L": -78.85,
                               "V_m": -78.85,
                               "V_th": -37.14,
                               "g": 9.12,
                               "t_ref": 3.75,
                               "tau_syn": syn_tau,
                               "th_spike_add": 0.37,
                               "th_spike_decay": 0.009,
                               "voltage_reset_add": 18.56,
                               "voltage_reset_fraction": 0.20,
                               "spike_dependent_threshold": mechs[0],  # True
                               "after_spike_currents": mechs[1],  # False
                               "adapting_threshold": mechs[2]})  # False


def create_lif_asc_psc(mechs, syn_tau):
    """Creates a nest glif_psc object for GLIF Level 3 (LIF-ASC) neuron"""
    return nest.Create("glif_psc",
                       params={"C_m": 58.72,
                               "E_L": -78.85,
                               "V_m": -78.85,
                               "V_reset": -78.85,
                               "V_th": -50.19,
                               "asc_amps": [-9.18, -198.94],
                               "asc_decay": [0.003, 0.1],
                               "asc_init": [0.0, 0.0],
                               "asc_r": [1.0, 1.0],
                               "g": 9.43,
                               "t_ref": 3.75,
                               "tau_syn": syn_tau,
                               "spike_dependent_threshold": mechs[0],  # False
                               "after_spike_currents": mechs[1],  # True
                               "adapting_threshold": mechs[2]})  # False


def create_lif_r_asc_psc(mechs, syn_tau):
    """Creates a nest glif_psc object for GLIF Level 4 (LIF-R-ASC) neuron"""
    return nest.Create("glif_psc",
                       params={"C_m": 58.72,
                               "E_L": -78.85,
                               "V_m": -78.85,
                               "V_th": -50.19,
                               "asc_amps": [-9.18, -198.94],
                               "asc_decay": [0.003, 0.1],
                               "asc_init": [0.0, 0.0],
                               "asc_r": [1.0, 1.0],
                               "g": 9.43,
                               "t_ref": 3.75,
                               "tau_syn": syn_tau,
                               "th_spike_add": 0.37,
                               "th_spike_decay": 0.009,
                               "voltage_reset_add": 18.56,
                               "voltage_reset_fraction": 0.20,
                               "spike_dependent_threshold": mechs[0],  # True
                               "after_spike_currents": mechs[1],  # True
                               "adapting_threshold": mechs[2]})  # False


def create_lif_r_asc_a_psc(mechs, syn_tau):
    """Creates a nest glif_psc object for GLIF Level 5 (LIF-R-ASC-A) neuron"""
    return nest.Create("glif_psc",
                       params={"C_m": 58.72,
                               "E_L": -78.85,
                               "V_m": -78.85,
                               "V_th": -51.68,
                               "asc_amps": [-9.18, -198.94],
                               "asc_decay": [0.003, 0.1],
                               "asc_init": [0.0, 0.0],
                               "asc_r": [1.0, 1.0],
                               "g": 9.43,
                               "t_ref": 3.75,
                               "tau_syn": syn_tau,
                               "th_spike_add": 0.37,
                               "th_spike_decay": 0.009,
                               "th_voltage_decay": 0.09,
                               "th_voltage_index": 0.005,
                               "voltage_reset_add": 18.56,
                               "voltage_reset_fraction": 0.20,
                               "spike_dependent_threshold": mechs[0],  # True
                               "after_spike_currents": mechs[1],  # True
                               "adapting_threshold": mechs[2]})  # True

##############################################################################
# We pre-define model mechanism parameters for each level of GLIF models
# in the order of[spike_dependent_threshold, after_spike_currents,
# adapting_threshold].We also pre - define the synapse time constant array,
# [2.0, 1.0] ms for two desgired synaptic ports of the neuron.

model_mech_dict = {"lif": [False, False, False],
                   "lif_r": [True, False, False],
                   "lif_asc": [False, True, False],
                   "lif_r_asc": [True, True, False],
                   "lif_r_asc_a": [True, True, True]}

syn_tau = [2.0, 1.0]

###############################################################################
# We pick one of the five GLIF model levels to be tested from
# ("lif", "lif_r", "lif_asc", "lif_r_asc", "lif_r_asc_a").
# Test "lif" model if name other than the five above names is set.
# For the picked GLIF model, model mechanisms are retrieved to create
# ``glif_psc`` node.The node is created by calling the above defined function
# for the picked model.We store the returned handle in variable for later
# reference.

glif_model = "lif"  # "lif", "lif_r", "lif_asc", "lif_r_asc", or "lif_r_asc_a"

if glif_model not in model_mech_dict.keys():
    glif_model = 'lif'

mechs = model_mech_dict[glif_model]

if glif_model == "lif":
    neuron = create_lif_psc(mechs, syn_tau)
elif glif_model == "lif_r":
    neuron = create_lif_r_psc(mechs, syn_tau)
elif glif_model == "lif_asc":
    neuron = create_lif_asc_psc(mechs, syn_tau)
elif glif_model == "lif_r_asc":
    neuron = create_lif_r_asc_psc(mechs, syn_tau)
elif glif_model == "lif_r_asc_a":
    neuron = create_lif_r_asc_a_psc(mechs, syn_tau)
else:
    neuron = create_lif_psc(mechs, syn_tau)

###############################################################################
# For the stimulation input to the glif_psc neuron, we create one excitory
# spike generator and one inhibitory spike generator, each of which generates
# three spikes; we also create one step current generator and a poisson
# generator, a parrot neuron(to be paired with the poisson generator).
# The three different injections are spread to three different periods,
# i.e., 0 ms ~200 ms, 200 ms ~500 ms, 600 ms ~900 ms.
# Each of the excitory and inhibitory spike generators generates three spikes
# at different times.Configuration of the current generator includes the
# definition of the start and stop times and the amplitude of the injected
# current. Configuration of the poisson generator includes the definition of
# the start and stop times and the rate of the injected spike train.

espikes = nest.Create("spike_generator",
                      params={"spike_times": [10., 100., 150.],
                              "spike_weights": [20.]*3})
ispikes = nest.Create("spike_generator",
                      params={"spike_times": [15., 99., 150.],
                              "spike_weights": [-20.]*3})
cg = nest.Create("step_current_generator",
                 params={"amplitude_values": [400., ],
                         "amplitude_times": [200., ],
                         "start": 200., "stop": 500.})
pg = nest.Create("poisson_generator",
                 params={"rate": 150000., "start": 600., "stop": 900.})
pn = nest.Create("parrot_neuron")

###############################################################################
# The generators are then connected to the neuron.Specification of
# the ``receptor_type`` uniquely defines the target receptor.
# We connect the spike generators, current generator, poisson generator( via
# parrot neuron ) to receptor 0, 1, and2 of the neuron, respectively.
# Note that poisson generator is connected to parrot neuron to transit the
# spike to the glif_psc neuron.

nest.Connect(cg, neuron, syn_spec={"delay": resolution})
nest.Connect(espikes, neuron,
             syn_spec={"delay": resolution, "receptor_type": 1})
nest.Connect(ispikes, neuron,
             syn_spec={"delay": resolution, "receptor_type": 1})
nest.Connect(pg, pn, syn_spec={"delay": resolution})
nest.Connect(pn, neuron, syn_spec={"delay": resolution, "receptor_type": 2})

###############################################################################
# A `multimeter` is created and connected to the neuron.The parameters
# specified for the multimeter include the list of quantities that should be
# recorded and the time interval at which quantities are measured.

mm = nest.Create("multimeter",
                 params={"interval": resolution,
                         "record_from": ["V_m", "I", "I_syn", "threshold",
                                         "threshold_spike",
                                         "threshold_voltage",
                                         "ASCurrents_sum"]})
nest.Connect(mm, neuron)

###############################################################################
# A `spike_detector` is created and connected to the neuron record the spikes
# generated by the glif_psc neuron.

sd = nest.Create("spike_detector")
nest.Connect(neuron, sd)

###############################################################################
# Run the simulation for 1000 ms and retrieve recorded data from
# the multimeter and spike detector.

nest.Simulate(1000.)

data = nest.GetStatus(mm, "events")[0]
spikes = nest.GetStatus(sd)[0]["events"]["times"]

###############################################################################
# We create an array with the time points when the quantities were actually
# recorded.

t = data["times"]

###############################################################################
# We plot the time traces of the membrane potential (in blue) and
# the overall threshold (in green), and the spikes (as red dots) in one panel;
# the spike component of threshold (in yellow) and the voltage component of
# threshold (in black) in another panel; the injected currents(in strong blue),
# the sum of after spike currents(in cyan), and the synaptic currents (in
# magenta) in responding to the spike inputs to the neuron in the third panel.

plt.figure()
gs = gridspec.GridSpec(3, 1, height_ratios=[2, 1, 1])

ax1 = plt.subplot(gs[0])
plt.plot(t, data["V_m"], "b")
plt.plot(t, data["threshold"], "g--")
plt.plot(spikes, [max(data["threshold"]) * 0.95] * len(spikes), "r.")
plt.legend(["V_m", "threshold", "spike"])
plt.ylabel("V (mV)")
plt.title("Simulation of glif_psc neuron for " + glif_model)

ax2 = plt.subplot(gs[1])
plt.plot(t, data["threshold_spike"], "y")
plt.plot(t, data["threshold_voltage"], "k--")
plt.legend(["threshold_spike", "threshold_voltage"])
plt.ylabel("V (mV)")

ax3 = plt.subplot(gs[2])
plt.plot(t, data["I"], "--")
plt.plot(t, data["ASCurrents_sum"], "c-.")
plt.plot(t, data["I_syn"], "m")
plt.legend(["I_e", "ASCurrents_sum", "I_syn"])
plt.ylabel("I (pA)")
plt.xlabel("t (ms)")

plt.show()
