# -*- coding: utf-8 -*-
'''
/*
 *  tsodyks_depressing.py
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* BeginDocumentation
Name: tsodyks_depressing - python script for overall test of iaf_neuron model
Derived from tsodyks_depressing.sli.

Description:
Script to test Tsodyks short term plasticity depressing synapses according to
'Neural Networks with Dynamic Synapses'
Misha Tsodyks, Klaus Pawelzik, Henry Markram
Neural computation 10, 821--853 (1998)

reproduces figure 1 A


author:  Birgit Kriener, Moritz Helias, Markus Diesmann
date:	 March 2006

'''

# import nest kernel:
import nest
import nest.voltage_trace
from numpy import exp
import pylab

# set parameters:
h       = 0.1    # simulation step size (ms)
Tau     = 40.    # membrane time constant
Theta   = 15.    # threshold
U0      = 0.     # reset potential of membrane potential
R       = 0.1    # 100 M Ohm
C       = Tau/R  # Tau (ms)/R in NEST units
TauR    = 2.     # refractory time
Tau_psc = 3.     # time constant of PSC (= Tau_inact)
Tau_rec = 800.   # recovery time
Tau_fac = 0.     # facilitation time
U       = 0.5    # facilitation parameter U
A       = 250.   # PSC weight in pA
f       = 20./1000. # frequency in Hz converted to 1/ms
Tend    = 1200.  # simulation time
TIstart = 50.    # start time of dc
TIend   = 1050.  # end time of dc
I0      = Theta*C/Tau/(1-exp(-(1/f-TauR)/Tau)) # dc amplitude

# print I0

# set up simulator:
nest.ResetKernel()
nest.SetKernelStatus({"resolution": h})

# set neuron parameters:
neuron_param = {"tau_m"     :  Tau,
                "t_ref"     :  TauR,
                "tau_syn_ex":  Tau_psc,
                "tau_syn_in":  Tau_psc,
                "C_m"       :  C,
                "V_reset"   :  U0,
                "E_L"       :  U0,
                "V_m"       :  U0,
                "V_th"      :  Theta}

# set defaults of desired neuron type with chosen parameters:
nest.SetDefaults("iaf_psc_exp", neuron_param)

# create two neurons of desired type:
neurons = nest.Create("iaf_psc_exp",2)

# set properties of dc:
nest.SetDefaults("dc_generator",{"amplitude": I0, "start": TIstart, "stop": TIend})

# create dc_generator:
dc_gen = nest.Create("dc_generator")

# create voltmeter
volts=nest.Create("voltmeter")

# set properties of voltmeter
nest.SetStatus(volts,[{"label": "Voltmeter", "withtime": True, "withgid": True,
                         "interval": 1.}])

# connect dc_generator to neuron 1:
nest.Connect(dc_gen,[neurons[0]])

# connect voltmeter to neuron 2:
nest.Connect(volts,[neurons[1]])

# set synapse parameters:
syn_param = {"tau_psc" :  Tau_psc,
            "tau_rec" :  Tau_rec,
            "tau_fac" :  Tau_fac,
            "U"       :  U,
            "delay"   :  0.1,
            "weight"  :  A,
            "u"       :  0.0,
            "x"       :  1.0}

# create desired synapse type with chosen parameters:
nest.CopyModel("tsodyks_synapse","syn",syn_param)

# connect neuron 1 with neuron 2 via synapse model 'syn':
nest.Connect([neurons[0]],[neurons[1]],model="syn")

# simulate:
nest.Simulate(Tend)

# plot membrane potential of neuron
nest.voltage_trace.from_device(volts)
