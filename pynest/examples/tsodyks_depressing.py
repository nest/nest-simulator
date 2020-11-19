# -*- coding: utf-8 -*-
#
# tsodyks_depressing.py
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

"""Tsodyks depressing example
--------------------------------

This scripts simulates two neurons. One is driven with dc-input and
connected to the other one with a depressing Tsodyks synapse. The membrane
potential trace of the second neuron is recorded.

This example reproduces Figure 1A of [1]_.
This example is analog to ``tsodyks_facilitating.py``, except that different
synapse parameters are used. Here, a large facilitation parameter ``U``
causes a fast saturation of the synaptic efficacy (Eq. 2.2), disabling a
facilitating behavior.

References
~~~~~~~~~~~~

.. [1] Tsodyks M, Pawelzik K, Markram H (1998). Neural networks with dynamic synapses. Neural
       computation, http://dx.doi.org/10.1162/089976698300017502

See Also
~~~~~~~~~~

:doc:`tsodyks_facilitating`

"""

###############################################################################
# First, we import all necessary modules for simulation and plotting.

import nest
import nest.voltage_trace
from numpy import exp

###############################################################################
# Second, the simulation parameters are assigned to variables. The neuron
# and synapse parameters are stored into a dictionary.

h = 0.1    # simulation step size (ms)
Tau = 40.    # membrane time constant
Theta = 15.    # threshold
E_L = 0.     # reset potential of membrane potential
R = 0.1    # 100 M Ohm
C = Tau / R  # Tau (ms)/R in NEST units
TauR = 2.     # refractory time
Tau_psc = 3.     # time constant of PSC (= Tau_inact)
Tau_rec = 800.   # recovery time
Tau_fac = 0.     # facilitation time
U = 0.5    # facilitation parameter U
A = 250.   # PSC weight in pA
f = 20. / 1000.  # frequency in Hz converted to 1/ms
Tend = 1200.  # simulation time
TIstart = 50.    # start time of dc
TIend = 1050.  # end time of dc
I0 = Theta * C / Tau / (1 - exp(-(1 / f - TauR) / Tau))  # dc amplitude

neuron_param = {"tau_m": Tau,
                "t_ref": TauR,
                "tau_syn_ex": Tau_psc,
                "tau_syn_in": Tau_psc,
                "C_m": C,
                "V_reset": E_L,
                "E_L": E_L,
                "V_m": E_L,
                "V_th": Theta}

syn_param = {"tau_psc": Tau_psc,
             "tau_rec": Tau_rec,
             "tau_fac": Tau_fac,
             "U": U,
             "delay": 0.1,
             "weight": A,
             "u": 0.0,
             "x": 1.0}

###############################################################################
# Third, we reset the kernel and set the resolution using ``SetKernelStatus``.

nest.ResetKernel()
nest.SetKernelStatus({"resolution": h})

###############################################################################
# Fourth, the nodes are created using ``Create``. We store the returned
# handles in variables for later reference.

neurons = nest.Create("iaf_psc_exp", 2)
dc_gen = nest.Create("dc_generator")
volts = nest.Create("voltmeter")

################################################################################
# Fifth, the ``iaf_psc_exp`` neurons, the ``dc_generator`` and the ``voltmeter``
# are configured using ``SetStatus``, which expects a list of node handles and
# a parameter dictionary or a list of parameter dictionaries.

neurons.set(neuron_param)
dc_gen.set(amplitude=I0, start=TIstart, stop=TIend)
volts.set(label="voltmeter", interval=1.)

###############################################################################
# Sixth, the ``dc_generator`` is connected to the first neuron
# (`neurons[0]`) and the ``voltmeter`` is connected to the second neuron
# (`neurons[1]`). The command ``Connect`` has different variants. Plain
# ``Connect`` just takes the handles of pre- and postsynaptic nodes and uses
# the default values for weight and delay. Note that the connection
# direction for the ``voltmeter`` reflects the signal flow in the simulation
# kernel, because it observes the neuron instead of receiving events from it.

nest.Connect(dc_gen, neurons[0])
nest.Connect(volts, neurons[1])

###############################################################################
# Seventh, the first neuron (`neurons[0]`) is connected to the second
# neuron (`neurons[1]`).  The command ``CopyModel`` copies the
# ``tsodyks_synapse`` model to the new name ``syn`` with parameters
# ``syn_param``.  The manually defined model ``syn`` is used in the
# connection routine via the ``syn_spec`` parameter.

nest.CopyModel("tsodyks_synapse", "syn", syn_param)
nest.Connect(neurons[0], neurons[1], syn_spec="syn")

###############################################################################
# Finally, we simulate the configuration using the command ``Simulate``,
# where the simulation time `Tend` is passed as the argument.  We plot the
# target neuron's membrane potential as a function of time.

nest.Simulate(Tend)
nest.voltage_trace.from_device(volts)
nest.voltage_trace.show()
