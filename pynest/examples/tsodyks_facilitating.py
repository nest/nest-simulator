# -*- coding: utf-8 -*-
#
# tsodyks_facilitating.py
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

'''
/* BeginDocumentation
Name: tsodyks_facilitating - python script for overall test of iaf_neuron model
Derived from tsodyks_facilitating.sli.

Description:
Script to test Tsodyks short term plasticity facilitating synapses according to
'Neural Networks with Dynamic Synapses'
Misha Tsodyks, Klaus Pawelzik, Henry Markram
Neural computation 10, 821--853 (1998)

reproduces figure 1 B


author:  Birgit Kriener, Moritz Helias, Markus Diesmann
date:	 March 2006

'''

# import nest kernel:
import nest
import nest.voltage_trace
import pylab
from numpy import exp


# set parameters:
h       = 0.1    # simulation step size (ms)
Tau     = 40.    # membrane time constant
Theta   = 15.    # threshold
U0      = 0.     # reset potential of membrane potential
R       = 1.     # membrane resistance (GOhm)
C       = Tau/R  # Tau (ms)/R in NEST units
TauR    = 2.     # refractory time
Tau_psc = 1.5    # time constant of PSC (= Tau_inact)
Tau_rec = 130.   # recovery time
Tau_fac = 530.   # facilitation time
U       = 0.03   # facilitation parameter U
A       = 1540.  # PSC weight in pA
f       = 20./1000. # frequency in Hz converted to 1/ms
Tend    = 1200.  # simulation time
TIstart = 50.    # start time of dc
TIend   = 1050.  # end time of dc
I0      = Theta*C/Tau/(1-exp(-(1/f-TauR)/Tau)) # dc amplitude

# set up simulator:
nest.ResetKernel()
nest.SetKernelStatus({"resolution": h})

# set neuron parameters:
neuron_param = {"tau_m"    :  Tau,
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

# set properties of voltmeter:
volts=nest.Create("voltmeter")

# create voltmeter:
nest.SetStatus(volts,[{"label": "voltmeter","withtime": True,"withgid": True,
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
nest.Connect([neurons[0]],[neurons[1]],syn_spec={'model': "syn"})

# simulate:
nest.Simulate(Tend)

# print membrane potential of neuron 2:
nest.voltage_trace.from_device(volts)
