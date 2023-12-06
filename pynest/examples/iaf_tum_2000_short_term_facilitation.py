# -*- coding: utf-8 -*-
#
# iaf_tum_2000_short_term_facilitation.py
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

r"""
Short-term facilitation example
-------------------------------

The :doc:`iaf_tum_2000 </models/iaf_tum_2000>` neuron [1]_ is a model with
*short-term synaptic plasticity*. Short-term plasticity can either strengthen
or weaken a synapse and acts on a timescale of milliseconds to seconds. This
example illustrates *short-term facilitation*, which is a transient increase
in synaptic strength caused by influx of calcium into the axon terminal after
spike generation. Facilitation results in an increase of neurotransmitters
being released by each succeeding presynaptic spike.

In the ``iaf_tum_2000`` model, a fraction :math:`u` of the available synaptic
resources :math:`x` is used by each presynaptic spike (see Eq. 3 and 4 or
Eq. 2.1 and 2.2 in [1]_ or [2]_, respectively). A parameter :math:`U \in [0, 1]`
determines the increase in :math:`u` with each spike. In a facilitating synapse,
:math:`u` is increased with each presynaptic spike, which reflects the
accumulation of calcium ions caused by spikes arriving in the presynaptic terminal,
and returns to the baseline value with a time constant :math:`\tau_\mathrm{fac}`.

In this example, we reproduce Figure 1B in [2]_. We connect two
``iaf_tum_2000`` neurons. The presynaptic neuron is driven by DC input and
we record the voltage trace of the postsynaptic neuron. Short-term facilitation
is enabled by setting :math:`U` to a small value, which causes a slow saturation
of the synaptic efficacy, enabling a facilitating behavior.

For an example on *short-term depression*, see
:doc:`../auto_examples/iaf_tum_2000_short_term_depression`.

.. note::

    The :doc:`iaf_tum_2000 </models/iaf_tum_2000>` neuron model combined with
    :doc:`static_synapse </models/static_synapse>` provides a more efficient
    implementation of the model studied in [1]_ and [2]_ than the combination
    of :doc:`iaf_psc_exp </models/iaf_psc_exp>` with
    :doc:`tsodyks_synapse </models/tsodyks_synapse>`.

References
~~~~~~~~~~

.. [1] Tsodyks M, Uziel A, Markram H (2000). Synchrony generation in recurrent
       networks with frequency-dependent synapses. The Journal of Neuroscience,
       20,RC50:1-5. URL: https://infoscience.epfl.ch/record/183402

.. [2] Tsodyks M, Pawelzik K, Markram H (1998). Neural networks with dynamic synapses. Neural
       computation, http://dx.doi.org/10.1162/089976698300017502

See Also
~~~~~~~~

:doc:`/models/iaf_tum_2000`, :doc:`iaf_tum_2000_short_term_depression`
"""

import matplotlib.pyplot as plt
import nest
import numpy as np

###############################################################################
# First we make sure that the NEST kernel is reset and the resolution of the
# simulation is 0.1 ms. We also define the simulation time.

nest.ResetKernel()
nest.resolution = 0.1  # simulation step size [ms]

T_sim = 1200.0  # simulation time [ms]

###############################################################################
# We set the neuronal membrane parameters according to [2]_.

tau_m = 60.0  # membrane time constant [ms]
R_m = 1.0  # membrane input resistance [GΩ]
C_m = tau_m / R_m  # membrane capacitance [pF]
V_th = 15.0  # threshold potential [mV]
V_reset = 0.0  # reset potential [mV]
t_ref = 2.0  # refractory period [ms]

###############################################################################
# We create one current generator and configure a stimulus that will drive
# the presynaptic neuron. Configuration of the current generator includes the
# definition of the start and stop times and the amplitude of the injected
# current.

stim_start = 50.0  # start time of DC input [ms]
stim_end = 1050.0  # end time of DC input [ms]
f = 20.0 / 1000.0  # frequency used in [2] [mHz]
dc_amp = V_th * C_m / tau_m / (1 - np.exp(-(1 / f - t_ref) / tau_m))  # DC amplitude [pA]

dc_gen = nest.Create(
    "dc_generator",
    1,
    params={"amplitude": dc_amp, "start": stim_start, "stop": stim_end},
)

###############################################################################
# Next, we set the synaptic parameters according to [2]_.

x = 1.0  # initial fraction of synaptic vesicles in the readily releasable pool
u = 0.0  # initial release probability of synaptic vesicles
U = 0.03  # fraction determining the increase in u with each spike
tau_psc = 1.5  # decay constant of PSCs (tau_inact in [2]) [ms]
tau_rec = 130.0  # recovery time from synaptic depression [ms]
tau_fac = 530.0  # time constant for facilitation [ms]

###############################################################################
# We create two ``iaf_tum_2000`` neurons. Since this model integrates the
# synaptic dynamics in the presynaptic neuron, the synaptic parameters, except
# for ``weight`` and ``delay``, are provided together with the neuron parameters
# to the model.

nrns = nest.Create(
    "iaf_tum_2000",
    2,
    params={
        "C_m": C_m,
        "tau_m": tau_m,
        "tau_syn_ex": tau_psc,
        "tau_syn_in": tau_psc,
        "V_th": V_th,
        "V_reset": V_reset,
        "E_L": V_reset,
        "V_m": V_reset,
        "t_ref": t_ref,
        "U": U,
        "tau_psc": tau_psc,
        "tau_rec": tau_rec,
        "tau_fac": tau_fac,
        "x": x,
        "u": u,
    },
)

###############################################################################
# We connect the DC generator to the presynaptic neuron.
nest.Connect(dc_gen, nrns[0])

###############################################################################
# We then connect the pre- and postsynaptic neurons. We use a ``static_synapse``
# to transfer the synaptic current computed in the presynaptic neuron. The
# synaptic weight and delay are passed with the static synapse's ``syn_spec``.
# Note that ``iaf_tum_2000`` neurons must be connected via ``receptor_type`` 1.

weight = 1540.0  # synaptic weight [pA]
delay = 0.1  # synaptic delay [ms]

nest.Connect(
    nrns[0],
    nrns[1],
    syn_spec={
        "synapse_model": "static_synapse",
        "weight": weight,
        "delay": delay,
        "receptor_type": 1,
    },
)

###############################################################################
# We add a ``voltmeter`` to sample the membrane potentials from the postsynaptic
# neuron in intervals of 1.0 ms. Note that the connection direction for the
# ``voltmeter`` reflects the signal flow in the simulation kernel; a ``voltmeter``
# observes the neuron instead of receiving events from it.

voltmeter = nest.Create("voltmeter", params={"interval": 1.0, "label": "Voltmeter"})
nest.Connect(voltmeter, nrns[1])

###############################################################################
# Finally, we simulate the system with simulation time ``T_sim`` and plot a
# voltage trace to produce the figure.

nest.Simulate(T_sim)
nest.voltage_trace.from_device(voltmeter)
plt.show()
