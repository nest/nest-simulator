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
"""

import pandas as pd
import pytest

import nest

pytestmark = pytest.mark.skipif_missing_threads


@pytest.fixture(autouse=True)
def set_resolution():
    nest.ResetKernel()
    nest.resolution = 0.1


def test_multithreaded():
    """
    test for correct creation of neuron-neuron, neuron-device, device-neuron
    and device-device connections using multiple threads.

    this script creates connections between neurons and devices and checks
    whether the actually created connections coincide with the expected
    connections

    connect with a mixture of neuron and devices
    """
    nest.set(total_num_virtual_procs=2)

    # Create nodes
    n1 = nest.Create("iaf_psc_delta")
    n2 = nest.Create("iaf_psc_delta")
    n3 = nest.Create("iaf_psc_delta")
    sg = nest.Create("spike_generator")
    sr = nest.Create("spike_recorder")
    vt = nest.Create("volume_transmitter")

    # Neuron-neuron
    nest.Connect(n1, n3)
    nest.Connect(n2, n3)
    nest.Connect(n2, n1)
    nest.Connect(n3, n1)
    nest.Connect(n2, n1, syn_spec={"weight": 1.0})

    # Neuron-device
    nest.Connect(n1, sr)
    nest.Connect(n1, n3)
    nest.Connect(n2, sr)
    nest.Connect(n3, sr)
    nest.Connect(n2, sr, syn_spec={"weight": 1.0})

    # Device-neuron
    nest.Connect(sg, n2)
    nest.Connect(sg, n3)
    nest.Connect(sg, n1)
    nest.Connect(sg, n1)
    nest.Connect(sg, n3, syn_spec={"weight": 1.0})

    # Device-device
    nest.Connect(sg, sr)
    nest.Connect(sg, sr)
    nest.Connect(sg, sr)
    nest.Connect(sg, sr, syn_spec={"weight": 1.0})

    # Neuron-globally receiving device (volume transmitter)
    nest.Connect(n1, vt)
    nest.Connect(n2, vt)

    conns = nest.GetConnections()
    print(pd.DataFrame.from_dict(conns.get()))

    conn1 = nest.GetConnections(n1, n3)
    conn_d = conn1.get(["source", "target", "target_thread"])
    print(conn_d.values())
    # print(pd.DataFrame.from_dict(conn1.get()))
    # print(conn1.get(["source", "target", "target_thread"]))
