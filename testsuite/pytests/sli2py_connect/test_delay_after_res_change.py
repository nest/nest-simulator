# -*- coding: utf-8 -*-
#
# test_delay_after_res_change.py
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
Test delays after resolution change.

This script checks that the min and max delays and the default delays
are still correct after the resolution has changed.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset_kernel():
    """
    Reset kernel to clear reset delay and resolution parameters.
    """

    nest.ResetKernel()


def test_change_to_high_resolution():
    """
    Test that min delay, max delay and default delay are not changed
    when increasing the resolution.
    """

    nest.SetKernelStatus({"resolution": 0.1, "min_delay": 2.1, "max_delay": 12.3})
    nest.SetKernelStatus({"resolution": 0.001})

    nest.SetDefaults("static_synapse", {"delay": 3.5})

    assert nest.min_delay == 2.1 and nest.max_delay == 12.3
    assert nest.GetDefaults("static_synapse")["delay"] == 3.5


def test_change_to_low_resolution():
    """
    Test that min delay, max delay and default delay are rounded accordingly
    when decreasing the resolution.
    Note: min delay is always rounded down, max delay is always rounded up.
    """

    nest.SetKernelStatus({"resolution": 0.1, "min_delay": 2.1, "max_delay": 12.3})
    nest.resolution = 1.0

    nest.SetDefaults("static_synapse", {"delay": 3.5})

    assert nest.min_delay == 2.0 and nest.max_delay == 13.0
    assert nest.GetDefaults("static_synapse")["delay"] == 4.0


def test_change_tics_per_ms():
    """
    Test that min delay, max delay and default delay are not changed
    when setting tics_per_ms to 10000.
    """

    nest.SetKernelStatus({"tics_per_ms": 10000, "resolution": 0.001, "min_delay": 2.1, "max_delay": 12.3})

    nest.SetDefaults("static_synapse", {"delay": 3.5})

    assert nest.min_delay == 2.1 and nest.max_delay == 12.3
    assert nest.GetDefaults("static_synapse")["delay"] == 3.5
