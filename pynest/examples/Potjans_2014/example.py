# -*- coding: utf-8 -*-
#
# example.py
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

'''
pynest microcicuit example
--------------------------

Example file to run the microcircuit.

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016
'''

import time
import numpy as np
import network
from network_params import net_dict
from sim_params import sim_dict
from stimulus_params import stim_dict


# Initialize the network and pass parameters to it.
net = network.Network(sim_dict, net_dict, stim_dict)
# Connect all nodes.
t1 = time.time()
net.setup()
time_con = time.time()-t1
print("Time to create the connections: %.2f seconds" % time_con)
# Simulate.
t2 = time.time()
net.simulate()
time_sim = time.time()-t2
print("Time to simulate: %.2f seconds" % time_sim)
# Plot a raster plot of the spikes of the simulated neurons and the average
# spike rate of all populations. For visual purposes only spikes in the last
# 200 ms are plotted here by default. The computation of spike rates discards
# the first 500 ms of the simulation to exclude initialization artifacts.
raster_plot_time_idx = np.array([sim_dict['t_sim'] - 200.0, sim_dict['t_sim']])
fire_rate_time_idx = np.array([500.0, sim_dict['t_sim']])
net.evaluate(raster_plot_time_idx, fire_rate_time_idx)
