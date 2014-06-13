# -*- coding: utf-8 -*-
#
# test-aeif_cond_alpha_RK5.py
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

## Test script for new implementation of AdEx numerics.
#  Stefan BUCHER (web@stefan-bucher.ch), July 2013.

import nest
import numpy as np
import timeit

"""
This script compares the two aeif_cond_alpha flavors with respect to sped and precision.
Version 1 is the GSL based 'aeif_cond_alpha' model.
Version 2 is called 'aeif_cond_alpha_RK5' which uses an explicitly coded version
of the RK-45 method as described in Numerical Recepies, Chap. 17.2, Press et al (2007).
Reference is Version 1 at a temporal resolution of 0.001 ms.

The test comparest both versions at a resolution of 0.1 ms with the reference. Tho errors are computed:
1. the difference is spike times wrt reference
2. the L2 (root mean squared) error of the voltage response to a step current input.
"""

def run_model(model='aeif_cond_alpha', dt=0.1,reps=1):
  nest.ResetKernel()
  nest.sr("30 setverbosity")
  nest.SetKernelStatus({"overwrite_files": True})
  nest.SetStatus([0],[{"resolution": dt}])
  nest.SetDefaults('aeif_cond_alpha_RK5',{'HMIN':0.001})
  nest.SetDefaults('aeif_cond_alpha_RK5',{'MAXERR':1e-10})
 
  neuron = nest.Create(model,2)
  nest.SetStatus(neuron,[{"V_peak": 0.0, "a": 4.0, "b":80.5}])
  
  dc=nest.Create("dc_generator")
  nest.SetStatus(dc,[{"amplitude":700.0, 
                      "start":700.0, 
                      "stop":2700.0}])
  nest.Connect(dc,[neuron[0]])
    
  sd = nest.Create('spike_detector')
  nest.Connect([neuron[0]],sd)
   
  meter0 = nest.Create('multimeter', params={'record_from': ['V_m', 'g_ex','g_in','w'], 'interval' :0.1})
  nest.Connect(meter0,[neuron[0]])
  nest.SetStatus(meter0,[{"to_file": False, "withtime": True}])
 
  t = timeit.Timer("nest.Simulate(3000)","import nest") 
  runtime = t.timeit(number=reps)/reps
  sptimes = nest.GetStatus(sd,"events")[0]['times']
  voltage_trace = nest.GetStatus(meter0,"events")[0]['V_m']
  return (runtime,sptimes,voltage_trace)

# Running Simulations
reference = run_model(model='aeif_cond_alpha',dt=0.001,reps=50)
gsl = run_model(model='aeif_cond_alpha',dt=0.1,reps=50)
test = run_model(model='aeif_cond_alpha_RK5',dt=0.1,reps=50)

# Runtime Comparison
print 'Runtime GSL: ' + str(gsl[0])
print 'Test: ' + str(test[0])
print 'Ratio: ' + str(test[0]/gsl[0])

# Spike Time Difference
print 'Spike Times GSL - Reference: ' + str(np.array(gsl[1])-np.array(reference[1]))
print 'Spike Times Test - Reference: ' + str(np.array(test[1])-np.array(reference[1]))

# L2-Norm of Voltage Traces
print 'L2-Error (per s) GSL - Reference: ' + str(np.linalg.norm(gsl[2]-reference[2],ord=2)/3 )
print 'L2-Error (per s) Test - Reference: ' + str(np.linalg.norm(test[2]-reference[2],ord=2)/3 )
print 'L2-Error ratio: ' + str((np.linalg.norm(test[2]-reference[2],ord=2)/3) /(np.linalg.norm(gsl[2]-reference[2],ord=2)/3) )
