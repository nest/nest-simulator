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

'''
microcircuit simulation parameters
----------------------------------

Simulation parameters for the microcircuit.

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016
'''

import os
sim_dict = {
    # Simulation time (in ms).
    't_sim': 1000.0,
    # Resolution of the simulation (in ms).
    'sim_resolution': 0.1,
    # Path to save the output data.
    'data_path': os.path.join(os.getcwd(), 'data/'),
    # Masterseed for NEST and NumPy.
    'master_seed': 55,
    # Number of threads per MPI process.
    'local_num_threads': 1,
    # Recording interval of the membrane potential (in ms).
    'rec_V_int': 1.0,
    # If True, data will be overwritten,
    # If False, a NESTError is raised if the files already exist.
    'overwrite_files': True,
    # Print the time progress, this should only be used when the simulation
    # is run on a local machine.
    'print_time': True
    }
