# -*- coding: utf-8 -*-
#
# stdp_check.py
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

from matplotlib.pylab import *

# Test script to reproduce changes in weight of a STDP synapse in an event-driven way.
# Pre- and post-synaptic spike trains are read in from spike_detector-0-0-3.gdf
# (output of test_stdp_poiss.sli).
# output: pre/post \t spike time \t weight
# 
# Synaptic dynamics for STDP synapses according to Abigail Morrison's
# STDP model (see stdp_rec.pdf).
# 
# first version: Moritz Helias, april 2006
# adapted to python MH, SK, May 2008

def stdp(w_init, w_max, pre_spikes, post_spikes, alpha, mu_plus, mu_minus, lmbd, tau_plus, tau_minus, delay):
    w = w_init # initial weight
    i = 0      # index of next presynaptic spike
    j = 0      # index of next postsynaptic spike

    K_plus = 0.
    K_minus = 0.
    last_t = 0.

    advance = True

    while advance:

        advance = False
        
        # next spike is presynaptic   
        if pre_spikes[i] < post_spikes[j]:
            dt = pre_spikes[i] - last_t
            
            # evolve exponential filters
            K_plus *= exp(-dt/tau_plus)            
            K_minus *= exp(-dt/tau_minus)

            # depression
            w = w/w_max - lmbd * alpha * (w/w_max)**mu_minus * K_minus
            if w > 0.:
                w *= w_max
            else:
                w = 0.

            print "pre\t%.16f\t%.16f" % (pre_spikes[i],w)
        
            K_plus += 1.
            last_t = pre_spikes[i] # time evolved until here
            if i < len(pre_spikes) - 1:
                i += 1
                advance = True

        # same timing of next pre- and postsynaptic spike
        elif pre_spikes[i] == post_spikes[j]:
            dt = pre_spikes[i] - last_t
            
            # evolve exponential filters
            K_plus *= exp(-dt/tau_plus)
            K_minus *= exp(-dt/tau_minus)

            # facilitation
            w = w/w_max + lmbd * (1.-w/w_max)**mu_plus * K_plus
            if w < 1.:
                w *= w_max
            else:
                w = w_max            
            print "post\t%.16f\t%.16f" % (post_spikes[j]-delay,w)

            # depression
            w = w/w_max - lmbd * alpha * (w/w_max)**mu_minus * K_minus
            if w > 0.:
                w *= w_max
            else:
                w = 0.
            print "pre\t%.16f\t%.16f" % (pre_spikes[i],w)
            
            K_plus += 1.
            K_minus += 1.
            last_t = pre_spikes[i] # time evolved until here
            if i < len(pre_spikes) - 1:
                i += 1
                advance = True              
            if j < len(post_spikes) - 1:
                j += 1
                advance = True

        # next spike is postsynaptic
        else:
            dt = post_spikes[j] - last_t
            
            # evolve exponential filters
            K_plus *= exp(-dt / tau_plus)
            K_minus *= exp(-dt / tau_minus)

            # facilitation
            w = w/w_max + lmbd * (1.-w/w_max)**mu_plus * K_plus
            if w < 1.:
                w *= w_max
            else:
                w = w_max

            print "post\t%.16f\t%.16f" % (post_spikes[j]-delay,w)
            
            K_minus += 1.
            last_t = post_spikes[j] # time evolved until here
            if j < len(post_spikes) - 1:
                j += 1
                advance = True
    
    return w

# stdp parameters
w_init    = 35.
w_max     = 70.
alpha     =   .95
mu_plus   =   .05
mu_minus  =   .05
lmbd      =   .025
tau_plus  = 20.
tau_minus = 20.

# dendritic delay
delay     =  1.

# load spikes from simulation with test_stdp_poiss.sli
spikes = load("spike_detector-0-0-3.gdf")

pre_spikes = spikes[find(spikes[:,0] == 5), 1]

# delay is purely dendritic
# postsynaptic spike arrives at sp_j + delay at the synapse
post_spikes = spikes[find(spikes[:,0] == 6), 1] + delay

# calculate development of stdp weight
stdp(w_init, w_max, pre_spikes, post_spikes, alpha, mu_plus, mu_minus, lmbd, tau_plus, tau_minus, delay)
