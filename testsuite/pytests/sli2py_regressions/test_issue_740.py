# -*- coding: utf-8 -*-
#
# test_issue_740.py
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

import warnings

import nest
import pytest

"""
Regression test for Issue #740 (GitHub).

Test ported from SLI regression test.
Ensure step_current_generator enforces strictly increasing amplitude_times after conversion to steps.

Author: Hans Ekkehard Plesser, August 2017
"""


AMPLITUDE_TIMES = [1.0, 1.0625, 1.125, 1.1875, 1.25, 2.0, 2.5]
AMPLITUDE_VALUES = [10.0, -20.0, 40.0, -80.0, 160.0, -320.0, 640.0]


def _simulate(resolution: float, allow_offgrid: bool) -> None:
    nest.ResetKernel()
    nest.set(tics_per_ms=1024, resolution=resolution)

    scg = nest.Create(
        "step_current_generator",
        params={
            "amplitude_times": AMPLITUDE_TIMES,
            "amplitude_values": AMPLITUDE_VALUES,
            "allow_offgrid_times": allow_offgrid,
        },
    )

    neuron = nest.Create(
        "iaf_psc_alpha",
        params={
            "V_th": 1e10,
            "C_m": 1.0,
            "tau_m": 1.0,
            "E_L": 0.0,
            "V_m": 0.0,
            "V_reset": 0.0,
        },
    )

    nest.Connect(scg, neuron)

    steps = int(5.0 / resolution)
    sim_time = steps * resolution
    sim_time = max(sim_time, resolution)

    nest.Simulate(sim_time)


@pytest.mark.parametrize(
    ("resolution", "allow_offgrid", "should_raise"),
    (
        (2.0**-5, False, False),
        (2.0**-10 * 3.0, False, True),
        (2.0**-10 * 3.0, True, False),
        (2.0**-4, False, False),
        (2.0**-3, False, True),
        (2.0**-3, True, True),
    ),
)
def test_issue_740_step_current_generator_time_checks(resolution, allow_offgrid, should_raise):
    """
    Ensure step_current_generator accepts only configurations with strictly increasing step times.
    """

    if should_raise:
        with pytest.raises(nest.NESTErrors.BadProperty, match="(increasing|representable)"):
            _simulate(resolution, allow_offgrid)
    else:
        _simulate(resolution, allow_offgrid)
