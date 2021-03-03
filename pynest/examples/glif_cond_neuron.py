# -*- coding: utf-8 -*-
#
# glif_cond_neuron.py
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
Conductance-based generalized leaky integrate and fire (GLIF) neuron example
----------------------------------------------------------------------------

Simple example of how to use the ``glif_cond`` neuron model for
five different levels of GLIF neurons.

Four stimulation paradigms are illustrated for the GLIF model
with externally applied current and spikes impinging

Voltage traces, injecting current traces, threshold traces, synaptic
conductance traces and spikes are shown.

KEYWORDS: glif_cond
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

###############################################################################
# We create the five levels of GLIF model to be tested, i.e.,
# ``lif``, ``lif_r``, ``lif_asc``, ``lif_r_asc``, ``lif_r_asc_a``.
# For each level of GLIF model, we create  a ``glif_cond`` node. The node is
# created by setting relative model mechanism parameters. Other neuron
# parameters are set as default. The five ``glif_cond`` node handles are
# combined as a list. Note that the default number of synaptic ports
# is two for spike inputs. One port is excitation receptor with time
# constant being 0.2 ms and reversal potential being 0.0 mV. The other port is
# inhibition receptor with time constant being 2.0 ms and -85.0 mV.
# Note that users can set as many synaptic ports as needed for ``glif_cond``
# by setting array parameters ``tau_syn`` and ``E_rev`` of the model.

n_lif = nest.Create("glif_cond",
                    params={"spike_dependent_threshold": False,
                            "after_spike_currents": False,
                            "adapting_threshold": False})
n_lif_r = nest.Create("glif_cond",
                      params={"spike_dependent_threshold": True,
                              "after_spike_currents": False,
                              "adapting_threshold": False})
n_lif_asc = nest.Create("glif_cond",
                        params={"spike_dependent_threshold": False,
                                "after_spike_currents": True,
                                "adapting_threshold": False})
n_lif_r_asc = nest.Create("glif_cond",
                          params={"spike_dependent_threshold": True,
                                  "after_spike_currents": True,
                                  "adapting_threshold": False})
n_lif_r_asc_a = nest.Create("glif_cond",
                            params={"spike_dependent_threshold": True,
                                    "after_spike_currents": True,
                                    "adapting_threshold": True})

neurons = n_lif + n_lif_r + n_lif_asc + n_lif_r_asc + n_lif_r_asc_a

###############################################################################
# For the stimulation input to the glif_cond neurons, we create one excitation
# spike generator and one inhibition spike generator, each of which generates
# three spikes; we also create one step current generator and a Poisson
# generator, a parrot neuron(to be paired with the Poisson generator).
# The three different injections are spread to three different time periods,
# i.e., 0 ms ~ 200 ms, 200 ms ~ 500 ms, 600 ms ~ 900 ms.
# Configuration of the current generator includes the definition of the start
# and stop times and the amplitude of the injected current. Configuration of
# the Poisson generator includes the definition of the start and stop times and
# the rate of the injected spike train.

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
                 params={"rate": 15000., "start": 600., "stop": 900.})
pn = nest.Create("parrot_neuron")

###############################################################################
# The generators are then connected to the neurons. Specification of
# the ``receptor_type`` uniquely defines the target receptor.
# We connect current generator to receptor 0, the excitation spike generator
# and the Poisson generator (via parrot neuron) to receptor 1, and the
# inhibition spike generator to receptor 2 of the GLIF neurons.
# Note that Poisson generator is connected to parrot neuron to transit the
# spikes to the glif_cond neuron.

nest.Connect(cg, neurons, syn_spec={"delay": resolution})
nest.Connect(espikes, neurons,
             syn_spec={"delay": resolution, "receptor_type": 1})
nest.Connect(ispikes, neurons,
             syn_spec={"delay": resolution, "receptor_type": 2})
nest.Connect(pg, pn, syn_spec={"delay": resolution})
nest.Connect(pn, neurons, syn_spec={"delay": resolution, "receptor_type": 1})

###############################################################################
# A ``multimeter`` is created and connected to the neurons. The parameters
# specified for the multimeter include the list of quantities that should be
# recorded and the time interval at which quantities are measured.

mm = nest.Create("multimeter",
                 params={"interval": resolution,
                         "record_from": ["V_m", "I", "g_1", "g_2",
                                         "threshold",
                                         "threshold_spike",
                                         "threshold_voltage",
                                         "ASCurrents_sum"]})
nest.Connect(mm, neurons)

###############################################################################
# A ``spike_recorder`` is created and connected to the neurons record the
# spikes generated by the glif_cond neurons.

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
# the sum of after spike currents (in cyan) in the third panel; and the synaptic
# conductances of the two receptors (in blue and orange) in responding to the
# spike inputs to the neurons in the fourth panel. We plot all these four
# panels for each level of GLIF model in a separated figure.

glif_models = ["lif", "lif_r", "lif_asc", "lif_r_asc", "lif_r_asc_a"]
for i in range(len(glif_models)):

    glif_model = glif_models[i]
    node_id = neurons[i].global_id
    plt.figure(glif_model)
    gs = gridspec.GridSpec(4, 1, height_ratios=[2, 1, 1, 1])
    t = data["times"][senders == 1]

    ax1 = plt.subplot(gs[0])
    plt.plot(t, data["V_m"][senders == node_id], "b")
    plt.plot(t, data["threshold"][senders == node_id], "g--")
    plt.plot(spikes[spike_senders == node_id],
             [max(data["threshold"][senders == node_id]) * 0.95] *
             len(spikes[spike_senders == node_id]), "r.")
    plt.legend(["V_m", "threshold", "spike"])
    plt.ylabel("V (mV)")
    plt.title("Simulation of glif_cond neuron of " + glif_model)

    ax2 = plt.subplot(gs[1])
    plt.plot(t, data["threshold_spike"][senders == node_id], "y")
    plt.plot(t, data["threshold_voltage"][senders == node_id], "k--")
    plt.legend(["threshold_spike", "threshold_voltage"])
    plt.ylabel("V (mV)")

    ax3 = plt.subplot(gs[2])
    plt.plot(t, data["I"][senders == node_id], "--")
    plt.plot(t, data["ASCurrents_sum"][senders == node_id], "c-.")
    plt.legend(["I_e", "ASCurrents_sum", "I_syn"])
    plt.ylabel("I (pA)")
    plt.xlabel("t (ms)")

    ax4 = plt.subplot(gs[3])
    plt.plot(t, data["g_1"][senders == node_id], "-")
    plt.plot(t, data["g_2"][senders == node_id], "--")
    plt.legend(["G_1", "G_2"])
    plt.ylabel("G (nS)")
    plt.xlabel("t (ms)")

plt.show()
