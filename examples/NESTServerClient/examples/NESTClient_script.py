# -*- coding: utf-8 -*-
#
# NESTClient_script.py
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


# Reset kernel
nest.ResetKernel()

# Create nodes
pg = nest.Create("poisson_generator", params={"rate": 6500.})
neurons = nest.Create("iaf_psc_alpha", 100)
sr = nest.Create("spike_recorder")

# Connect nodes
nest.Connect(pg, neurons, syn_spec={"weight": 10.})
nest.Connect(neurons[::10], sr)

# Simulate
nest.Simulate(1000.)

# Get events
n_events = sr.get("n_events")
