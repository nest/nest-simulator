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

# This script reproduces the spike synchronization
# behavior of integrate-and-fire neurons in response to a subthreshold
# oscillation. This phenomenon is shown in Fig. 1 of

#   C.D. Brody and J.J. Hopfield
#   Simple Networks for Spike-Timing-Based Computation,
#   with Application to Olfactory Processing
#   Neuron 37, 843-852 (2003)

# Neurons receive a weak 35Hz oscillation, a gaussian noise current
# and an increasing DC. The time-locking capability is shown to
# depend on the input current given. The result is then plotted using pylab.
# All parameters are taken from the above paper.
#
# units are the usual NEST units: pA,pF,ms,mV,Hz
#
# Sven Schrader

import nest
import nest.raster_plot
import pylab
import numpy

N=1000 # number of neurons

bias_begin=140. # bias current from...
bias_end=200.   # ...to (ms)
T=600 # simulation time (ms)

def bias(n):
    # constructs the dictionary with current ramp
    return { 'I_e': (n * (bias_end-bias_begin)/N + bias_begin) }


driveparams  = {'amplitude':50., 'frequency':35.}
noiseparams  = {'mean':0.0, 'std':200.}
neuronparams = { 'tau_m':20., 'V_th':20., 'E_L':10.,
                 't_ref':2., 'V_reset':0., 'C_m':200., 'V_m':0.}

neurons = nest.Create('iaf_psc_alpha',N)
sd      = nest.Create('spike_detector')
noise   = nest.Create('noise_generator')
drive   = nest.Create('ac_generator')

nest.SetStatus(drive,   driveparams )
nest.SetStatus(noise,   noiseparams )
nest.SetStatus(neurons, neuronparams)
nest.SetStatus(neurons, map(bias, neurons))

nest.SetStatus(sd, {"withgid": True, "withtime": True})

nest.DivergentConnect(drive, neurons)
nest.DivergentConnect(noise, neurons)
nest.ConvergentConnect(neurons, sd)

nest.Simulate(T)

nest.raster_plot.from_device(sd, hist=True)
