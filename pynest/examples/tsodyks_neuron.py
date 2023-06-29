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

from pprint import pprint

import matplotlib.pyplot as plt
import numpy as np

import nest

nest.ResetKernel()

weight = 250.0
delay = 0.1

nest.SetDefaults("iaf_tsodyks", {"I_e": 0.0})

spikegen = nest.Create("spike_generator", 1)
nest.SetStatus(spikegen, {"spike_times": [1.0, 5.0]})

# iaf_tsodyks with static synapse
nrn_tsodyks_pre = nest.Create("iaf_tsodyks", 1)
nrn_tsodyks_post = nest.Create("iaf_tsodyks", 1)


# Set huge synaptic weight, control spike releases by refractory period
# nest.SetStatus(nrn_tsodyks_pre, {'t_ref': 10000.})

m_tsodyks = nest.Create("multimeter", 1)
nest.SetStatus(m_tsodyks, {"record_from": ["V_m", "I_syn_ex"]})

nest.CopyModel("static_synapse", "syn_static", {"weight": weight, "delay": delay})
nest.CopyModel("static_synapse", "spike_forcing_syn", {"weight": 10000000.0, "delay": delay})

nest.Connect(spikegen, nrn_tsodyks_pre, syn_spec="spike_forcing_syn")
nest.Connect(nrn_tsodyks_pre, nrn_tsodyks_post, syn_spec="syn_static")

nest.Connect(m_tsodyks, nrn_tsodyks_post)


# iaf_psc_exp with tsodyks_synapse
nrn_exp_pre = nest.Create("iaf_psc_exp", 1)
nrn_exp_post = nest.Create("iaf_psc_exp", 1)
# nest.SetStatus(nrn_exp_pre, {'t_ref': 10000.})

m_exp = nest.Create("multimeter", 1)
nest.SetStatus(m_exp, {"record_from": ["V_m", "I_syn_ex"]})
syn_param = {
    "tau_psc": 3.0,
    "tau_rec": 400.0,
    "tau_fac": 1000.0,
    "U": 0.5,
    "delay": delay,
    "weight": weight,
    "u": 0.0,
    "x": 0.0,
    "y": 0.0,
}
nest.CopyModel("tsodyks_synapse", "syn_tsodyks", syn_param)
nest.Connect(spikegen, nrn_exp_pre, syn_spec="spike_forcing_syn")
nest.Connect(nrn_exp_pre, nrn_exp_post, syn_spec="syn_tsodyks")
nest.Connect(m_exp, nrn_exp_post)

nest.Simulate(10.0)

r_tsodyks = nest.GetStatus(m_tsodyks, "events")[0]
print("iaf_tsodyks with static synapse:")
pprint(r_tsodyks)


r_exp = nest.GetStatus(m_exp, "events")[0]
print("\n iaf_psc_exp with tsodyks_synapse:")
pprint(r_exp)

fig, axes = plt.subplots(nrows=2, ncols=1, figsize=(8, 6), tight_layout=True)

axes[0].plot(r_tsodyks["V_m"], label="iaf_tsodyks / static_syn")
axes[0].plot(r_exp["V_m"], label="iaf_psc_exp / tsodyks_syn")
axes[0].set(xlabel="time", ylabel="V_m")
axes[0].legend(loc="upper right")

axes[1].plot(r_tsodyks["I_syn_ex"], label="iaf_tsodyks / static_syn")
axes[1].plot(r_exp["I_syn_ex"], label="iaf_psc_exp / tsodyks_syn")
axes[1].set(xlabel="time", ylabel="I_syn_ex")
axes[1].legend(loc="upper right")


plt.show()
