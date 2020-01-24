# -*- coding: utf-8 -*-
#
# run_microcircuit.py
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

"""PyNEST Microcircuit Example
---------------------------------

This is an example script for running the microcircuit model and generating
basic plots of the network activity.

"""

###############################################################################
# Import the necessary modules.

import time
import numpy as np
import network
from network_params import net_dict
from sim_params import sim_dict
from stimulus_params import stim_dict

###############################################################################
# Initialize the network with simulation, network and stimulation parameters,
# create and connect all nodes and simulate.

tic = time.time()
net = network.Network(sim_dict, net_dict, stim_dict)
toc = time.time() - tic
print("Time to initialize the network: %.2f s" % toc)

tic = time.time()
net.setup()
toc = time.time() - tic
print("Time to create and connect all nodes: %.2f s" % toc)

tic = time.time()
net.simulate()
toc = time.time() - tic
print("Time to simulate: %.2f s" % toc)

###############################################################################
# Plot a spike raster of the simulated neurons and the per-neuron average spike
# rate of all populations. For visual purposes only, spikes 100 ms
# before and 100 ms after the thalamic stimulus time are plotted here by
# default. The computation of spike rates discards the first 500 ms of
# the simulation to exclude initialization artifacts.

raster_plot_time_idx = np.array(
    [stim_dict['th_start'] - 100.0, stim_dict['th_start'] + 100.0]
    )
fire_rate_time_idx = np.array([500.0, sim_dict['t_sim']])
net.evaluate(raster_plot_time_idx, fire_rate_time_idx)
