# -*- coding: utf-8 -*-
#
# test_ticket_157.py
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


def test_ticket_157():
    """
    Test to verify the behavior of the simulation with a specific resolution.

    This test sets up a simple network with an iaf_psc_alpha neuron and a
    poisson_generator_ps. It simulates the network for a specified duration
    to ensure that the simulation runs without assertion failures.

    Previously, this script caused a C++ assertion to fail for h==0.1, but not
    for h==0.01 or h==0.001. This test ensures that the issue is resolved.
    """

    h = 0.1
    T = 3.0

    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": h})

    n = nest.Create("iaf_psc_alpha")
    p = nest.Create("poisson_generator_ps", params={"rate": 10000.0})

    nest.Connect(p, n)

    nest.Simulate(T)

    # If the simulation completes without exceptions, the test passes
