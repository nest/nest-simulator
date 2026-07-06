# -*- coding: utf-8 -*-
#
# test_connect_array_mpi.py
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

import numpy as np
import pandas as pd
import pytest
from mpi_test_wrapper import MPITestAssertEqual

# Functions to be passed via decorators must be in module namespace without qualifiers
from numpy import meshgrid


@pytest.mark.skipif_incompatible_mpi
@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize(
    "connspec",
    [{"rule": "all_to_all"}, {"rule": "fixed_indegree", "indegree": 5}, {"rule": "fixed_outdegree", "outdegree": 5}],
)
@MPITestAssertEqual([2, 3], debug=False)
def test_issue_connect_array_mpi(connspec):
    """
    Confirm that connections are created correctly when weights and delays are passed as arrays.

    The test evaluates locally on each rank that expected values have been set.
    Additionally, the wrapper checks for consistency across different numbers of MPI processes.
    This is mostly for illustration of how to combine local and global testing and not strictly necessary.
    """

    import nest
    import pandas as pd  # noqa: F811

    N = 10

    nest.ResetKernel()
    nest.total_num_virtual_procs = 6

    nrns = nest.Create("parrot_neuron", n=N)

    # Build arrays assigning unique weight and delay to each source-target pair
    def gids_to_weights(sgids, tgids):
        return sgids + 100 * tgids

    def gids_to_delays(sgids, tgids):
        return nest.resolution * (1 + 97 * sgids + tgids)

    gids = np.array(nrns.tolist())

    if connspec["rule"] == "all_to_all":
        sg, tg = np.meshgrid(gids, gids)
    elif connspec["rule"] == "fixed_indegree":
        sg, tg = np.meshgrid(gids[: connspec["indegree"]], gids)
    elif connspec["rule"] == "fixed_outdegree":
        sg, tg = np.meshgrid(gids[: connspec["outdegree"]], gids)

    weights = gids_to_weights(sg, tg)
    delays = gids_to_delays(sg, tg)

    nest.Connect(nrns, nrns, conn_spec=connspec, syn_spec={"weight": weights, "delay": delays})

    conns = pd.DataFrame.from_dict(nest.GetConnections().get(["source", "target", "weight", "delay"]))

    if connspec["rule"] == "all_to_all":
        # Weights are assigned in order for all-to-all, so we know precisely which source-target
        # pair shall have which weight and delay
        expected_weights = gids_to_weights(conns.source, conns.target)
        expected_delays = gids_to_delays(conns.source, conns.target)

        pd.testing.assert_series_equal(conns.weight, expected_weights, check_dtype=False, check_names=False)
        pd.testing.assert_series_equal(conns.delay, expected_delays, check_dtype=False, check_names=False)

    elif connspec["rule"] == "fixed_indegree":
        # Each target neuron has all the weights passed in but mapping to source neurons is unknown
        # Thus, we extract weight/delays for each target neuron and compare as sets
        for ix, tgt in enumerate(nrns):
            if tgt.local:
                expected_weights = weights[ix, :]
                expected_delays = delays[ix, :]
                actual_weights = conns.loc[conns.target == tgt.global_id].weight
                actual_delays = conns.loc[conns.target == tgt.global_id].delay
                assert set(actual_weights) == set(expected_weights)
                assert set(actual_delays) == set(expected_delays)

    elif connspec["rule"] == "fixed_outdegree":
        # Here, we know which set of weights/delays each source neuron has, but we don't know how
        # the target neurons and thus weights/delays are local. Thus, we can only check for having a subset.
        for ix, src in enumerate(nrns):
            expected_weights = weights[ix, :]
            expected_delays = delays[ix, :]
            actual_weights = conns.loc[conns.source == src.global_id].weight
            actual_delays = conns.loc[conns.source == src.global_id].delay
            assert set(actual_weights) <= set(expected_weights)
            assert set(actual_delays) <= set(expected_delays)

    conns.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False, sep="\t")  # noqa: F821
