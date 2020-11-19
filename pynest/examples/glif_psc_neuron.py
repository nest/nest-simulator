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
------------------------------------------------------------------------

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
# We also pre-define the synapse time constant array, [2.0, 1.0] ms for
# the two desired synaptic ports of the GLIF neurons. Note that the default
# synapse time constant is [2.0] ms, which is for neuron with one port.

syn_tau = [2.0, 1.0]

###############################################################################
# We create the five levels of GLIF model to be tested, i.e.,
# ``lif``, ``lif_r``, ``lif_asc``, ``lif_r_asc``, ``lif_r_asc_a``.
# For each level of GLIF model, we create  a ``glif_psc`` node. The node is
# created by setting relative model mechanism parameters and the time constant
# of the 2 synaptic ports as mentioned above. Other neuron parameters are set
# as default. The five ``glif_psc`` node handles were combined as a list.

n_lif = nest.Create("glif_psc",
                    params={"spike_dependent_threshold": False,
                            "after_spike_currents": False,
                            "adapting_threshold": False,
                            "tau_syn": syn_tau})
n_lif_r = nest.Create("glif_psc",
                      params={"spike_dependent_threshold": True,
                              "after_spike_currents": False,
                              "adapting_threshold": False,
                              "tau_syn": syn_tau})
n_lif_asc = nest.Create("glif_psc",
                        params={"spike_dependent_threshold": False,
                                "after_spike_currents": True,
                                "adapting_threshold": False,
                                "tau_syn": syn_tau})
n_lif_r_asc = nest.Create("glif_psc",
                          params={"spike_dependent_threshold": True,
                                  "after_spike_currents": True,
                                  "adapting_threshold": False,
                                  "tau_syn": syn_tau})
n_lif_r_asc_a = nest.Create("glif_psc",
                            params={"spike_dependent_threshold": True,
                                    "after_spike_currents": True,
                                    "adapting_threshold": True,
                                    "tau_syn": syn_tau})

neurons = n_lif + n_lif_r + n_lif_asc + n_lif_r_asc + n_lif_r_asc_a

###############################################################################
# For the stimulation input to the glif_psc neurons, we create one excitation
# spike generator and one inhibition spike generator, each of which generates
# three spikes; we also create one step current generator and a Poisson
# generator, a parrot neuron (to be paired with the Poisson generator).
# The three different injections are spread to three different time periods,
# i.e., 0 ms ~ 200 ms, 200 ms ~ 500 ms, 600 ms ~ 900 ms.
# Each of the excitation and inhibition spike generators generates three spikes
# at different time points. Configuration of the current generator includes the
# definition of the start and stop times and the amplitude of the injected
# current. Configuration of the Poisson generator includes the definition of
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
# The generators are then connected to the neurons. Specification of
# the ``receptor_type`` uniquely defines the target receptor.
# We connect current generator, the spike generators, Poisson generator (via
# parrot neuron) to receptor 0, 1, and 2 of the GLIF neurons, respectively.
# Note that Poisson generator is connected to parrot neuron to transit the
# spikes to the glif_psc neuron.

nest.Connect(cg, neurons, syn_spec={"delay": resolution})
nest.Connect(espikes, neurons,
             syn_spec={"delay": resolution, "receptor_type": 1})
nest.Connect(ispikes, neurons,
             syn_spec={"delay": resolution, "receptor_type": 1})
nest.Connect(pg, pn, syn_spec={"delay": resolution})
nest.Connect(pn, neurons, syn_spec={"delay": resolution, "receptor_type": 2})

###############################################################################
# A ``multimeter`` is created and connected to the neurons. The parameters
# specified for the multimeter include the list of quantities that should be
# recorded and the time interval at which quantities are measured.

mm = nest.Create("multimeter",
                 params={"interval": resolution,
                         "record_from": ["V_m", "I", "I_syn", "threshold",
                                         "threshold_spike",
                                         "threshold_voltage",
                                         "ASCurrents_sum"]})
nest.Connect(mm, neurons)

###############################################################################
# A ``spike_recorder`` is created and connected to the neurons record the
# spikes generated by the glif_psc neurons.

sr = nest.Create("spike_recorder")
nest.Connect(neurons, sr)

###############################################################################
# Run the simulation for 1000 ms and retrieve recorded data from
# the multimeter and spike recorder.

nest.Simulate(1000.)

data = mm.events
senders = data["senders"]

spike_data = sr.events
spike_senders = spike_data["senders"]
spikes = spike_data["times"]

###############################################################################
# We plot the time traces of the membrane potential (in blue) and
# the overall threshold (in green), and the spikes (as red dots) in one panel;
# the spike component of threshold (in yellow) and the voltage component of
# threshold (in black) in another panel; the injected currents (in strong blue),
# the sum of after spike currents (in cyan), and the synaptic currents (in
# magenta) in responding to the spike inputs to the neurons in the third panel.
# We plot all these three panels for each level of GLIF model in a separated
# figure.

glif_models = ["lif", "lif_r", "lif_asc", "lif_r_asc", "lif_r_asc_a"]
for i in range(len(glif_models)):

    glif_model = glif_models[i]
    node_id = neurons[i].global_id
    plt.figure(glif_model)
    gs = gridspec.GridSpec(3, 1, height_ratios=[2, 1, 1])
    t = data["times"][senders == 1]

    ax1 = plt.subplot(gs[0])
    plt.plot(t, data["V_m"][senders == node_id], "b")
    plt.plot(t, data["threshold"][senders == node_id], "g--")
    plt.plot(spikes[spike_senders == node_id],
             [max(data["threshold"][senders == node_id]) * 0.95] *
             len(spikes[spike_senders == node_id]), "r.")
    plt.legend(["V_m", "threshold", "spike"])
    plt.ylabel("V (mV)")
    plt.title("Simulation of glif_psc neuron of " + glif_model)

    ax2 = plt.subplot(gs[1])
    plt.plot(t, data["threshold_spike"][senders == node_id], "y")
    plt.plot(t, data["threshold_voltage"][senders == node_id], "k--")
    plt.legend(["threshold_spike", "threshold_voltage"])
    plt.ylabel("V (mV)")

    ax3 = plt.subplot(gs[2])
    plt.plot(t, data["I"][senders == node_id], "--")
    plt.plot(t, data["ASCurrents_sum"][senders == node_id], "c-.")
    plt.plot(t, data["I_syn"][senders == node_id], "m")
    plt.legend(["I_e", "ASCurrents_sum", "I_syn"])
    plt.ylabel("I (pA)")
    plt.xlabel("t (ms)")

plt.show()
