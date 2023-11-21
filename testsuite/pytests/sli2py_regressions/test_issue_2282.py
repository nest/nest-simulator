# -*- coding: utf-8 -*-
#
# test_issue_2282.py
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
Regression test for Issue #2282 (GitHub).

The test ensures that the `multimeter` cannot be connected to a
`noise_generator` if NEST runs with multiple threads.
"""

import nest
import numpy as np
import pytest

pytestmark = pytest.mark.skipif_missing_threads


def simulator(num_threads):
    """
    Simulator for the tests.

    The simulator sets up a system with a multimeter-recording from
    `noise_generator`, which is allowed in single-threaded mode but
    prohibited in multi-threaded mode.
    """

    nest.ResetKernel()
    nest.local_num_threads = num_threads

    nrns = nest.Create("iaf_psc_alpha", 3)
    noise = nest.Create("noise_generator", 3, {"mean": 1000.0, "std": 1.0, "dt": 0.1})
    mm = nest.Create("multimeter", {"record_from": ["I"], "interval": 0.1})

    nest.Connect(noise, nrns, "one_to_one", {"delay": 0.1})
    nest.Connect(mm, noise)

    nest.Simulate(2.0)

    recording = mm.events.get("I")

    return recording


def test_allowed_multimeter_record_noise_generator_singlethreaded():
    """
    Test that single-threaded mode works.

    The recording returned by `simulator` should only contain non-zero values.
    """

    recording = simulator(num_threads=1)
    assert np.all(recording > 0.0)


def test_prohibited_multimeter_record_noise_generator_multithreaded():
    """Test that an error is thrown in multi-threaded mode."""

    with pytest.raises(nest.kernel.NESTError):
        simulator(num_threads=2)
