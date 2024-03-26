# -*- coding: utf-8 -*-
#
# glif_psc_double_alpha_neuron.py
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
Current-based generalized leaky integrate and fire (GLIF) neuron with double alpha \
synaptic function
-------------------------------------------------------------------------------------------------------------

Simple example of how to use the ``glif_psc_double_alpha`` neuron model that illustrates
differences from the ``glif_psc`` neuron model.

The behavior of the ``glif_psc_double_alpha`` neuron model is the same as the ``glif_psc``
neuron model, except that the synaptic currents are modeled as a double alpha function.
Therefore, in this example, we only compare the difference in the synaptic currents
between the two models. Compared to the single alpha function, the double alpha function
has much more control over the shape of the tail of the synaptic current.

Simple synaptic inputs are applied to the neuron and the resulting voltage and
current traces are shown for the two models.
"""

##############################################################################
# First, we import all necessary modules to simulate, analyze and plot this
# example.

import matplotlib.gridspec as gridspec
import matplotlib.pyplot as plt
import nest

##############################################################################
# We initialize NEST and set the simulation resolution.

nest.ResetKernel()
resolution = 0.05
nest.resolution = resolution

##############################################################################
# We also pre-define the synapse time constant arrays.
# In contrast to ``glif_psc`` models, ``glif_psc_double_alpha`` models have
# two components of synaptic currents, one for the fast component and the other
# for the slow component. The relative amplitude also needs to be set, so there
# are three parameters to define per receptor port. For this example, we keep the
# ``tau_syn_fast`` to 2 ms for simplicity, and vary the ``tau_syn_slow`` and
# ``amp_slow`` to illustrate how the parameters affect the shape of the synaptic
# currents.

tau_syn_glif_psc = [2.0, 2.0, 2.0]  # value for the ``glif_psc`` model

tau_syn_fast = [2.0, 2.0, 2.0]  # common between 'timing' and 'amp' manipulations

# for the slow component timing manipuation
tau_syn_slow_timing = [4.0, 6.0, 8.0]
amp_slow_timing = [0.5, 0.5, 0.5]

# for the slow component amplitude manipulation
tau_syn_slow_amp = [6.0, 6.0, 6.0]
amp_slow_amp = [0.2, 0.5, 0.8]

###############################################################################
# Now we create three neurons: ``glif_psc``, ``glif_psc_double_alpha_timing``,
# and ``glif_psc_double_alpha_amp``. The parameters for the ``glif_psc`` neuron
# are set as default. The parameters for the ``glif_psc_double_alpha_timing``
# neuron are set to have the same ``tau_syn_fast`` as the ``glif_psc`` neuron,
# and the ``tau_syn_slow`` and ``amp_slow`` are set to the values defined above
# for the timing manipulation.

n_glif_psc = nest.Create(
    "glif_psc",
    params={
        "spike_dependent_threshold": False,
        "after_spike_currents": False,
        "adapting_threshold": False,
        "tau_syn": tau_syn_glif_psc,
    },
)

n_glif_psc_double_alpha_timing = nest.Create(
    "glif_psc_double_alpha",
    params={
        "spike_dependent_threshold": False,
        "after_spike_currents": False,
        "adapting_threshold": False,
        "tau_syn_fast": tau_syn_fast,
        "tau_syn_slow": tau_syn_slow_timing,
        "amp_slow": amp_slow_timing,
    },
)

n_glif_psc_double_alpha_amp = nest.Create(
    "glif_psc_double_alpha",
    params={
        "spike_dependent_threshold": False,
        "after_spike_currents": False,
        "adapting_threshold": False,
        "tau_syn_fast": tau_syn_fast,
        "tau_syn_slow": tau_syn_slow_amp,
        "amp_slow": amp_slow_amp,
    },
)

neurons = n_glif_psc + n_glif_psc_double_alpha_timing + n_glif_psc_double_alpha_amp

###############################################################################
# For the stimulation input to the ``glif_psc`` neurons, we create three excitation
# spike generators, each one with a single spike.

espike1 = nest.Create("spike_generator", params={"spike_times": [10.0], "spike_weights": [20.0]})
espike2 = nest.Create("spike_generator", params={"spike_times": [110.0], "spike_weights": [20.0]})
espike3 = nest.Create("spike_generator", params={"spike_times": [210.0], "spike_weights": [20.0]})

###############################################################################
# The generators are then connected to the neurons. Specification of
# the ``receptor_type`` uniquely defines the target receptor.
# We connect each of the spikes generator to a different receptor that have different
# parameters.

nest.Connect(espike1, neurons, syn_spec={"delay": resolution, "receptor_type": 1})
nest.Connect(espike2, neurons, syn_spec={"delay": resolution, "receptor_type": 2})
nest.Connect(espike3, neurons, syn_spec={"delay": resolution, "receptor_type": 3})

###############################################################################
# A ``multimeter`` is created and connected to the neurons. The parameters
# specified for the multimeter include the list of quantities that should be
# recorded and the time interval at which quantities are measured.

mm = nest.Create(
    "multimeter",
    params={
        "interval": resolution,
        "record_from": ["V_m", "I_syn"],
    },
)
nest.Connect(mm, neurons)

###############################################################################
# Run the simulation for 300 ms and retrieve recorded data from
# the multimeter and spike recorder.

nest.Simulate(300.0)
data = mm.events

###############################################################################
# We plot the time traces of the synaptic current and the  membrane potential.
# Each input current is annotated with the corresponding parameter value of the
# receptor. The blue line is the synaptic current of the ``glif_psc`` neuron, and
# the red line is the synaptic current of the ``glif_psc_double_alpha`` neuron.


# defining the figure property for each parameter variation type,
variation_types = ["timing", "amp"]
annotate_variable = ["tau_syn_slow", "amp_slow"]
annotate_values = [tau_syn_slow_timing, amp_slow_amp]
fig_titles = [
    "Variation of tau_syn_slow: tau_syn_fast = 2.0, amp_slow = 0.5",
    "Variation of amp_slow: tau_syn_fast = 2.0, tau_syn_slow = 6.0",
]


senders = data["senders"]
t = data["times"][senders == 1]

for i, variation_type in enumerate(variation_types):
    plt.figure(variation_type, figsize=(10, 5))
    gs = gridspec.GridSpec(2, 1, height_ratios=[1, 1])
    data_types = ["I_syn", "V_m"]
    data_types_names = ["Synaptic current (pA)", "Membrane potential (mV)"]
    for j, data_type in enumerate(data_types):
        d = data[data_type]
        ax = plt.subplot(gs[j])
        ax.plot(t, d[senders == 1], "b", label="glif_psc (tau_syn=2.0)")
        ax.plot(t, d[senders == 2 + i], "r", label="glif_psc_double_alpha")
        if j == 0:
            # place legend outside the plot
            ax.legend(bbox_to_anchor=(1.05, 1), loc="upper left", borderaxespad=0)
        else:
            ax.set_xlabel("time (ms)")

        ax.set_ylabel(data_types_names[j])

    # now let's annotate each of the input with the corresponding parameter.
    spike_timings = [10.0, 110.0, 210.0]
    ax = plt.subplot(gs[0])
    for j, spike_timing in enumerate(spike_timings):
        ax.annotate(
            f"{annotate_variable[i]}={annotate_values[i][j]}",
            xy=(spike_timing + 10, 20),
            xytext=(spike_timing + 10, 25),
            arrowprops=dict(arrowstyle="->", connectionstyle="arc3"),
        )
    plt.title(fig_titles[i])
    plt.tight_layout()


plt.show()
