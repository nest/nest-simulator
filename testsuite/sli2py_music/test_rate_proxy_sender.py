#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# test_rate_proxy_sender.py
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

rate_neuron = nest.Create("lin_rate_ipn", params={"rate": 1.5, "mu": 1.5, "sigma": 0.0})

mrop = nest.Create("music_rate_out_proxy", params={"port_name": "rate_out"})

nest.Connect(
    rate_neuron, mrop, "one_to_one", syn_spec={"synapse_model": "rate_connection_instantaneous", "music_channel": 0}
)

nest.Simulate(500)
