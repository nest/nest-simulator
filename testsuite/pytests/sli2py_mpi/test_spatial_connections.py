# -*- coding: utf-8 -*-
#
# test_spatial_connections.py
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
from mpi_test_wrapper import MPITestAssertEqual

"""
Confirm that spatial connections are created consistently for fixed VP.

This test is parameterized at three levels:
- Neuron and generator as source
- Free and grid layers
- Different spatial connection rules

Connections to recorders are handled in a separate test, as they only work with
the pairwise-bernoulli-on-source connection rule.
"""


# We cannot use nest or numpy outside the test function itself, so we need to create
# the free positions with basic Python commands.
@pytest.mark.skipif_incompatible_mpi
@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("source_model", ["poisson_generator", "parrot_neuron"])
@pytest.mark.parametrize(
    "geometry",
    [
        ("free", {"pos": [(x, y) for x in range(-2, 3) for y in range(-2, 3)], "extent": [6, 6], "edge_wrap": True}),
        ("grid", {"shape": [5, 5], "extent": [6, 6], "edge_wrap": True}),
    ],
)
@pytest.mark.parametrize(
    "conn_spec",
    [
        {"rule": "fixed_indegree", "indegree": 10},
        {"rule": "fixed_outdegree", "outdegree": 10},
        {"rule": "pairwise_bernoulli", "p": 0.5, "use_on_source": False},
        {"rule": "pairwise_bernoulli", "p": 0.5, "use_on_source": True},
    ],
)
@MPITestAssertEqual([1, 2, 4], debug=False)
def test_spatial_connections(source_model, geometry, conn_spec):
    """
    Confirm that spatial connections are created consistently for fixed VP.

    The test is performed on connection data written to OTHER_LABEL.
    """

    import nest
    import numpy as np
    import pandas as pd

    nest.ResetKernel()
    nest.total_num_virtual_procs = 4

    kind, specs = geometry
    if kind == "free":
        pos = nest.spatial.free(**specs)
    else:
        assert kind == "grid"
        pos = nest.spatial.grid(**specs)
    source_layer = nest.Create(source_model, positions=pos)
    target_layer = nest.Create("parrot_neuron", positions=pos)

    nest.Connect(
        source_layer,
        target_layer,
        {**conn_spec, "mask": {"circular": {"radius": 2.5}}},
        {
            "weight": nest.spatial_distributions.gaussian(10 * nest.spatial.distance, std=2),
            "delay": 0.1 + 0.2 * nest.spatial.distance,
        },
    )

    conns = nest.GetConnections()
    df = pd.DataFrame.from_dict(conns.get(["source", "target", "weight", "delay"]))
    df.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False, sep="\t")  # noqa: F821
