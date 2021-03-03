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

"""PyNEST Microcircuit: Run Simulation
-----------------------------------------

This is an example script for running the microcircuit model and generating
basic plots of the network activity.

"""

###############################################################################
# Import the necessary modules and start the time measurements.

from stimulus_params import stim_dict
from network_params import net_dict
from sim_params import sim_dict
import network
import nest
import numpy as np
import time
time_start = time.time()

###############################################################################
# Initialize the network with simulation, network and stimulation parameters,
# then create and connect all nodes, and finally simulate.
# The times for a presimulation and the main simulation are taken
# independently. A presimulation is useful because the spike activity typically
# exhibits a startup transient. In benchmark simulations, this transient should
# be excluded from a time measurement of the state propagation phase. Besides,
# statistical measures of the spike activity should only be computed after the
# transient has passed.

net = network.Network(sim_dict, net_dict, stim_dict)
time_network = time.time()

net.create()
time_create = time.time()

net.connect()
time_connect = time.time()

net.simulate(sim_dict['t_presim'])
time_presimulate = time.time()

net.simulate(sim_dict['t_sim'])
time_simulate = time.time()

###############################################################################
# Plot a spike raster of the simulated neurons and a box plot of the firing
# rates for each population.
# For visual purposes only, spikes 100 ms before and 100 ms after the thalamic
# stimulus time are plotted here by default.
# The computation of spike rates discards the presimulation time to exclude
# initialization artifacts.

raster_plot_interval = np.array([stim_dict['th_start'] - 100.0,
                                 stim_dict['th_start'] + 100.0])
firing_rates_interval = np.array([sim_dict['t_presim'],
                                  sim_dict['t_presim'] + sim_dict['t_sim']])
net.evaluate(raster_plot_interval, firing_rates_interval)
time_evaluate = time.time()

###############################################################################
# Summarize time measurements. Rank 0 usually takes longest because of the
# data evaluation and print calls.

print(
    '\nTimes of Rank {}:\n'.format(
        nest.Rank()) +
    '  Total time:          {:.3f} s\n'.format(
        time_evaluate -
        time_start) +
    '  Time to initialize:  {:.3f} s\n'.format(
        time_network -
        time_start) +
    '  Time to create:      {:.3f} s\n'.format(
        time_create -
        time_network) +
    '  Time to connect:     {:.3f} s\n'.format(
        time_connect -
        time_create) +
    '  Time to presimulate: {:.3f} s\n'.format(
        time_presimulate -
        time_connect) +
    '  Time to simulate:    {:.3f} s\n'.format(
        time_simulate -
        time_presimulate) +
    '  Time to evaluate:    {:.3f} s\n'.format(
        time_evaluate -
        time_simulate))
