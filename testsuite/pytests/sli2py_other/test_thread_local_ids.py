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
Test that thread-local IDs are set correctly on multithreaded simulations.
"""

import nest
import pytest

pytestmark = pytest.mark.skipif_missing_threads


@pytest.fixture(autouse=True)
def set_kernel():
    nest.ResetKernel()
    nest.local_num_threads = 4


def test_thread_local_ids_after_connect():
    """Ensure thread-local IDs are still set correctly after ``Connect``."""

    nc = nest.Create("iaf_psc_alpha", 4)
    nest.Connect(
        nc,
        nc,
        conn_spec={"rule": "all_to_all"},
        syn_spec={"synapse_model": "static_synapse_hpc"},
    )

    assert nc.thread_local_id == (0, 0, 0, 0)


def test_thread_local_ids_after_simulate():
    """Ensure thread-local IDs are still set correctly after ``Simulate``."""

    nc = nest.Create("iaf_psc_alpha", 4)
    nest.Simulate(10.0)

    assert nc.thread_local_id == (0, 0, 0, 0)
