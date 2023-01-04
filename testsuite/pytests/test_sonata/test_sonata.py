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
#from pathlib import Path
import os

skip_if_no_hdf5 = pytest.mark.skipif(not nest.ll_api.sli_func("statusdict/have_hdf5 ::"),
                                     reason="requires NEST built with HDF5 support")


root_path = os.path.dirname(__file__)
sonata_path = os.path.join(root_path, "300_pointneurons")
config = os.path.join(sonata_path, "circuit_config.json")
sim_config = os.path.join(sonata_path, "simulation_config.json")

''' 
root_path = Path(__file__).resolve().parent
sonata_path = root_path.joinpath("300_pointneurons")
config = sonata_path.joinpath("circuit_config.json")
sim_config = sonata_path.joinpath("simulation_config.json")
'''

EXPECTED_NUM_NODES = 400  # 300 'internal' nodes + 100 'external' nodes
EXPECTED_NUM_CONNECTIONS = 48432
EXPECTED_NUM_SPIKES = 18794
NUM_THREADS = [1, 2, 4]

# TODO:
# make more concise tests
# use 300_pointneurons from pynest/examples dir
# (300_pointneurons in testsuite is also with typo in hdf5 dsets)
# test sending the config paths as str, pathlib.Path and pathlib.PurePath
# test setting of chunk size


@pytest.fixture
def reset():
    pytest.importorskip('h5py')  # Skip test if h5py is not found
    nest.ResetKernel()


@pytest.fixture
def sonata_network_fixture():
    sonata_net = nest.SonataNetwork(config, sim_config)
    return sonata_net


@skip_if_no_hdf5
def testSonataConnector(reset, sonata_network_fixture):
    """Correct positions used in Connect with free positions"""
    sonata_net = sonata_network_fixture
    assert (sonata_net.config['target_simulator'] == 'NEST')


@skip_if_no_hdf5
@pytest.mark.parametrize("num_threads", NUM_THREADS)
def testCreate(reset, sonata_network_fixture, num_threads):
    sonata_net = sonata_network_fixture
    nest.set(overwrite_files=True, total_num_virtual_procs=num_threads)
    sonata_net.Create()

    num_nodes = nest.GetKernelStatus('network_size')
    assert (num_nodes == EXPECTED_NUM_NODES)


@skip_if_no_hdf5
@pytest.mark.parametrize("num_threads", NUM_THREADS)
def testConnect(reset, sonata_network_fixture, num_threads):
    sonata_net = sonata_network_fixture
    nest.set(overwrite_files=True, total_num_virtual_procs=num_threads)
    sonata_net.Create()
    sonata_net.Connect()

    num_connections = nest.GetKernelStatus('num_connections')
    assert (num_connections == EXPECTED_NUM_CONNECTIONS)


@skip_if_no_hdf5
@pytest.mark.parametrize("num_threads", NUM_THREADS)
def testSimulate(reset, sonata_network_fixture, num_threads):
    sonata_net = sonata_network_fixture
    nest.set(overwrite_files=True, total_num_virtual_procs=num_threads)
    sonata_net.BuildNetwork()
    sonata_net.Simulate()

    kernel_status = nest.GetKernelStatus()
    assert (kernel_status['network_size'] == EXPECTED_NUM_NODES)
    assert (kernel_status['num_connections'] == EXPECTED_NUM_CONNECTIONS)
    assert (kernel_status['local_spike_counter'] == EXPECTED_NUM_SPIKES)
