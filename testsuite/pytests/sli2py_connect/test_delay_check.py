# -*- coding: utf-8 -*-
#
# test_delay_check.py
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
Test of delay

This script creates a number of synapses and tests, whether min_delay and max_delay
are set as expected. It will report for each test separately, whether it was passed or failed

"""

import nest
import pytest


@pytest.fixture(autouse=True)
def base_setup():
    """
    Reset kernel and create two neurons, connected by a static synapse with delay 2.0ms.
    """
    nest.ResetKernel()
    pytest.n1, pytest.n2 = nest.Create("iaf_psc_alpha", 2)
    nest.SetDefaults("static_synapse", {"delay": 2.0})
    nest.Connect(pytest.n1, pytest.n2)


def test_standard_connect():
    """
    Test base setup.
    """
    assert nest.min_delay == 2.0 and nest.max_delay == 2.0


def test_candidate_not_reported():
    """
    Test that max_delay is not changed without connecting.
    """
    nest.SetDefaults("static_synapse", {"delay": 10.0})
    assert nest.min_delay == 2.0 and nest.max_delay == 2.0


def test_min_delay():
    """
    Test that min_delay is changed after connecting with static synapse of delay 1.0ms.
    """
    nest.SetDefaults("static_synapse", {"delay": 1.0})
    nest.Connect(pytest.n2, pytest.n1)
    assert nest.min_delay == 1.0 and nest.max_delay == 2.0


def test_connect():
    """
    Test that max_delay is changed when connecting and providing the delay in the synapse specification dictionary.
    """
    nest.Connect(pytest.n2, pytest.n1, syn_spec={"weight": 1.0, "delay": 6.0})
    assert nest.min_delay == 2.0 and nest.max_delay == 6.0


def test_setstatus_min_delay():
    """
    Test that min_delay is changed after setting the status of the synapse.
    """
    conn = nest.GetConnections(source=pytest.n1, target=pytest.n2, synapse_model="static_synapse")
    nest.SetStatus(conn, {"delay": 0.1})
    assert nest.min_delay == 0.1 and nest.max_delay == 2.0
