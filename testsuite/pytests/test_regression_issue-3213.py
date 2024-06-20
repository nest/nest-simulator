# -*- coding: utf-8 -*-
#
# test_regression_issue-3213.py
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
Test that GetConnections works if NodeCollection with gaps is provided as source arg.
"""


def test_get_conns_works():
    """Main concern is that GetConnections() passes, expected number of connections based on all-to-all."""

    num_n = 12
    n = nest.Create("parrot_neuron", num_n)
    nest.Connect(n, n)
    pick = [3, 7, 9, 11]
    conns = nest.GetConnections(source=n[pick])
    assert len(conns) == num_n * len(pick)
