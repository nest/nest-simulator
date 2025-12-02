#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# test_rate_proxy_receiver.py
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

mrip = nest.Create("music_rate_in_proxy", params={"port_name": "rate_in", "music_channel": 0})
neuron = nest.Create("lin_rate_ipn", params={"sigma": 0.0})
# To let receiving neuron reach equilibrium
mm = nest.Create("multimeter", params={"interval": 0.1, "record_from": ["rate"], "start": 400.0})

nest.Connect(mrip, neuron, "one_to_one", syn_spec={"synapse_model": "rate_connection_instantaneous"})
nest.Connect(mm, neuron)

nest.Simulate(500)

rates = mm.events["rate"]

# Test that all rates are equal to the drive rate, 1.5, in the sender
assert min(rates) == max(rates)
assert max(rates) == 1.5
