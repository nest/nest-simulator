# -*- coding: utf-8 -*-
#
# sim_params.py
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

"""PyNEST EI-clustered network: Simulation Parameters
------------------------------------------------

A dictionary with parameters defining the simulation.

"""

sim_dict = {
    # The full simulation time is the sum of a presimulation time and the main
    # simulation time.
    # presimulation time (in ms)
    "warmup": 1000.0,
    # simulation time (in ms)
    "simtime": 10000.0,
    # resolution of the simulation (in ms)
    "dt": 0.1,
    "randseed": 55,
    # Number of virtual processes
    "n_vp": 4,
}
