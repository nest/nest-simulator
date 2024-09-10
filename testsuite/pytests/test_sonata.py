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

from pathlib import Path

import nest
import pytest

# Skip all tests in this module if no HDF5 or OpenMP threads
pytestmark = [pytest.mark.skipif_missing_hdf5, pytest.mark.skipif_missing_threads]

# We consider two possible cases:
# - When running via `make installcheck`, this file is in $INSTALLDIR/share/nest/testsuite/pytests,
#   while the data is in $INSTALLDIR/share/doc/nest/examples/pynest/sonata_example.
# - When running from the source dir, this file is in $SOURCEDIR/testsuite/pytests,
#   while the data is in $SOURCEDIR/pynest/examples/sonata_example.
for relpath in ["../../../doc/nest/examples/pynest", "../../pynest/examples"]:
    sonata_path = Path(__file__).parent / relpath / "sonata_example" / "300_pointneurons"
    config = sonata_path / "circuit_config.json"
    sim_config = sonata_path / "simulation_config.json"
    have_sonata_files = config.is_file() and sim_config.is_file()
    if have_sonata_files:
        break
else:
    have_sonata_files = False


EXPECTED_NUM_NODES = 400  # 300 'internal' + 100 'external'
EXPECTED_NUM_CONNECTIONS = 48432
EXPECTED_NUM_SPIKES = 18828

# Meaning of hyperslab sizes for 300_pointneurons model:
# 2**10=1024 : Edge HDF5 files will be read in hyperslabs (chunks)
# 2**20=1048576 : Edge files read in their entirety (default hyperslab size value)
HYPERSLAB_SIZES = [2**10, 2**20]
NUM_THREADS = [1, 2, 4]


@pytest.mark.parametrize("hyperslab_size", HYPERSLAB_SIZES)
@pytest.mark.parametrize("num_threads", NUM_THREADS)
def test_SonataNetwork(num_threads, hyperslab_size):
    # Tests must fail if input files not found, since that points to a
    # misconfiguration of the NEST installation.
    assert have_sonata_files, "SONATA files not found"

    nest.ResetKernel()
    nest.set(total_num_virtual_procs=num_threads)
    sonata_net = nest.SonataNetwork(config, sim_config)
    node_collections = sonata_net.BuildNetwork(hdf5_hyperslab_size=hyperslab_size)

    # Verify network was built correctly
    kernel_status = nest.GetKernelStatus()
    assert kernel_status["network_size"] == EXPECTED_NUM_NODES
    assert kernel_status["num_connections"] == EXPECTED_NUM_CONNECTIONS

    # Verify network dynamics
    srec = nest.Create("spike_recorder")
    nest.Connect(node_collections["internal"], srec)
    sonata_net.Simulate()
    spike_data = srec.events
    post_times = spike_data["times"]
    assert post_times.size == EXPECTED_NUM_SPIKES
