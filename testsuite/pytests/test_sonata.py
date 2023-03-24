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

# Skip all tests in this module if no HDF5
have_hdf5 = nest.ll_api.sli_func("statusdict/have_hdf5 ::")
pytestmark = pytest.mark.skipif(not have_hdf5, reason="Requires NEST built with HDF5 support")

# We consider two possible cases:
# - When running via `make installcheck`, this file is in $INSTALLDIR/share/nest/testsuite/pytests,
#   while the data is in $INSTALLDIR/share/doc/nest/examples/pynest/sonata_example.
# - When running from the source dir, this file is in $SOURCEDIR/testsuite/pytests,
#   while the data is in $SOURCEDIR/pynest/examples/sonata_example.
for relpath in ['../../../doc/nest/examples/pynest', '../../pynest/examples']:
    sonata_path = Path(__file__).parent / relpath / 'sonata_example' / '300_pointneurons'
    config = sonata_path / 'circuit_config.json'
    sim_config = sonata_path / 'simulation_config.json'
    have_sonata_files = config.is_file() and sim_config.is_file()
    if have_sonata_files:
        break
else:
    have_sonata_files = False


EXPECTED_NUM_NODES = 401  # 300 'internal' + 100 'external' + 1 device
EXPECTED_NUM_CONNECTIONS = 48432
EXPECTED_NUM_SPIKES = 18828

# Meaning of chunk sizes for 300_pointneurons model:
# 2**10=1024 : Edge HDF5 files will be read in chunks
# 2**20=1048576 : Edge files read in their entirety (default chunk size value)
CHUNK_SIZES = [2**10, 2**20]
NUM_THREADS = [1, 2, 4]


@pytest.mark.parametrize("chunk_size", CHUNK_SIZES)
@pytest.mark.parametrize("num_threads", NUM_THREADS)
def testSonataNetwork(num_threads, chunk_size):
    # Tests must fail if input files not found, since that points to a misconfiguration of the NEST installation.
    assert have_sonata_files, f"SONATA input files not found {testhist}"
    nest.ResetKernel()
    nest.set(total_num_virtual_procs=num_threads)
    sonata_net = nest.SonataNetwork(config, sim_config)
    node_collections = sonata_net.BuildNetwork(chunk_size=chunk_size)
    srec = nest.Create("spike_recorder")
    nest.Connect(node_collections["internal"], srec)
    sonata_net.Simulate()
    spike_data = srec.events
    post_times = spike_data['times']
    kernel_status = nest.GetKernelStatus()
    assert kernel_status['network_size'] == EXPECTED_NUM_NODES
    assert kernel_status['num_connections'] == EXPECTED_NUM_CONNECTIONS
    assert post_times.size == EXPECTED_NUM_SPIKES
