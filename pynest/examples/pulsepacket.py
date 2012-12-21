# -*- coding: utf-8 -*-
#
# pulsepacket.py
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

# This script compares the average and individual membrane potential excursions
# in response to a single pulse packet with an analytically acquired voltage trace.
# A pulse packet is a transient spike volley with a Gaussian rate profile.
#
#
# The user can specify the neural parameters, the parameters of the
# pulse-packet and the number of trials.

# Sven Schrader, Nov 2008

import nest
import numpy
import pylab
import array

from numpy import exp

a         = 100   # number of spikes in one pulse packet
sdev      = 50.   # ms width of pulse packet
weight    = 0.1   # mV psp amplitude

n_neurons = 10    # number of trials

pulsetime = 500.  # ms occurrence time (center) of pulse-packet
simtime   = 1000. # ms total simulation duration

Cm        = 200.   # pF, capacitance
tau_s     = 0.5    # ms, synaptic time constant
tau_m     = 20.    # ms, membrane time constant
V0        = 0.0    # mV, resting potential
Vth       = 9999.0 # mV, firing threshold. Keep high when looking at passive properties.

simulation_resolution  = 0.1 # ms
sampling_resolution    = 1.  # ms, for voltmeter
convolution_resolution = 1.  # ms, for the analytics



#####################################################################
# analytical section. Here we need to convert to SI units

e  = exp(1)
pF = 1e-12
ms = 1e-3 # helper variables as units

# make gauss-kernel
sigma = (sdev*ms)**2
mu    = 0.0
x     = numpy.arange(-4*sdev , 4*sdev , convolution_resolution * ms )
gauss = 1 / (sigma * numpy.sqrt(2) * numpy.pi) * exp( -(x-mu)**2 / (sigma*numpy.sqrt(2) ) )
gauss /= pylab.sum(gauss) # normalize to unit area

# make unit psp
#
# step 1: make time axis. We use the tenfold sum of tau_s and tau_m as width. This should suffice.
t_psp   = numpy.arange(0, 10 * (tau_m * ms + tau_s * ms) , convolution_resolution * ms )

# step 2 : calculate psp. Its maximum is used below as a fudge-factor for the psp amplitude
psp =  ( Cm * pF ) / ( tau_s * ms ) * (1/( Cm * pF )) * (e/( tau_s * ms ) ) * \
     (  ((-t_psp * exp(-t_psp/(tau_s * ms) )) / (1/( tau_s * ms )-1 / ( tau_m * ms ) )) +\
        (exp(-t_psp/( tau_m * ms )) - exp(-t_psp/( tau_s * ms ))) / ((1/( tau_s * ms ) - 1/( tau_m * ms ))**2)  )

# step 3: normalize to amplitude 1, thereby obtaining the maximum
fudge = 1/numpy.max(psp) # fudge is also used below
psp *= fudge

# step 4: pad with zeros on the left side. Avoids offsets when using convolution
tmp = numpy.zeros(2*len(psp))
tmp[len(psp)-1:-1] += psp
psp = tmp
del tmp

P = a * weight * pylab.convolve(gauss, psp)

l   = len(P)
t_P = convolution_resolution * numpy.linspace( -l/2., l/2., l) + pulsetime + 1. # one ms delay



#########################################################################
# simulation section

nest.ResetKernel()

nest.SetStatus([0],[{'resolution':simulation_resolution}])
J = Cm*weight/tau_s*fudge
nest.SetDefaults('static_synapse', {'weight':J} )

n  = nest.Create('iaf_psc_alpha',n_neurons,
                 {'V_th':Vth, 'tau_m':tau_m, 'tau_syn_ex':tau_s, 'C_m':Cm, 'E_L':V0,'V_reset':V0,'V_m':V0})
pp = nest.Create('pulsepacket_generator',n_neurons, {'pulse_times':[pulsetime], 'activity':a, 'sdev':sdev})
vm = nest.Create('voltmeter', 1, {'record_to':['memory'], 'withtime':True, 'withgid':True, 'interval':sampling_resolution})

nest.Connect(pp,n)
nest.DivergentConnect(vm,n)

nest.Simulate(simtime)

V       = nest.GetStatus(vm,'events')[0]['V_m']
t_V     = nest.GetStatus(vm,'events')[0]['times']
senders = nest.GetStatus(vm,'events')[0]['senders']


#########################################################################
# plotting...

v={}
t={}

for s in range(senders.size):
    currentsender=senders[s]
    if not v.has_key(currentsender) :
        v[currentsender] = array.array('f')
    v[currentsender].append(float(V[s]))

    if not t.has_key(currentsender) :
        t[currentsender] = array.array('f')
    t[currentsender].append(float(t_V[s]))

if n_neurons >1:
    average_V = numpy.zeros(len(v[n[0]]))
    for neuron in n:
        average_V += v[neuron]
    average_V /= n_neurons

pylab.hold(True)

p2 = pylab.plot(t_P,P+V0,color='red',linewidth=3)
p2[0].set_zorder(n_neurons+1)

if n_neurons > 1:
    p3 = pylab.plot(t[n[0]], average_V ,color='blue',linewidth=2)
    p3[0].set_zorder(n_neurons+2)

for neuron in n:
    pylab.plot(t[neuron],v[neuron],color='gray')

if n_neurons > 1:
    pylab.legend( ( 'analytical solution', 'averaged potential','membrane potential') )
else:
    pylab.legend( ( 'analytical solution','membrane potential') )

pylab.xlabel('ms')
pylab.ylabel('mV')
pylab.xlim((-10*(tau_m+tau_s) + pulsetime , 10*(tau_m+tau_s) + pulsetime))
