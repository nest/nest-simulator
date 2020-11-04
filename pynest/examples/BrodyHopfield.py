# -*- coding: utf-8 -*-
#
# BrodyHopfield.py
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


"""Spike synchronization through subthreshold oscillation
------------------------------------------------------------

This script reproduces the spike synchronization behavior
of integrate-and-fire neurons in response to a subthreshold
oscillation. This phenomenon is shown in Fig. 1 of [1]_

Neurons receive a weak 35 Hz oscillation, a gaussian noise current
and an increasing DC. The time-locking capability is shown to
depend on the input current given. The result is then plotted using
matplotlib. All parameters are taken from the above paper.

References
~~~~~~~~~~~~~

.. [1] Brody CD and Hopfield JJ (2003). Simple networks for
       spike-timing-based computation, with application to olfactory
       processing. Neuron 37, 843-852.
"""

#################################################################################
# First, we import all necessary modules for simulation, analysis, and plotting.

import nest
import nest.raster_plot
import matplotlib.pyplot as plt

###############################################################################
# Second, the simulation parameters are assigned to variables.

N = 1000           # number of neurons
bias_begin = 140.  # minimal value for the bias current injection [pA]
bias_end = 200.    # maximal value for the bias current injection [pA]
T = 600            # simulation time (ms)

# parameters for the alternating-current generator
driveparams = {'amplitude': 50., 'frequency': 35.}
# parameters for the noise generator
noiseparams = {'mean': 0.0, 'std': 200.}
neuronparams = {'tau_m': 20.,  # membrane time constant
                'V_th': 20.,  # threshold potential
                'E_L': 10.,  # membrane resting potential
                't_ref': 2.,  # refractory period
                'V_reset': 0.,  # reset potential
                'C_m': 200.,  # membrane capacitance
                'V_m': 0.}      # initial membrane potential

###############################################################################
# Third, the nodes are created using ``Create``. We store the returned handles
# in variables for later reference.

neurons = nest.Create('iaf_psc_alpha', N)
sr = nest.Create('spike_recorder')
noise = nest.Create('noise_generator')
drive = nest.Create('ac_generator')

###############################################################################
# Set the parameters specified above for the generators using ``set``.

drive.set(driveparams)
noise.set(noiseparams)

###############################################################################
# Set the parameters specified above for the neurons. Neurons get an internal
# current. The first neuron additionally receives the current with amplitude
# `bias_begin`, the last neuron with amplitude `bias_end`.

neurons.set(neuronparams)
neurons.I_e = [(n * (bias_end - bias_begin) / N + bias_begin)
               for n in range(1, len(neurons) + 1)]

###############################################################################
# Connect alternating current and noise generators as well as
# `spike_recorder`s to neurons

nest.Connect(drive, neurons)
nest.Connect(noise, neurons)
nest.Connect(neurons, sr)

###############################################################################
# Simulate the network for time `T`.

nest.Simulate(T)

###############################################################################
# Plot the raster plot of the neuronal spiking activity.

nest.raster_plot.from_device(sr, hist=True)
plt.show()
