# -*- coding: utf-8 -*-
#
# test_issue_1640.py
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
Regression test for Issue #1640 (GitHub).
"""

import nest
import numpy as np
import pytest


@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("num_threads", [2, 3, 4])
@pytest.mark.parametrize("num_neurons", range(1, 11))
def test_getconnections_with_pre_post_device_multithreaded(num_threads, num_neurons):
    """
    Multithreaded test of `GetConnections` with devices as source and target.

    The test ensures that `GetConnections` does not return erroneous connections
    when devices are connected as both source and target while using
    multiple threads.
    """

    nest.ResetKernel()
    nest.local_num_threads = num_threads

    neurons = nest.Create("iaf_psc_alpha", num_neurons)
    pgen = nest.Create("poisson_generator")
    srec = nest.Create("spike_recorder")

    # A poisson_generator is connected as source
    nest.Connect(pgen, neurons)
    # An additional device is connected as target
    nest.Connect(neurons, srec)

    # We only want the connections where the poisson_generator is source
    conns = nest.GetConnections(source=pgen)

    # Check that the poisson_generator is the source for all connections
    expected_src_id = pgen.global_id
    actual_src_ids = conns.get("source")
    actual_src_ids = np.atleast_1d(actual_src_ids)  # Ensure source ids are iterable
    assert np.all(actual_src_ids == expected_src_id)

    # Check that number of connections is correct
    expected_num_conns = num_neurons
    actual_num_conns = actual_src_ids.size
    assert actual_num_conns == expected_num_conns
