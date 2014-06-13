# -*- coding: utf-8 -*-
#
# test_quantal_stp_synapse.py
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

# This script compares the two variants of the Tsodyks/Markram synapse in NEST.

import nest
import nest.voltage_trace
import numpy
import pylab

nest.ResetKernel()
n_syn=10.0 # number of synapses in a connection
n_trials=100# number of measurement trials

# parameter set for facilitation
fac_params={"U":0.02, "u":0.02, "tau_fac":500., "tau_rec":200.,"weight":1.}

# Here we assign the parameter set to the synapse models 
t1_params=fac_params       # for tsodyks2_synapse
t2_params=t1_params.copy() # for furhmann_synapse

t1_params['x']=t1_params['U']
t2_params['n']=n_syn
t2_params['weight']=1./n_syn

nest.SetDefaults("tsodyks2_synapse",t1_params)
nest.SetDefaults("quantal_stp_synapse",t2_params)
nest.SetDefaults("iaf_psc_exp",{"tau_syn_ex": 3.})

neuron = nest.Create("iaf_psc_exp",3)

nest.Connect([neuron[0]],[neuron[1]],syn_spec="tsodyks2_synapse")
nest.Connect([neuron[0]],[neuron[2]],syn_spec="quantal_stp_synapse")

voltmeter = nest.Create("voltmeter",2)
nest.SetStatus(voltmeter, {"withgid": True, "withtime": True})

# one dry run to brin synapses in 'standard' rest state
nest.SetStatus([neuron[0]], "I_e", 376.0)
nest.Simulate(500.0)
nest.SetStatus([neuron[0]], "I_e", 0.0)
nest.Simulate(1000.0)

nest.Connect([voltmeter[0]], [neuron[1]])
nest.Connect([voltmeter[1]], [neuron[2]])

for t in range(n_trials):
    nest.SetStatus([neuron[0]], "I_e", 376.0)
    nest.Simulate(500.0)
    nest.SetStatus([neuron[0]], "I_e", 0.0)
    nest.Simulate(1000.0)

nest.Simulate(.1) # flush the last voltmeter events from the queue

vm= numpy.array(nest.GetStatus([voltmeter[1]],'events')[0]['V_m'])
vm_reference=numpy.array(nest.GetStatus([voltmeter[0]],'events')[0]['V_m'])

vm.shape=(n_trials,1500)
vm_reference.shape=(n_trials,1500)

vm_mean=numpy.array([numpy.mean(vm[:,i]) for (i,j) in enumerate(vm[0,:])]) 
vm_ref_mean=numpy.array([numpy.mean(vm_reference[:,i]) for (i,j) in enumerate(vm_reference[0,:])]) 
pylab.plot(vm_mean)
pylab.plot(vm_ref_mean)
print (numpy.mean((vm_ref_mean-vm_mean)**2))
pylab.show()
