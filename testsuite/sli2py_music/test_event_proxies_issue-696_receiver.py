#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# test_event_proxies_issue-696_receiver.py
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

n_neurons = 11

neurons = nest.Create("iaf_psc_alpha", n_neurons)
inputs = nest.Create(
    "music_event_in_proxy", n_neurons, params={"port_name": ["in"] * n_neurons, "music_channel": list(range(n_neurons))}
)

nest.Connect(inputs, neurons, "one_to_one", {"weight": 750.0})

nest.Simulate(1)
