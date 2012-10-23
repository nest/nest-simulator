#! /usr/bin/env python
#
# stdp_dopa_check.py
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
import numpy as n

# Test script to reproduce changes in weight of a dopamine modulated STDP synapse in an event-driven way.
# Pre- and post-synaptic spike trains are read in from spikes-6-0.gdf
# (output of test_stdp_dopa.py).
# output: pre/post/dopa \t spike time \t weight
# 
# Synaptic dynamics for dopamine modulated STDP synapses as used in [1], based on [2]
#
# References:
# [1] Potjans W, Morrison A and Diesmann M (2010). Enabling functional neural circuit simulations with distributed computing of neuromodulated plasticity.  Front. Comput. Neurosci. 4:141. doi:10.3389/fncom.2010.00141
# [2] Izhikevich, E. M. (2007). Solving the distal reward problem through linkage of STDP and dopamine signaling. Cereb. Cortex 17(10), 2443-2452.
# 
# author: Wiebke Potjans, October 2010


def stdp_dopa(w_init, pre_spikes, post_spikes, dopa_spikes, tau_e, tau_d, A_minus, A_plus, tau_plus, tau_minus, dendritic_delay, delay_d):
    
    w = w_init # initial weight
    w_min = 0. # minimal weight
    w_max = 200. #maximal weight
    i=0 # index of presynaptic spike
    j=0 # index of postsynaptic spike
    k=0 # index of dopamine spike

    last_post_spike = dendritic_delay
    Etrace = 0.
    Dtrace = 0.
    last_e_update = 0.
    last_w_update = 0.
    last_pre_spike = 0.
    last_dopa_spike = 0.
    
   
    advance = True

    while advance:
       
        advance = False
        
        # next spike is presynaptic
        if ((pre_spikes[i] < post_spikes[j]) and  (pre_spikes[i] < dopa_spikes[k])):
            dt = pre_spikes[i] - last_post_spike
            
             
            # weight update
            w = w + Etrace * Dtrace / (1./tau_e+1./tau_d) *(exp((last_e_update-last_w_update)/tau_e)*exp((last_dopa_spike-last_w_update)/tau_d)-exp((last_e_update-pre_spikes[i])/tau_e)*exp((last_dopa_spike-pre_spikes[i])/tau_d))
            
            if(w<w_min):
                w=w_min
            if(w>w_max):
                w=w_max
                       
           
            print "pre\t%.4f\t%.4f" % (pre_spikes[i],w)
            
            last_w_update = pre_spikes[i]          

            Etrace = Etrace * exp((last_e_update - pre_spikes[i])/tau_e) - A_minus*exp(-dt/tau_minus)
            
            last_e_update = pre_spikes[i]
            last_pre_spike = pre_spikes[i]
            
            if i < len(pre_spikes) - 1:
                i += 1
                advance = True
        

        # next spike is postsynaptic
        if( (post_spikes[j] < pre_spikes[i]) and (post_spikes[j] < dopa_spikes[k])):
            dt = post_spikes[j] - last_pre_spike

            # weight update
            w = w - Etrace * Dtrace / (1./tau_e+1./tau_d)*(exp((last_e_update-post_spikes[j])/tau_e)*exp((last_dopa_spike-post_spikes[j])/tau_d)-exp((last_e_update-last_w_update)/tau_e)*exp((last_dopa_spike-last_w_update)/tau_d))
            
            if(w<w_min):
                w=w_min
            if(w>w_max):
                w=w_max
            
            print "post\t%.4f\t%.4f" % (post_spikes[j],w)
            last_w_update = post_spikes[j]
            Etrace = Etrace * exp((last_e_update - post_spikes[j])/tau_e) + A_plus*exp(-dt/tau_plus)
           
            last_e_update = post_spikes[j] 
            last_post_spike = post_spikes[j]


            if j < len(post_spikes) - 1:
                j += 1
                advance = True

        # next spike is dopamine spike
        if ((dopa_spikes[k] < pre_spikes[i]) and (dopa_spikes[k] < post_spikes[j])):

        
            # weight update
            w = w - Etrace * Dtrace / (1./tau_e+1./tau_d) *(exp((last_e_update-dopa_spikes[k])/tau_e)*exp((last_dopa_spike-dopa_spikes[k])/tau_d)-exp((last_e_update-last_w_update)/tau_e)*exp((last_dopa_spike-last_w_update)/tau_d))
            if(w<w_min):
                w=w_min
            if(w>w_max):
                w=w_max
            

            print "dopa\t%.4f\t%.4f" % (dopa_spikes[k],w)
            last_w_update = dopa_spikes[k]
            Dtrace = Dtrace * exp((last_dopa_spike - dopa_spikes[k])/tau_d) + 1/tau_d
                                  
            last_dopa_spike = dopa_spikes[k]

            if k < len(dopa_spikes) - 1:
                k += 1
                advance = True
            
          
            if(dopa_spikes[k]==dopa_spikes[k-1]):
                advance = False
                Dtrace = Dtrace + 1/tau_d
                if k < len(dopa_spikes) - 1:
                    k += 1
                    advance = True


        # pre and postsynaptic spikes are at the same time
        # Etrace is not updated for this case; therefore no weight update is required
        if ((pre_spikes[i]==post_spikes[j]) and (pre_spikes[i] < dopa_spikes[k])):
            if i < len(pre_spikes) - 1:
                i += 1
                advance = True

            if j < len(post_spikes) -1:
                j +=1
                advance = True
                
            
        # presynaptic spike and dopamine spike are at the same time
        if ((pre_spikes[i]==dopa_spikes[k]) and (pre_spikes[i] < post_spikes[j])):
            dt = pre_spikes[i] - last_post_spike  

            w = w + Etrace * Dtrace / (1./tau_e+1./tau_d) *(exp((last_e_update-last_w_update)/tau_e)*exp((last_dopa_spike-last_w_update)/tau_d)-exp((last_e_update-pre_spikes[i])/tau_e)*exp((last_dopa_spike-pre_spikes[i])/tau_d))

            
            if(w<w_min):
                w=w_min
            if(w>w_max):
                w=w_max
            
            
            print "pre\t%.4f\t%.4f" % (pre_spikes[i],w)
            last_w_update = pre_spikes[i]
            Etrace = Etrace * exp((last_e_update - pre_spikes[i])/tau_e) - A_minus*exp(-dt/tau_minus)
                       
            last_e_update = pre_spikes[i]
            last_pre_spike = pre_spikes[i]
            
            if i < len(pre_spikes) - 1:
                i += 1
                advance = True

            Dtrace = Dtrace * exp((last_dopa_spike - dopa_spikes[k])/tau_d) + 1/tau_d
            last_dopa_spike = dopa_spikes[k]

            if k < len(dopa_spikes) - 1:
                k += 1
                advance = True

                    

        # postsynaptic spike and dopamine spike are at the same time
       
        if ((post_spikes[j]==dopa_spikes[k]) and (post_spikes[j] < pre_spikes[i])):           
            # weight update
            w = w - Etrace * Dtrace / (1./tau_e+1./tau_d)*(exp((last_e_update-post_spikes[j])/tau_e)*exp((last_dopa_spike-post_spikes[j])/tau_d)-exp((last_e_update-last_w_update)/tau_e)*exp((last_dopa_spike-last_w_update)/tau_d))
            if(w<w_min):
                w=w_min
            if(w>w_max):
                w=w_max
            

            print "post\t%.4f\t%.4f" % (post_spikes[j],w)
            last_w_update = post_spikes[j]
            Etrace = Etrace * exp((last_e_update - post_spikes[j])/tau_e) + A_plus*exp(-dt/tau_plus)
           
            last_e_update = post_spikes[j] 
            last_post_spike = post_spikes[j]


            if j < len(post_spikes) - 1:
                j += 1
                advance = True

            Dtrace = Dtrace * exp((last_dopa_spike - dopa_spikes[k])/tau_d) + 1/tau_d
            last_dopa_spike = dopa_spikes[k]

            if k < len(dopa_spikes) - 1:
                k += 1
                advance = True
            
                            
        # all three spikes are at the same time
        if ((post_spikes[j]==dopa_spikes[k]) and (post_spikes[j]==pre_spikes[i])):
             # weight update
            w = w - Etrace * Dtrace / (1./tau_e+1./tau_d) *(exp((last_e_update-dopa_spikes[k])/tau_e)*exp((last_dopa_spike-dopa_spikes[k])/tau_d)-exp((last_e_update-last_w_update)/tau_e)*exp((last_dopa_spike-last_w_update)/tau_d))
            if(w<w_min):
                w=w_min
            if(w>w_max):
                w=w_max
            

            print "dopa\t%.4f\t%.4f" % (dopa_spikes[k],w)
            last_w_update = dopa_spikes[k]
            Dtrace = Dtrace * exp((last_dopa_spike - dopa_spikes[k])/tau_d) + 1/tau_d
               
            last_dopa_spike = dopa_spikes[k]

            if k < len(dopa_spikes) - 1:
                k += 1
                advance = True
            
          
            if(dopa_spikes[k]==dopa_spikes[k-1]):
                advance = False
                Dtrace = Dtrace + 1/tau_d
                if k < len(dopa_spikes) - 1:
                    k += 1
                    advance = True

                        
                    
 
    return w

# stdp dopa parameters
w_init    = 35.
tau_plus  = 20.
tau_minus = 15.
tau_e = 1000.
tau_d = 200. 
A_minus = 1.5   
A_plus = 1.0
dendritic_delay = 1.0
delay_d = 1.


# load spikes from simulation with test_stdp_dopa.py
spikes = n.loadtxt("spikes-3-0.gdf")
pre_spikes = spikes[find(spikes[:,0]==4),1]
# delay is purely dendritic
# postsynaptic spike arrives at sp_j + dendritic_delay at the synapse
post_spikes =spikes[find(spikes[:,0]==5),1] + dendritic_delay
# dopa spike arrives  at sp_j + delay_d at the synapse
dopa_spikes = spikes[find(spikes[:,0]==6),1] + delay_d

# calculate development of stdp weight 
w = stdp_dopa(w_init, pre_spikes, post_spikes, dopa_spikes, tau_e, tau_d, A_minus, A_plus, tau_plus, tau_minus, dendritic_delay, delay_d)
print w


