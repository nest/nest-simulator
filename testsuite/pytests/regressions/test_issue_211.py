# -*- coding: utf-8 -*-
#
# test_issue_211.py
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

"""
Regression test for Issue #211 (GitHub).

This set of tests create connections between neurons and devices using multiple
threads and check whether the actually created connections coincide with the
expected connections.
"""

import nest
import pandas as pd
import pandas.testing as pdtest
import pytest

pytestmark = pytest.mark.skipif_missing_threads


@pytest.fixture(autouse=True)
def set_kernel():
    nest.ResetKernel()
    nest.local_num_threads = 2


def get_actual_connections():
    """
    Retrieve actual connections as a sorted `pandas.DataFrame`.

    This is a helper function used by the subsequent tests. The connections
    created by the kernel must be sorted in order for deterministic comparison
    with expectations. The connections retrieved from `GetConnections` are
    sorted by, in decreasing order of priority, the target thread id, source
    node id and target node id.
    """

    syn_collection = nest.GetConnections()
    actual_connections = pd.DataFrame.from_dict(syn_collection.get(["source", "target", "target_thread"]))

    actual_connections.sort_values(by=["target_thread", "source", "target"], ignore_index=True, inplace=True)

    return actual_connections


def test_connect_neuron_neuron_multithreaded():
    """
    Ensure correct creation of neuron-neuron connections using multiple threads.
    """

    n1 = nest.Create("iaf_psc_delta")
    n2 = nest.Create("iaf_psc_delta")
    n3 = nest.Create("iaf_psc_delta")

    nest.Connect(n1, n3)
    nest.Connect(n2, n3)
    nest.Connect(n2, n1)
    nest.Connect(n3, n1)
    nest.Connect(n2, n1, syn_spec={"weight": 1.0})

    expected_conns = pd.DataFrame(
        [[1, 3, 1], [2, 1, 1], [2, 1, 1], [2, 3, 1], [3, 1, 1]],
        columns=["source", "target", "target_thread"],
    )
    actual_conns = get_actual_connections()

    pdtest.assert_frame_equal(actual_conns, expected_conns)


def test_connect_neuron_device_multithreaded():
    """
    Ensure correct creation of neuron-device connections using multiple threads.
    """

    n1 = nest.Create("iaf_psc_delta")
    n2 = nest.Create("iaf_psc_delta")
    n3 = nest.Create("iaf_psc_delta")
    sr = nest.Create("spike_recorder")

    nest.Connect(n1, sr)
    nest.Connect(n2, sr)
    nest.Connect(n3, sr)

    expected_conns = pd.DataFrame(
        [[2, 4, 0], [1, 4, 1], [3, 4, 1]],
        columns=["source", "target", "target_thread"],
    )
    actual_conns = get_actual_connections()

    pdtest.assert_frame_equal(actual_conns, expected_conns)


def test_connect_device_neuron_multithreaded():
    """
    Ensure correct creation of device-neuron connections using multiple threads.
    """

    n1 = nest.Create("iaf_psc_delta")
    n2 = nest.Create("iaf_psc_delta")
    n3 = nest.Create("iaf_psc_delta")
    sg = nest.Create("spike_generator")

    nest.Connect(sg, n1)
    nest.Connect(sg, n2)
    nest.Connect(sg, n3)

    expected_conns = pd.DataFrame(
        [[4, 2, 0], [4, 1, 1], [4, 3, 1]],
        columns=["source", "target", "target_thread"],
    )
    actual_conns = get_actual_connections()

    pdtest.assert_frame_equal(actual_conns, expected_conns)


def test_connect_device_device_multithreaded():
    """
    Ensure correct creation of device-device connections using multiple threads.
    """

    sr1 = nest.Create("spike_recorder")
    sr2 = nest.Create("spike_recorder")
    sg = nest.Create("spike_generator")

    nest.Connect(sg, sr1)
    nest.Connect(sg, sr2)

    expected_conns = pd.DataFrame(
        [[3, 2, 0], [3, 1, 1]],
        columns=["source", "target", "target_thread"],
    )
    actual_conns = get_actual_connections()

    pdtest.assert_frame_equal(actual_conns, expected_conns)


def test_connect_neuron_global_device_multithreaded():
    """
    Ensure correct creation of neuron-global device connections using multiple threads.
    """

    n1 = nest.Create("iaf_psc_delta")
    n2 = nest.Create("iaf_psc_delta")
    vt = nest.Create("volume_transmitter")

    nest.Connect(n1, vt)
    nest.Connect(n2, vt)

    expected_conns = pd.DataFrame(
        [[1, 3, 0], [2, 3, 0], [1, 3, 1], [2, 3, 1]],
        columns=["source", "target", "target_thread"],
    )
    actual_conns = get_actual_connections()

    pdtest.assert_frame_equal(actual_conns, expected_conns)
