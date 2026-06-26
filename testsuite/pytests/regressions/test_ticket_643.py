# -*- coding: utf-8 -*-
#
# test_ticket_643.py
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

import nest
import pytest

"""
Regression test for Ticket #643.

Test ported from SLI regression test.
Ensure ring buffers allocate correctly under many threads when receiving noise.
"""


@pytest.mark.skipif_missing_threads
def test_ticket_643_noise_generator_under_many_threads():
    """
    Ensure multithreaded simulation with a noise generator does not crash.
    """

    nest.ResetKernel()
    nest.local_num_threads = 256

    neuron = nest.Create("iaf_psc_alpha")
    generator = nest.Create("noise_generator", params={"mean": 200.0, "std": 100.0, "dt": 1.0})

    nest.Connect(generator, neuron)

    nest.Simulate(1.0)
