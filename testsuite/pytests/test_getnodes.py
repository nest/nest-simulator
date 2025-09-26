# -*- coding: utf-8 -*-
#
# test_getnodes.py
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
Test GetNodes
"""

import nest
import pytest


@pytest.fixture(autouse=True, params=[1, 2])
def create_neurons(request):
    nest.ResetKernel()
    nest.local_num_threads = request.param

    nest.Create("iaf_psc_alpha", 3)
    nest.Create("iaf_psc_delta", 2, {"V_m": -77.0})
    nest.Create("iaf_psc_alpha", 4, {"V_m": [-77.0, -66.0, -77.0, -66.0], "tau_m": [10.0, 11.0, 12.0, 13.0]})
    nest.Create("iaf_psc_exp", 4)
    nest.Create("spike_generator", 3)


@pytest.mark.parametrize("local_only", [True, False])
class TestGetNodes:
    """Tests for GetNodes() function."""

    def test_GetNodes(self, local_only):
        """Test that nodes are correctly retrieved if on parameters are specified."""

        nodes_ref = nest.NodeCollection(list(range(1, nest.network_size + 1)))
        if local_only:
            nodes_ref = sum(n for n in nodes_ref if n.local)

        nodes = nest.GetNodes(local_only=local_only)

        assert nodes == nodes_ref

    @pytest.mark.parametrize(
        "filter, expected_ids",
        [
            [{"V_m": -77.0}, [4, 5, 6, 8]],
            [{"V_m": -77.0, "tau_m": 12.0}, [8]],
            [{"model": "iaf_psc_exp"}, [10, 11, 12, 13]],
            [{"model": "spike_generator"}, [14, 15, 16]],
        ],
    )
    def test_GetNodes_with_params(self, local_only, filter, expected_ids):
        """Test that nodes are correctly filtered."""

        nodes_ref = nest.NodeCollection(expected_ids)
        if local_only:
            nodes_ref = sum(n for n in nodes_ref if n.local)

        nodes = nest.GetNodes(properties=filter, local_only=local_only)

        assert nodes == nodes_ref

    def test_GetNodes_no_match(self, local_only):
        """
        Ensure we get an empty result if nothing matches.

        This would lead to crashes in MPI-parallel code before #3460.
        """

        nodes = nest.GetNodes({"V_m": 100.0})
        assert len(nodes) == 0
