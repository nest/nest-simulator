# -*- coding: utf-8 -*-
#
# test_ticket_514.py
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

"""
Regression test for Ticket #514.

Test ported from SLI regression test.
Ensure that poisson_generator_ps can drive more than 128 targets.
"""


def test_ticket_514_poisson_generator_ps_connects_many_targets():
    """
    Ensure poisson_generator_ps connects to 1000 targets without errors.
    """

    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": 0.1})

    pg = nest.Create("poisson_generator_ps")
    nest.SetStatus(pg, {"rate": 1000.0})

    neurons = nest.Create("iaf_psc_delta", n=1000)

    nest.Connect(pg, neurons)
    nest.Simulate(1000.0)

    connections = nest.GetConnections(source=pg, target=neurons)

    assert len(connections) == len(neurons)
