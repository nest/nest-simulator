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

import os
import nest
import pytest

skip_if_no_hdf5 = pytest.mark.skipif(not nest.ll_api.sli_func("statusdict/have_hdf5 ::"),
                                     reason="requires NEST built with HDF5 support")

root_path = os.path.dirname(__file__)
base_path = os.path.join(root_path, '300_pointneurons')
config = 'circuit_config.json'
sim_config = 'simulation_config.json'
EXPECTED_NUM_NODES = 400  # 300 'internal' nodes + 100 'external' nodes
EXPECTED_NUM_CONNECTIONS = 48432
EXPECTED_NUM_SPIKES = 18794
NUM_THREADS = [1, 2, 4]


@pytest.fixture
def reset():
    pytest.importorskip('h5py')  # Skip test if h5py is not found
    nest.ResetKernel()


@pytest.fixture
def sonata_connector_fixture():
    sonata_connector = nest.SonataConnector(base_path, config, sim_config)
    simtime = 0
    if 'tstop' in sonata_connector.config['run']:
        simtime = sonata_connector.config['run']['tstop']
    else:
        simtime = sonata_connector.config['run']['duration']
    return sonata_connector, simtime


@skip_if_no_hdf5
def testSonataConnector(reset, sonata_connector_fixture):
    """Correct positions used in Connect with free positions"""
    print('base_path:', base_path)
    sonata_connector, _ = sonata_connector_fixture

    assert(sonata_connector.config['target_simulator'] == 'NEST')


@skip_if_no_hdf5
@pytest.mark.parametrize("num_threads", NUM_THREADS)
def testCreate(reset, sonata_connector_fixture, num_threads):
    sonata_connector, _ = sonata_connector_fixture
    nest.set(resolution=sonata_connector.config['run']['dt'], overwrite_files=True, total_num_virtual_procs=num_threads)
    sonata_connector.Create()

    num_nodes = nest.GetKernelStatus('network_size')
    assert(num_nodes == EXPECTED_NUM_NODES)


@skip_if_no_hdf5
@pytest.mark.parametrize("num_threads", NUM_THREADS)
def testConnect(reset, sonata_connector_fixture, num_threads):
    sonata_connector, _ = sonata_connector_fixture
    nest.set(resolution=sonata_connector.config['run']['dt'], overwrite_files=True, total_num_virtual_procs=num_threads)
    sonata_connector.Create()
    sonata_connector.Connect()

    num_connections = nest.GetKernelStatus('num_connections')
    assert(num_connections == EXPECTED_NUM_CONNECTIONS)


@skip_if_no_hdf5
@pytest.mark.parametrize("num_threads", NUM_THREADS)
def testSimulate(reset, sonata_connector_fixture, num_threads):
    sonata_connector, simtime = sonata_connector_fixture
    nest.set(resolution=sonata_connector.config['run']['dt'], overwrite_files=True, total_num_virtual_procs=num_threads)
    sonata_connector.Create()
    sonata_connector.Connect()
    nest.Simulate(simtime)

    num_spikes = nest.GetKernelStatus('local_spike_counter')
    assert(num_spikes == EXPECTED_NUM_SPIKES)


@skip_if_no_hdf5
@pytest.mark.parametrize("num_threads", NUM_THREADS)
def testSonataNetwork(reset, sonata_connector_fixture, num_threads):
    """SonataNetwork convenience function"""
    sonata_connector, _ = sonata_connector_fixture
    nest.set(total_num_virtual_procs=num_threads)
    sonata_connector.CreateSonataNetwork(simulate=True)

    kernel_status = nest.GetKernelStatus()
    assert(kernel_status['network_size'] == EXPECTED_NUM_NODES)
    assert(kernel_status['num_connections'] == EXPECTED_NUM_CONNECTIONS)
    assert(kernel_status['local_spike_counter'] == EXPECTED_NUM_SPIKES)
