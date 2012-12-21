# -*- coding: utf-8 -*-
#
# CampbellSiegert.py
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

# CampbellSiegert.py
# 
# Example script that applies Campbell's theorem and Siegert's rate approximation.
# 
# This script calculates the firing rate of an integrate-and-fire neuron
# in response to a series of Poisson generators, each specified with
# a rate and a synaptic weight.
# The calculated rate is compared with a simulation using the iaf_psc_alpha model
# 
#
#

# Sven Schrader, Nov 2008, Siegert implementation by Tom Tetzlaff

from scipy.special import erf
from scipy.optimize import fmin

import numpy
from numpy import sqrt, exp

import pylab

import nest

# example 1
weights    = [0.1]   # mV psp amplitudes
rates      = [8000.] # Hz

# example 2, should have same result as example 1
#weights    = [0.1, 0.1]
#rates      = [4000., 4000.]

Cm         = 250.   # pF, capacitance
tau_syn_ex = 0.5    # ms, synaptic time constants
tau_syn_in = 2.0    # 
tau_m      = 20.    # ms, membrane time constant
tref       = 2.0    # ms, refractory period
V0         = 0.0    # mV, resting potential
Vth        = 20.0   # mV, firing threshold


simtime    = 20000 # ms
n_neurons  = 10    # number of simulated neurons

pi = numpy.pi
e  = exp(1)

pF = 1e-12
ms = 1e-3
pA = 1e-12
mV = 1e-3


mu     = 0.0
sigma2 = 0.0
J      = []

assert(len(weights) == len(rates))

########################################################################################
# Analytical section
for rate, weight in zip(rates, weights):

    if weight >0:
        tau_s = tau_syn_ex
    else:
        tau_s = tau_syn_in
        
    t_psp   = numpy.arange(0, 10 * (tau_m*ms + tau_s*ms),0.0001 )

    # calculation of a single PSP
    psp = lambda x:  -(Cm*pF) / (tau_s*ms) * (1/(Cm*pF)) * (e/(tau_s*ms)) * \
         (((-x * exp(-x/(tau_s*ms))) / (1/(tau_s*ms )-1 / (tau_m*ms))) +\
          (exp(-x/(tau_m*ms)) - exp(-x/(tau_s*ms))) / ((1/(tau_s*ms) - 1/(tau_m*ms))**2)  )

    min_result = fmin(psp, [0], full_output=1, disp=0)

    fudge = -1./min_result[1] # fudge is used here to scale psC amplitude from psP amplitude
    J.append( Cm*weight/tau_s*fudge) # <-------|

    # Campbell's Theorem 
    # the mean membrane potential mu and variance sigma adds up for each Poisson source
    mu    += ((V0*mV) + rate * \
             (J[-1]*pA) * (tau_s*ms) * e * (tau_m*ms) / (Cm*pF))

    sigma2 += rate * \
            (2* tau_m*ms + tau_s*ms ) * \
            (J[-1]*pA * tau_s*ms *e * tau_m*ms/ ( 2 * (Cm*pF) * (tau_m*ms + tau_s*ms) ) ) ** 2

sigma = sqrt(sigma2)


# Siegert's rate approximation
num_iterations = 100                                        
ul = (Vth*mV - mu) / (sigma)/sqrt(2)                        
ll = (V0*mV  - mu) / (sigma)/sqrt(2)                        
interval = (ul-ll)/num_iterations                           
tmpsum = 0.0                                                
for cu in range(0,num_iterations+1):                        
  u = ll + cu * interval                                    
  f = exp(u**2)*(1+erf(u))                                  
  tmpsum += interval * sqrt(pi) * f                         
                                                            
r =  1. / (tref*ms + tau_m*ms * tmpsum)                       


########################################################################################
# Simulation section
nest.ResetKernel()

nest.sr('20 setverbosity')
neurondict = {'V_th':Vth, 'tau_m':tau_m, 'tau_syn_ex':tau_syn_ex,'tau_syn_in':tau_syn_in,  'C_m':Cm, 'E_L':V0, 't_ref':tref, 'V_m': V0, 'V_reset': V0}

if (mu*1000) < Vth:
    neurondict['V_m'] = mu*1000.
    
nest.SetDefaults('iaf_psc_alpha', neurondict)
n      = nest.Create('iaf_psc_alpha', n_neurons)
n_free = nest.Create('iaf_psc_alpha', 1 ,[{'V_th':999999.}]) # high threshold as we want free membrane potential
pg     = nest.Create('poisson_generator', len(rates), [ {'rate':float(rate_i)} for rate_i in rates]   )
vm     = nest.Create('voltmeter', 1,     [{'record_to':['memory'], 'withtime':True, 'withgid':True, 'interval':.1}])
sd     = nest.Create('spike_detector',1, [{'record_to':['memory'], 'withtime':True, 'withgid':True}])

for i, currentpg in enumerate(pg):
    nest.DivergentConnect([currentpg],n,weight=float(J[i]), delay=0.1)
    nest.Connect([currentpg],n_free,      {'weight':J[i]})

nest.Connect(vm,n_free)
nest.ConvergentConnect(n,sd)

nest.Simulate(simtime)

# free membrane potential (first 100 steps are omitted)
v_free = nest.GetStatus(vm,'events')[0]['V_m'][100:-1]

print 'mean membrane potential  (actual/calculated):', numpy.mean(v_free), mu*1000
print 'variance  (actual/calculated):               ', numpy.var(v_free), sigma2*1e6
print 'firing rate (actual/calculated):             ', nest.GetStatus(sd,'n_events')[0] / (n_neurons*simtime*ms), r
