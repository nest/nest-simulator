# -*- coding: utf-8 -*-
#
# test_issue_105.py
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
Regression test for Issue #105 (GitHub).
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def set_resolution():
    nest.ResetKernel()
    nest.resolution = 0.1


@pytest.mark.parametrize("delay", [0.81, 0.82, 0.8499])
def test_min_delay_on_fresh_connection(delay):
    """
    Ensure that NEST sets `min_delay` correctly from delay in `syn_spec`.

    This test ensures that `min_delay` is set correctly for delays
    that are not multiples of the resolution.
    """

    nrn = nest.Create("iaf_psc_alpha")
    conn = nest.Connect(nrn, nrn, "one_to_one", {"delay": delay}, return_synapsecollection=True)

    expected_delay = 0.8
    assert conn.delay == expected_delay
    assert nest.min_delay == expected_delay


@pytest.mark.parametrize("delay", [0.81, 0.82, 0.8499])
def test_min_delay_on_synapsecollection_set(delay):
    """
    Ensure that NEST sets `min_delay` correctly on `SynapseCollection` set.

    This test ensures that `min_delay` is set correctly for delays
    that are not multiples of the resolution.
    """

    nrn = nest.Create("iaf_psc_alpha")
    conn = nest.Connect(nrn, nrn, "one_to_one", return_synapsecollection=True)
    conn.delay = delay

    expected_delay = 0.8
    assert conn.delay == expected_delay
    assert nest.min_delay == expected_delay


@pytest.mark.parametrize("delay", [0.85, 0.86, 0.8999])
def test_max_delay_on_fresh_connection(delay):
    """
    Ensure that NEST sets `max_delay` correctly from delay in `syn_spec`.

    This test ensures that `max_delay` is set correctly for delays
    that are not multiples of the resolution.
    """

    nrn = nest.Create("iaf_psc_alpha")
    conn = nest.Connect(nrn, nrn, "one_to_one", {"delay": delay}, return_synapsecollection=True)

    expected_delay = 0.9
    assert conn.delay == expected_delay
    assert nest.max_delay == expected_delay


@pytest.mark.parametrize("delay", [1.95, 1.96, 1.999])
def test_max_delay_on_synapsecollection_set(delay):
    """
    Ensure that NEST sets `max_delay` correctly on `SynapseCollection` set.

    This test ensures that `max_delay` is set correctly for delays
    that are not multiples of the resolution.
    """

    nrn = nest.Create("iaf_psc_alpha")
    conn = nest.Connect(nrn, nrn, "one_to_one", return_synapsecollection=True)
    conn.delay = delay

    expected_delay = 2.0
    assert conn.delay == expected_delay
    assert nest.max_delay == expected_delay


@pytest.mark.parametrize("min_delay", [0.81, 0.89])
def test_min_and_max_delay_on_set_kernel_status(min_delay):
    """
    Test setting `min_delay` and `max_delay` on kernel set.

    Test that setting `min_delay` and `max_delay` works so that setting certain
    min/max values will allow those values to be used in subsequent connections.
    Note that effects can only be seen in the kernel status after a connection
    has been created.

    We expect that both `min_delay` values are rounded to 0.8 and the
    `max_delay` value is rounded to 1.5.
    """

    max_delay = 1.49

    nest.set(min_delay=min_delay, max_delay=max_delay)

    nrn = nest.Create("iaf_psc_alpha")
    nest.Connect(nrn, nrn, "one_to_one", {"delay": min_delay})
    nest.Connect(nrn, nrn, "one_to_one", {"delay": max_delay})

    expected_min_delay = 0.8
    expected_max_delay = 1.5
    assert nest.min_delay == expected_min_delay
    assert nest.max_delay == expected_max_delay
