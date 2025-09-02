# -*- coding: utf-8 -*-
#
# test_ticket_451.py
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


def test_ticket_451():
    """
    Guard against infinite loops in Random*Connect.

    This test ensures that conditions leading to infinite loops in
    random connection routines are caught when restrictions on multapses
    and autapses are taken into account.

    Author: Hans Ekkehard Plesser, 2010-09-20
    """

    # Create three iaf_psc_alpha neurons
    nodes = nest.Create("iaf_psc_alpha", 3)
    first = nodes[:1]  # Take first node

    # Test fixed indegree connections
    with pytest.raises(Exception):
        nest.Connect(
            nodes, first, {"rule": "fixed_indegree", "indegree": 4, "allow_multapses": False, "allow_autapses": True}
        )

    nest.Connect(
        nodes, first, {"rule": "fixed_indegree", "indegree": 3, "allow_multapses": False, "allow_autapses": True}
    )
    nest.Connect(
        nodes, first, {"rule": "fixed_indegree", "indegree": 2, "allow_multapses": False, "allow_autapses": True}
    )

    with pytest.raises(Exception):
        nest.Connect(
            nodes, first, {"rule": "fixed_indegree", "indegree": 4, "allow_multapses": False, "allow_autapses": False}
        )

    nest.Connect(
        nodes, first, {"rule": "fixed_indegree", "indegree": 2, "allow_multapses": False, "allow_autapses": False}
    )
    nest.Connect(
        nodes, first, {"rule": "fixed_indegree", "indegree": 1, "allow_multapses": True, "allow_autapses": True}
    )

    # Test fixed outdegree connections
    with pytest.raises(Exception):
        nest.Connect(
            first, nodes, {"rule": "fixed_outdegree", "outdegree": 4, "allow_multapses": False, "allow_autapses": True}
        )

    nest.Connect(
        first, nodes, {"rule": "fixed_outdegree", "outdegree": 3, "allow_multapses": False, "allow_autapses": True}
    )
    nest.Connect(
        first, nodes, {"rule": "fixed_outdegree", "outdegree": 2, "allow_multapses": False, "allow_autapses": True}
    )

    with pytest.raises(Exception):
        nest.Connect(
            first, nodes, {"rule": "fixed_outdegree", "outdegree": 4, "allow_multapses": False, "allow_autapses": False}
        )

    nest.Connect(
        first, nodes, {"rule": "fixed_outdegree", "outdegree": 2, "allow_multapses": False, "allow_autapses": False}
    )
    nest.Connect(
        first, nodes, {"rule": "fixed_outdegree", "outdegree": 1, "allow_multapses": True, "allow_autapses": False}
    )
