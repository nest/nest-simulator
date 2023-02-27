# -*- coding: utf-8 -*-
#
# test_sonata.py
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
import pytest
from pathlib import Path

skip_if_no_hdf5 = pytest.mark.skipif(not nest.ll_api.sli_func("statusdict/have_hdf5 ::"),
                                     reason="requires NEST built with HDF5 support")

root_path = Path(__file__).resolve().parent
sonata_path = root_path.joinpath("300_pointneurons")
config = sonata_path.joinpath("circuit_config.json")
sim_config = sonata_path.joinpath("simulation_config.json")


EXPECTED_NUM_NODES = 400  # 300 'internal' nodes + 100 'external' nodes
EXPECTED_NUM_CONNECTIONS = 48432
EXPECTED_NUM_SPIKES = 18794

# Meaning of chunk sizes for 300_pointneurons model:
# 2**10=1024 : Edge HDF5 files will be read in chunks
# 2**20=1048576 : Edge files read in their entirety (default chunk size value)
CHUNK_SIZES = [2**10, 2**20]
NUM_THREADS = [1, 2, 4]


@pytest.fixture
def reset():
    pytest.importorskip('h5py')  # Skip test if h5py is not found
    nest.ResetKernel()


@skip_if_no_hdf5
@pytest.mark.parametrize("chunk_size", CHUNK_SIZES)
@pytest.mark.parametrize("num_threads", NUM_THREADS)
def testSonataNetwork(reset, num_threads, chunk_size):
    nest.set(total_num_virtual_procs=num_threads)
    sonata_net = nest.SonataNetwork(config, sim_config)
    sonata_net.BuildNetwork(chunk_size=chunk_size)
    sonata_net.Simulate()
    kernel_status = nest.GetKernelStatus()
    assert kernel_status['network_size'] == EXPECTED_NUM_NODES
    assert kernel_status['num_connections'] == EXPECTED_NUM_CONNECTIONS
    assert kernel_status['local_spike_counter'] == EXPECTED_NUM_SPIKES
