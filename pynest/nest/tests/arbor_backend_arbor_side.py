# -*- coding: utf-8 -*-
#
# arbor_backend_arbor_side.py
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

# This script simulates an Arbor run with MPI spike exchange to external
# NEST. It saves the received spikes to file for testing.
# Based on arbor_proxy.py in arbor_cosim_example.
from mpi4py import MPI
import numpy as np
import math

############################################################################
# Some helper functions

# for debug printing in MPI environment
print_debug = True
print_prefix = "ARB_PROXY_PY: "


def gather_spikes(spikes, comm):
    # We need to know how much data we will receive in this gather action
    size = comm.size                                 #
    receive_count_array = np.zeros(size, dtype='uint32')
    send_count_array = np.array([spikes.size], dtype='uint32')
    comm.Allgather(send_count_array, receive_count_array)

    # Calculate the amount of spikes
    cummulative_sum_spikes = np.cumsum(receive_count_array)
    offsets = np.zeros(size)
    # start with a zero and skip the last entry in cumsum
    offsets[1:] = cummulative_sum_spikes[:-1]

    # Create buffers for sending and receiving
    # Total nr spikes received is the last entry in cumsum
    # Allgatherv only available as raw byte buffers
    receive_spikes = np.ones(cummulative_sum_spikes[-1], dtype='byte')
    send_buffer = spikes.view(dtype=np.byte)  # send as a byte view in spikes
    receive_buffer = [receive_spikes, receive_count_array, offsets, MPI.BYTE]

    comm.Allgatherv(send_buffer, receive_buffer)
    # print_spike_array_d(receive_spikes.view('uint32,uint32, float32'))

    return receive_spikes.view('uint32,uint32, float32')


class comm_information():
    # Helper class for MPI configuration
    # TODO: with N>2 simulators self whole function needs to be cleaned up
    def __init__(self, is_arbor):
        self.is_arbor = is_arbor
        self.is_nest = not is_arbor

        self.global_rank = MPI.COMM_WORLD.rank
        self.global_size = MPI.COMM_WORLD.size

        # all arbor go into split 1
        color = 1 if is_arbor else 0
        self.world = MPI.COMM_WORLD
        self.comm = self.world.Split(color)

        local_size = self.comm.size
        self.local_rank = self.comm.rank

        self.arbor_size = local_size if self.is_arbor else self.global_size - local_size  # noqa
        self.nest_size = self.global_size - self.arbor_size

        input = np.array([self.global_rank], dtype=np.int32)
        local_ranks = np.zeros(local_size, dtype=np.int32)

        # Grab all local ranks. Sort find the first non consecutive.
        # This will be the other root
        self.comm.Allgather(input, local_ranks)
        local_ranks.sort()

        # Small helper function to look for first non consecutive entry.
        # Ranks can interleaved, the first non consecutive would be nest

        def first_missing(np_array):
            for idx in range(np_array.size-1):
                if not (np_array[idx+1] - np_array[idx] == 1):
                    return np_array[idx] + 1
            # Default the last rank plus one
            return np_array[-1]+1

        if (self.is_arbor):
            self.arbor_root = local_ranks[0]
            self.nest_root = first_missing(local_ranks) if self.arbor_root == 0 else 0  # noqa
        else:
            self.nest_root = local_ranks[0]
            self.arbor_root = first_missing(local_ranks) if self.nest_root == 0 else 0  # noqa

    def __str__(self):
        return str("global ( rank: " + str(self.global_rank) + ", size: " + str(self.global_size) + "\n" +  # noqa
                   "local rank " + str(self.local_rank) + "\n" +
                   "self is arbor\n" if self.is_arbor else "self is nest\n" +
                   "arbor (root: " + str(self.arbor_root) + ", size: " + str(self.arbor_size) + ")\n" +  # noqa
                   "nest (root: " + str(self.nest_root) + ", size: " + str(self.nest_size) + ")\n")  # noqa


#####################################################################
# MPI configuration
comm_info = comm_information(True)
# Only print one the root arbor rank
if comm_info.local_rank != comm_info.arbor_root:
    print_debug = False

# Sim Config
num_arbor_cells = 100
min_delay = 10
duration = 100

########################################################################
# handshake #1: communicate the number of cells between arbor and nest
# send nr of arbor cells
output = np.array([num_arbor_cells], dtype=np.int32)
comm_info.world.Bcast(output, comm_info.arbor_root)

# Receive nest cell_nr
output = np.array([0], dtype=np.int32)
comm_info.world.Bcast(output, root=comm_info.nest_root)

num_nest_cells = output[0]
num_total_cells = num_nest_cells + num_arbor_cells

print("num_arbor_cells: " + str(num_arbor_cells) + " " +
      "num_nest_cells: " + str(num_nest_cells) + " " +
      "num_total_cells: " + str(num_total_cells))

########################################################################
# hand shake #2: min delay
# first send the arbor delays
arb_com_time = min_delay / 2.0
output = np.array([arb_com_time], dtype=np.float32)
comm_info.world.Bcast(output, comm_info.arbor_root)

# receive the nest delays
output = np.array([0], dtype=np.float32)
comm_info.world.Bcast(output, comm_info.nest_root)
nest_com_time = output[0]
print("nest_com_time: " + str(nest_com_time))

###############################################################
# Process the delay and calculate new simulator settings
# TODO: This doubling smells cludgy
double_min_delay = 2 * min(arb_com_time, nest_com_time)
print("min_delay: " + str(double_min_delay))
delta = double_min_delay / 2.0
steps = int(math.floor(duration / delta))

# Extra step at end if not a whole multiple
if (steps * delta < duration):
    steps += 1

###############################################################
# Handshake #3: steps
output = np.array([steps], dtype=np.int32)
comm_info.world.Bcast(output, comm_info.arbor_root)

print("delta: " + str(delta) + ", " +
      "sim_duration: " + str(duration) + ", " +
      "steps: " + str(steps) + ", ")

#######################################################
# main simulated simulation loop inclusive nr of steps.
received_spikes = []
for step in range(steps+1):
    print("step: " + str(step) + ": " + str(step * delta))

    # We are sending no spikes from arbor to nest.
    # Create a array with size zero with correct type
    output = np.zeros(0, dtype='uint32, uint32, float32')
    received_spikes += [[node, t] for node, _, t in gather_spikes(output, comm_info.world)]

np.savetxt('arbor_spikes.dat', received_spikes, ['%d', '%.1f'])

print("Reached arbor_proxy.py end")
