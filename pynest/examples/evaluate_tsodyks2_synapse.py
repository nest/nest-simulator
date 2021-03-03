# -*- coding: utf-8 -*-
#
# evaluate_tsodyks2_synapse.py
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

"""Example of the tsodyks2_synapse in NEST
---------------------------------------------

This synapse model implements synaptic short-term depression and short-term f
according to [1]_ and [2]_. It solves Eq (2) from [1]_ and modulates U according

This connection merely scales the synaptic weight, based on the spike history
parameters of the kinetic model. Thus, it is suitable for any type of synapse
that is current or conductance based.

The parameter `A_se` from the publications is represented by the
synaptic weight. The variable `x` in the synapse properties is the
factor that scales the synaptic weight.

Parameters
~~~~~~~~~~~

The following parameters can be set in the status dictionary:

* U           - probability of release increment (U1) [0,1], default=0.
* u           - Maximum probability of release (U_se) [0,1], default=0.
* x           - current scaling factor of the weight, default=U
* tau_rec     - time constant for depression in ms, default=800 ms
* tau_fac     - time constant for facilitation in ms, default=0 (off)

Notes
~~~~~~~

Under identical conditions, the ``tsodyks2_synapse`` produces slightly lower
peak amplitudes than the ``tsodyks_synapse``. However, the qualitative behavior
is identical.

This compares the two synapse models.

References
~~~~~~~~~~~

.. [1] Tsodyks MV, and Markram H. (1997). The neural code between
       neocortical depends on neurotransmitter release probability. PNAS,
       94(2), 719-23.
.. [2] Fuhrmann G, Segev I, Markram H, and Tsodyks MV. (2002). Coding of
       temporal information by activity-dependent synapses. Journal of
       Neurophysiology, 8. https://doi.org/10.1152/jn.00258.2001
.. [3] Maass W, and Markram H. (2002). Synapses as dynamic memory buffers.
       Neural Networks, 15(2), 155-161.
       http://dx.doi.org/10.1016/S0893-6080(01)00144-7
"""

import nest
import nest.voltage_trace

nest.ResetKernel()

###############################################################################
# Parameter set for depression

dep_params = {"U": 0.67, "u": 0.67, 'x': 1.0, "tau_rec": 450.0,
              "tau_fac": 0.0, "weight": 250.}

###############################################################################
# Parameter set for facilitation

fac_params = {"U": 0.1, "u": 0.1, 'x': 1.0, "tau_fac": 1000.,
              "tau_rec": 100., "weight": 250.}

###############################################################################
# Now we assign the parameter set to the synapse models.

t1_params = fac_params       # for tsodyks_synapse
t2_params = t1_params.copy()  # for tsodyks2_synapse

nest.SetDefaults("tsodyks2_synapse", t1_params)
nest.SetDefaults("tsodyks_synapse", t2_params)
nest.SetDefaults("iaf_psc_exp", {"tau_syn_ex": 3.})

###############################################################################
# Create three neurons.

neuron = nest.Create("iaf_psc_exp", 3)

###############################################################################
# Neuron one produces spikes. Neurons 2 and 3 receive the spikes via the two
# synapse models.

nest.Connect(neuron[0], neuron[1], syn_spec="tsodyks_synapse")
nest.Connect(neuron[0], neuron[2], syn_spec="tsodyks2_synapse")

###############################################################################
# Now create two voltmeters to record the responses.

voltmeter = nest.Create("voltmeter", 2)

###############################################################################
# Connect the voltmeters to the neurons.

nest.Connect(voltmeter[0], neuron[1])
nest.Connect(voltmeter[1], neuron[2])

###############################################################################
# Now simulate the standard STP protocol: a burst of spikes, followed by a
# pause and a recovery response.

neuron[0].I_e = 376.0

nest.Simulate(500.0)
neuron[0].I_e = 0.0
nest.Simulate(500.0)
neuron[0].I_e = 376.0
nest.Simulate(500.0)

###############################################################################
# Finally, generate voltage traces. Both are shown in the same plot and
# should be almost completely overlapping.

nest.voltage_trace.from_device(voltmeter[0])
nest.voltage_trace.from_device(voltmeter[1])
nest.voltage_trace.show()
