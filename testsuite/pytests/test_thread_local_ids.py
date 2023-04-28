# -*- coding: utf-8 -*-
#
# test_thread_local_ids.py
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
Test that both connecting with an *_hpc synapse and simulating triggers the creation of thread-local neuron ids.
"""

import numpy as np
import pytest
import nest


@pytest.fixture(autouse=True)
def base_setup():
    """
    Set number of threads to T and create T neurons, i.e., one neuron per thread.
    The thread-local ids of all T neurons should be 0 as they are first neurons per thread.
    """

    T = 4  # number of threads
    nest.ResetKernel()
    nest.local_num_threads = T
    pytest.neurons = nest.Create("iaf_psc_alpha", T)


@pytest.mark.skipif_missing_threads
def test_thread_local_ids_after_connect():
    """
    Test that connecting with a static_synapse_hpc triggers the creation of thread-local ids (all 0).
    """

    nest.Connect(
        pytest.neurons, pytest.neurons, syn_spec={"synapse_model": "static_synapse_hpc"}
    )
    thread_local_ids = np.array(nest.GetStatus(pytest.neurons, ["thread_local_id"]))
    assert np.all(thread_local_ids.flatten() == 0)


@pytest.mark.skipif_missing_threads
def test_thread_local_ids_after_simulate():
    """
    Test that simulating with a static_synapse_hpc triggers the creation of thread-local ids (all 0).
    """

    nest.Simulate(1.0)
    thread_local_ids = np.array(nest.GetStatus(pytest.neurons, ["thread_local_id"]))
    assert np.all(thread_local_ids.flatten() == 0)
