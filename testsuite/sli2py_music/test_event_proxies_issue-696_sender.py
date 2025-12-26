#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# test_event_proxies_issue-696_sender.py
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

nest.local_num_threads = 10
nest.resolution = 0.1

n_neurons = 11
generators = nest.Create("spike_generator", n_neurons, params={"spike_times": [0.1, 0.2, 0.3]})
neurons = nest.Create("parrot_neuron", n_neurons)

nest.Connect(generators, neurons, "one_to_one", {"delay": 0.1})

meop = nest.Create("music_event_out_proxy", params={"port_name": "out"})


nest.Connect(neurons, meop, "all_to_all", {"music_channel": [list(range(n_neurons))]})

nest.Simulate(1)
