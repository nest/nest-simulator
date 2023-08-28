# -*- coding: utf-8 -*-
#
# verify_offset.py
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
import nest
import numpy as np

# TODO: This is just a test script, remove from repository before PR

nest.ResetKernel()

weight = 250.0
delay = 0.1

nest.SetDefaults("iaf_tum_2000", {"I_e": 0.0})

spikegen = nest.Create("spike_generator", 1)
nest.SetStatus(spikegen, {"spike_times": [1.0]})

# iaf_tum_2000 with static synapse
# nrn_pre = nest.Create("iaf_psc_exp_ps", 1)
nrn_pre = nest.Create("iaf_tum_2000", 1)
nrn_post = nest.Create("iaf_tum_2000", 1)


# Set huge synaptic weight, control spike releases by refractory period
nest.SetStatus(nrn_pre, {"t_ref": 10000.0})

# m = nest.Create('multimeter', 1)
# nest.SetStatus(m, {"record_from": ["V_m", "I_syn_ex"]})

nest.CopyModel("static_synapse", "syn_static", {"weight": weight, "delay": delay})
nest.CopyModel("static_synapse", "spike_forcing_syn", {"weight": 10000000.0, "delay": delay})

nest.Connect(spikegen, nrn_pre, syn_spec="spike_forcing_syn")
nest.Connect(nrn_pre, nrn_post, syn_spec="syn_static")

# nest.Connect(m, nrn_post)

nest.Simulate(10)
