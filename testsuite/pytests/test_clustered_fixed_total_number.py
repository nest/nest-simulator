# -*- coding: utf-8 -*-
#
# test_clustered_fixed_total_number.py
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
Test models with calcium concentration.

This set of tests verify the behavior of the calcium concentration in models
that inherit from the strutural plasticity node class in the kernel.
"""

import nest
import numpy as np
import pytest


@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("n_threads", [1, 2, 3])
def test_clustered_fixed_total_number(n_threads):
    """Minimal test for clustered connectivity."""

    nest.ResetKernel()
    nest.local_num_threads = n_threads

    a = nest.Create("parrot_neuron", n=50)
    b = nest.Create("parrot_neuron", n=50)

    num_conns = 1000
    w_intra = 100
    w_cross = 3

    nest.Connect(
        a,
        b,
        {"rule": "clustered_fixed_total_number", "N": num_conns, "num_clusters": 2},
        nest.CollocatedSynapses({"weight": w_intra}, {"weight": w_cross}),
    )

    assert nest.num_connections * nest.num_processes == num_conns

    # All odd numbered neurons get assigned to one cluster and the even numbered
    # ones to the other. Thus, the sum of source and target ids for will be even
    # for intra-cluster connections and odd for cross-cluster connections.
    c = nest.GetConnections().get(["source", "target", "weight"], output="pandas")

    assert all(c.loc[(c["source"] + c["target"]) % 2 == 0, "weight"] == w_intra)
    assert all(c.loc[(c["source"] + c["target"]) % 2 == 1, "weight"] == w_cross)
