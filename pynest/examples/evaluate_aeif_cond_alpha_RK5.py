# -*- coding: utf-8 -*-
#
# evaluate_aeif_cond_alpha_RK5.py
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

# Test script for new implementation of AdEx numerics.
#  Stefan BUCHER (web@stefan-bucher.ch), July 2013.

import nest
import numpy as np
import timeit

"""
Example of the Adaptive Exponential Integrate and Fire (AdEx) in NEST.
----------------------------------------------------------------------

This script compares the two aeif_cond_alpha flavors with respect to speed
and precision.

Version 1 is the GSL based 'aeif_cond_alpha' model.
Version 2 is called 'aeif_cond_alpha_RK5' which uses an explicitly coded
version of the RK-45 method as described in Numerical Recepies, Chap. 17.2,
Press et al (2007).

Reference is Version 1 at a temporal resolution of 0.001 ms.

The test comparest both versions at a resolution of 0.1 ms with the reference.
Two errors are computed:
1. the difference is spike times wrt reference
2. the L2 (root mean squared) error of the voltage response
   to a step current input.

aeif_cond_alpha_RK5 is the adaptive exponential integrate-and-fire neuron
according to Brette and Gerstner (2005). Synaptic conductances are modelled
as alpha-functions.

This implementation uses a 5th order Runge-Kutta solver with adaptive stepsize
to integrate the differential equation (see Numerical Recipes 3rd Edition,
Press et al. 2007, Ch. 17.2).

The membrane potential is given by the following differential equation:

  C dV/dt = - g_L (V-E_L) + g_L Delta_T exp((V-V_T)/Delta_T)
            - g_e(t) (V-E_e) - g_i(t) (V-E_i) - w + I_e

and

  tau_w dw/dt = a (V-E_L) - w

Parameters:
The following parameters can be set in the status dictionary.

Dynamic state variables:
  V_m        double - Membrane potential in mV
  g_ex       double - Excitatory synaptic conductance in nS.
  dg_ex      double - First derivative of g_ex in nS/ms
  g_in       double - Inhibitory synaptic conductance in nS.
  dg_in      double - First derivative of g_in in nS/ms.
  w          double - Spike-adaptation current in pA.

Membrane Parameters:
  C_m        double - Capacity of the membrane in pF
  t_ref      double - Duration of refractory period in ms.
  V_reset    double - Reset value for V_m after a spike. In mV.
  E_L        double - Leak reversal potential in mV.
  g_L        double - Leak conductance in nS.
  I_e        double - Constant external input current in pA.

Spike adaptation parameters:
  a          double - Subthreshold adaptation in nS.
  b          double - Spike-triggered adaptation in pA.
  Delta_T    double - Slope factor in mV
  tau_w      double - Adaptation time constant in ms
  V_th       double - Spike initiation threshold in mV
  V_peak     double - Spike detection threshold in mV.

Synaptic parameters:
  E_ex       double - Excitatory reversal potential in mV.
  tau_syn_ex double - Rise time of excitatory synaptic conductance
                      in ms (alpha function).
  E_in       double - Inhibitory reversal potential in mV.
  tau_syn_in double - Rise time of the inhibitory synaptic conductance
                      in ms (alpha function).

Numerical integration parameters:
  HMIN       double - Minimal stepsize for numerical integration
                      in ms (default 0.001ms).
  MAXERR     double - Error estimate tolerance for adaptive stepsize control
                      (steps accepted if err<=MAXERR). In mV. Note that the
                      error refers to the difference between the 4th and 5th
                      order RK terms. Default 1e-10 mV.
"""


def run_model(model='aeif_cond_alpha', dt=0.1, reps=1):
    '''
    Helper function to run a standard protocol with a neuron of model `model.
    The parameters specify the resolution and number of repetitions.
    '''
    nest.ResetKernel()
    nest.sr("30 setverbosity")
    nest.SetKernelStatus({"overwrite_files": True})
    nest.SetStatus([0], [{"resolution": dt}])
    nest.SetDefaults('aeif_cond_alpha_RK5', {'HMIN': 0.001})
    nest.SetDefaults('aeif_cond_alpha_RK5', {'MAXERR': 1e-10})

    neuron = nest.Create(model, 2)
    nest.SetStatus(neuron, [{"V_peak": 0.0, "a": 4.0, "b": 80.5}])

    dc = nest.Create("dc_generator")
    nest.SetStatus(dc, [{"amplitude": 700.0,
                         "start": 700.0,
                         "stop": 2700.0}])
    nest.Connect(dc, [neuron[0]])

    sd = nest.Create('spike_detector')
    nest.Connect([neuron[0]], sd)

    meter0 = nest.Create('multimeter',
                         params={'record_from': ['V_m', 'g_ex', 'g_in', 'w'],
                                 'interval': 0.1})
    nest.Connect(meter0, [neuron[0]])
    nest.SetStatus(meter0, [{"to_file": False, "withtime": True}])

    t = timeit.Timer("nest.Simulate(3000)", "import nest")
    runtime = t.timeit(number=reps) / reps
    sptimes = nest.GetStatus(sd, "events")[0]['times']
    voltage_trace = nest.GetStatus(meter0, "events")[0]['V_m']
    return (runtime, sptimes, voltage_trace)

'''
Now we run the different simulations.
Three traces a produced. First, a reference trace of the original
`aeif_cond_alpha model with 100 times higher resolution than the default.
The second trace is from the `aeif_cond_alpha model with default resolution
and the final trace is from the `aeif_cond_alphaRK5 model also at default
resolution.
'''

reference = run_model(model='aeif_cond_alpha', dt=0.001, reps=50)
gsl = run_model(model='aeif_cond_alpha', dt=0.1, reps=50)
test = run_model(model='aeif_cond_alpha_RK5', dt=0.1, reps=50)

'''
Compare the execution times of the models.
'''

print 'Runtime GSL: ' + str(gsl[0])
print 'Test: ' + str(test[0])
print 'Ratio: ' + str(test[0] / gsl[0])

'''
Compute the error on the spike times.
'''
print 'Spike Times GSL - Reference:' + str(np.array(gsl[1]) -
                                           np.array(reference[1]))
print 'Spike Times Test - Reference: ' + str(np.array(test[1]) -
                                             np.array(reference[1]))

'''
Also compare the L2-Norm of Voltage Traces
'''

print ('L2-Error (per s) GSL - Reference: ' +
       str(np.linalg.norm(gsl[2] - reference[2], ord=2) / 3))
print ('L2-Error (per s) Test - Reference: ' +
       str(np.linalg.norm(test[2] - reference[2], ord=2) / 3))
print ('L2-Error ratio: ' +
       str((np.linalg.norm(test[2] - reference[2], ord=2) / 3) /
           (np.linalg.norm(gsl[2] - reference[2], ord=2) / 3)))
