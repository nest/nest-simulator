# -*- coding: utf-8 -*-
#
# test_tsodyks2_synapse.py
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

nest.ResetKernel()

#Parameter set for depression
dep_params={"U":0.67, "u":0.67, 'x':1.0, "tau_rec":450.0, "tau_fac":0.0, "weight":250.}

# parameter set for facilitation
fac_params={"U":0.1, "u":0.1, 'x':1.0, "tau_fac":1000.,"tau_rec":100.,"weight":250.}

# Here we assign the parameter set to the synapse models 
t1_params=fac_params       # for tsodyks_synapse
t2_params=t1_params.copy() # for tsodyks2_synapse

nest.SetDefaults("tsodyks2_synapse",t1_params)
nest.SetDefaults("tsodyks_synapse",t2_params)
nest.SetDefaults("iaf_psc_exp",{"tau_syn_ex": 3.})

neuron = nest.Create("iaf_psc_exp",3)

nest.Connect([neuron[0]],[neuron[1]],syn_spec="tsodyks_synapse")
nest.Connect([neuron[0]],[neuron[2]],syn_spec="tsodyks2_synapse")
voltmeter = nest.Create("voltmeter",2)
nest.SetStatus(voltmeter, {"withgid": True, "withtime": True})
nest.Connect([voltmeter[0]], [neuron[1]])
nest.Connect([voltmeter[1]], [neuron[2]])

nest.SetStatus([neuron[0]], "I_e", 376.0)
nest.Simulate(500.0)
nest.SetStatus([neuron[0]], "I_e", 0.0)
nest.Simulate(500.0)
nest.SetStatus([neuron[0]], "I_e", 376.0)
nest.Simulate(500.0)

nest.voltage_trace.from_device([voltmeter[0]])
nest.voltage_trace.from_device([voltmeter[1]])
nest.voltage_trace.show()
