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

import pytest

def test_ticket_514():
    """
    Ensure that poisson_generator_ps can be connected to 1000 neurons.
    """
    import nest

    pg = nest.Create("poisson_generator_ps")
    neurons = nest.Create("iaf_psc_delta", n=1000)

    nest.Connect(pg, neurons)

    connections = nest.GetConnections(source=pg, target=neurons)

    assert len(connections) == len(neurons)
