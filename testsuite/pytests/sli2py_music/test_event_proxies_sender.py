#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# test_event_proxies_sender.py
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
import numpy as np

# create voltmeter first to ensure same GID in sender and receiver
vm = nest.Create("voltmeter", params={"label": "sender", "record_to": "memory"})

sg = nest.Create("spike_generator", params={"spike_times": [1.0, 1.5, 2.0]})

n = nest.Create("iaf_psc_alpha")

# create last to ensure same GID for neuron as in receiver
pn = nest.Create("parrot_neuron")

nest.Connect(sg, pn)
nest.Connect(pn, n, "one_to_one", syn_spec={"weight": 750.0})

nest.Connect(vm, n)

meop = nest.Create("music_event_out_proxy", params={"port_name": "spikes_out"})

nest.Connect(pn, meop, "one_to_one", syn_spec={"music_channel": 0})

nest.Simulate(10)

reference_vms = [-70.000, -70.000, -70.000, -68.156, -61.917, -70.000, -70.000, -70.000, -65.205]

np.testing.assert_allclose(reference_vms, vm.events["V_m"], atol=1e-3)
