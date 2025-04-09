# -*- coding: utf-8 -*-
#
# run_simulation.py
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

"""PyNEST EI-clustered network: Run Simulation
-----------------------------------------------

This is an example script for running the EI-clustered model
with two stimulations and generating a raster plot.
"""

import matplotlib.pyplot as plt
import network
from helper import raster_plot
from network_params import net_dict
from sim_params import sim_dict
from stimulus_params import stim_dict

if __name__ == "__main__":
    # Creates object which creates the EI clustered network in NEST
    ei_network = network.ClusteredNetwork(sim_dict, net_dict, stim_dict)

    # Runs the simulation and returns the spiketimes
    # get simulation initializes the network in NEST
    # and runs the simulation
    # it returns a dict with the average rates,
    # the spiketimes and the used parameters
    result = ei_network.get_simulation()
    ax = raster_plot(
        result["spiketimes"],
        tlim=(0, sim_dict["simtime"]),
        colorgroups=[
            ("k", 0, net_dict["N_E"]),
            ("darkred", net_dict["N_E"], net_dict["N_E"] + net_dict["N_I"]),
        ],
    )
    plt.savefig("clustered_ei_raster.png")
    print(f"Firing rate of excitatory neurons: {result['e_rate']:6.2f} spikes/s")
    print(f"Firing rate of inhibitory neurons: {result['i_rate']:6.2f} spikes/s")
