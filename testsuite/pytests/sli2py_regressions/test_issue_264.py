# -*- coding: utf-8 -*-
#
# test_issue_264.py
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
Regression test for Issue #264 (GitHub).

Ensure error is raised on times incommensurate with resolution.

.. note::
    To ensure that the resolution 1.0 and test value 1.5 can be represented
    exactly in tics, we set `tics_per_ms` below to the usual default. This
    handles the unusual case where NEST is compiled with a different value.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def set_resolution():
    nest.ResetKernel()
    nest.set(resolution=1.0, tics_per_ms=1000.0)


def test_incommensurate_simulation_time():
    """
    Ensure error is raised on simulation time incommensurate with resolution.

    This test ensures that NEST raises an error if the requested simulation
    time is not a multiple of the resolution.
    """

    with pytest.raises(nest.kernel.NESTErrors.BadParameter):
        nest.Simulate(1.5)


@pytest.mark.parametrize("item", ["start", "stop", "origin"])
@pytest.mark.parametrize("device", ["spike_generator", "spike_recorder"])
def test_incommensurate_resolution_on_set_defaults(device, item):
    """
    Ensure error is raised on `SetDefaults` options incommensurate with resolution.

    This test ensures that NEST raises an error if one attempts to set
    start/stop/origin for devices that are not multiples of the resolution.
    """

    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        nest.SetDefaults(device, {item: 1.5})


@pytest.mark.parametrize("item", ["start", "stop", "origin"])
@pytest.mark.parametrize("device", ["spike_generator", "spike_recorder"])
def test_incommensurate_resolution_on_device_set(device, item):
    """
    Ensure error is raised on device `set` options incommensurate with resolution.

    This test ensures that NEST raises an error if one attempts to set
    start/stop/origin for devices that are not multiples of the resolution.
    """

    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        d = nest.Create(device)
        d.set({item: 1.5})
