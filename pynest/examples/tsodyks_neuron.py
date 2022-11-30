# -*- coding: utf-8 -*-
#
# tsodyks_neuron.py
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

import nest
from pprint import pprint

nest.ResetKernel()


weight = 250.0
delay = 0.1

spikegen = nest.Create('spike_generator', 1)
nest.SetStatus(spikegen, {"spike_times": [1., 3., 5., 6., ]})

# iaf_tsodyks with static synapse
nrn_tsodyks = nest.Create("iaf_tsodyks", 2)
m_tsodyks = nest.Create('multimeter', 1)
nest.SetStatus(m_tsodyks, {"record_from": ["V_m", "I_syn_ex"]})
nest.CopyModel("static_synapse", "syn_static", {"weight": weight, "delay": delay})
nest.Connect(nrn_tsodyks, nrn_tsodyks, syn_spec="syn_static")
nest.Connect(spikegen, nrn_tsodyks)
nest.Connect(m_tsodyks, nrn_tsodyks)

# iaf_psc_exp with tsodyks_synapse
nrn_exp = nest.Create("iaf_psc_exp", 2)
m_exp = nest.Create('multimeter', 1)
nest.SetStatus(m_exp, {"record_from": ["V_m", "I_syn_ex"]})
syn_param = {"tau_psc": 3.0,
             "tau_rec": 400.0,
             "tau_fac": 1000.0,
             "U": 0.5,
             "delay": delay,
             "weight": weight,
             "u": 0.0,
             "x": 0.0,
             "y": 0.0
             }
nest.CopyModel("tsodyks_synapse", "syn_tsodyks", syn_param)
nest.Connect(nrn_exp, nrn_exp, syn_spec="syn_tsodyks")
nest.Connect(spikegen, nrn_exp)
nest.Connect(m_exp, nrn_exp)

nest.Simulate(10.0)

r_tsodyks = nest.GetStatus(m_tsodyks, "events")[0]
r_exp = nest.GetStatus(m_exp, "events")[0]

print("iaf_tsodyks with static synapse:")
pprint(r_tsodyks)

print("\n iaf_psc_exp with tsodyks_synapse:")
pprint(r_exp)
