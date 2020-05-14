# -*- coding: utf-8 -*-
#
# NESTClientAPI_example.py
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

from NESTServerClient import NESTClientAPI


# API interface for NEST Server
napi = NESTClientAPI()

# Reset kernel
napi.ResetKernel()

# Create nodes
pg = napi.Create("poisson_generator", params={"rate": 6500.})
neurons = napi.Create("iaf_psc_alpha", 100)
sd = napi.Create("spike_detector")

# Connect nodes
napi.Connect(pg, neurons, syn_spec={'weight': 10.})
napi.Connect(neurons, sd)

# Simulate
napi.Simulate(1000.0)

# Get events
n_events = napi.GetStatus(sd, 'n_events')[0]
print('Number of events:', n_events)
