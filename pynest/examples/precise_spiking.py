# -*- coding: utf-8 -*-
#
# precise_spiking.py
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
Comparing precise and grid-based neuron models
----------------------------------------------

In traditional time-driven simulations, spikes are constrained to the
time grid at a user-defined resolution. The precise spiking models
overcome this by handling spikes in continuous time [1]_ and [2]_.

The precise spiking neuron models in NEST include: ``iaf_psc_exp_ps``,
``iaf_psc_alpha_ps`` and ``iaf_psc_delta_ps``.
More detailed information about the precise spiking models can be
found here:
https://www.nest-simulator.org/simulations-with-precise-spike-times/

This example compares the conventional grid-constrained model and the
precise version for an integrate-and-fire neuron model with exponential
postsynaptic currents [2]_.

References
~~~~~~~~~~

.. [1] Morrison A, Straube S, Plesser HE, Diesmann M. 2007. Exact subthreshold
       integration with continuous spike times in discrete-time neural network
       simulations. Neural Computation. 19(1):47-79.
       https://doi.org/10.1162/neco.2007.19.1.47

.. [2] Hanuschkin A, Kunkel S, Helias M, Morrison A and Diesmann M. 2010. A
       general and efficient method for incorporating precise spike times in
       globally time-driven simulations. Froniers in Neuroinformatics. 4:113.
       https://doi.org/10.3389/fninf.2010.00113

"""


###############################################################################
# First, we import all necessary modules for simulation, analysis, and
# plotting.


import nest
import matplotlib.pyplot as plt


###############################################################################
# Second, we assign the simulation parameters to variables.


simtime = 100.0           # ms
stim_current = 700.0           # pA
resolutions = [0.1, 0.5, 1.0]  # ms


###################################################################################
# Now, we simulate the two versions of the neuron models (i.e. discrete-time:
# ``iaf_psc_exp``; precise: ``iaf_psc_exp_ps``) for each of the defined
# resolutions. The neurons use their default parameters and we stimulate them
# by injecting a current using a ``dc_generator`` device. The membrane
# potential is recorded by a ``voltmeter``, the spikes are recorded by
# a ``spike_recorder``.  The data is stored in a dictionary for later
# use.


data = {}

for h in resolutions:
    data[h] = {}
    for model in ["iaf_psc_exp", "iaf_psc_exp_ps"]:
        nest.ResetKernel()
        nest.SetKernelStatus({'resolution': h})

        neuron = nest.Create(model)
        voltmeter = nest.Create("voltmeter", params={"interval": h})
        dc = nest.Create("dc_generator", params={"amplitude": stim_current})
        sr = nest.Create("spike_recorder")

        nest.Connect(voltmeter, neuron)
        nest.Connect(dc, neuron)
        nest.Connect(neuron, sr)

        nest.Simulate(simtime)

        vm_status = voltmeter.events
        sr_status = sr.events
        data[h][model] = {"vm_times": vm_status['times'],
                          "vm_values": vm_status['V_m'],
                          "spikes": sr_status['times'],
                          "V_th": neuron.V_th}


###############################################################################
# After simulation, we plot the results from the simulation. The figure
# illustrates the membrane potential excursion of the two models due to
# injected current simulated for 100 ms for a different timestep in each panel.
# The blue line is the voltage trace of the discrete-time neuron, the red line
# is that of the precise spiking version of the same model.
#
# Please note that the temporal differences between the traces in the different
# panels is caused by the different resolutions used.


colors = ["#3465a4", "#cc0000"]

for v, h in enumerate(sorted(data)):
    plot = plt.subplot(len(data), 1, v + 1)
    plot.set_title("Resolution: {0} ms".format(h))

    for i, model in enumerate(data[h]):
        times = data[h][model]["vm_times"]
        potentials = data[h][model]["vm_values"]
        spikes = data[h][model]["spikes"]
        spikes_y = [data[h][model]["V_th"]] * len(spikes)

        plot.plot(times, potentials, "-", c=colors[i], ms=5, lw=2, label=model)
        plot.plot(spikes, spikes_y, ".", c=colors[i], ms=5, lw=2)

    if v == 2:
        plot.legend(loc=4)
    else:
        plot.set_xticklabels('')

plt.show()
