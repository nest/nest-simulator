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
pynest microcircuit example
---------------------------

Example file to run the microcircuit.

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016

This example uses the function GetNodes, which is deprecated. A deprecation
warning is therefore issued. For details about deprecated functions, see
documentation.
'''

import time
import numpy as np
import network
from network_params import net_dict
from sim_params import sim_dict
from stimulus_params import stim_dict


# Initialize the network and pass parameters to it.
tic = time.time()
net = network.Network(sim_dict, net_dict, stim_dict)
toc = time.time() - tic
print("Time to initialize the network: %.2f s" % toc)
# Connect all nodes.
tic = time.time()
net.setup()
toc = time.time() - tic
print("Time to create the connections: %.2f s" % toc)
# Simulate.
tic = time.time()
net.simulate()
toc = time.time() - tic
print("Time to simulate: %.2f s" % toc)
# Plot a raster plot of the spikes of the simulated neurons and the average
# spike rate of all populations. For visual purposes only spikes 100 ms
# before and 100 ms after the thalamic stimulus time are plotted here by
# default. The computation of spike rates discards the first 500 ms of
# the simulation to exclude initialization artifacts.
raster_plot_time_idx = np.array(
    [stim_dict['th_start'] - 100.0, stim_dict['th_start'] + 100.0]
    )
fire_rate_time_idx = np.array([500.0, sim_dict['t_sim']])
net.evaluate(raster_plot_time_idx, fire_rate_time_idx)
