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


@pytest.mark.skipif_incompatible_mpi
@pytest.mark.skipif_missing_threads
@MPITestAssertEqual([2, 3], debug=True)
def test_issue_connect_array_mpi():
    """
    Confirm that connections are created correctly when weights and delays are passed as arrays.

    The test evaluates locally on each rank that expected values have been set.
    Additionally, the wrapper checks for consistency across different numbers of MPI processes.
    This is mostly for illustration of how to combine local and global testing and not strictly necessary.
    """

    import nest
    import pandas as pd

    N = 10

    nest.ResetKernel()
    nest.total_num_virtual_procs = 6

    nrns = nest.Create("parrot_neuron", n=N)

    # Build arrays assigning unique weight and delay to each source-target pair
    gids_to_weights = lambda sgids, tgids: sgids + 100 * tgids
    gids_to_delays = lambda sgids, tgids: nest.resolution * (1 + 97 * sgids + tgids)

    gids = np.array(nrns.tolist())
    sg, tg = np.meshgrid(gids, gids)
    weights = gids_to_weights(sg, tg)
    delays = gids_to_delays(sg, tg)

    nest.Connect(nrns, nrns, conn_spec="all_to_all", syn_spec={"weight": weights, "delay": delays})

    conns = pd.DataFrame.from_dict(nest.GetConnections().get(["source", "target", "weight", "delay"]))

    expected_weights = gids_to_weights(conns.source, conns.target)
    expected_delays = gids_to_delays(conns.source, conns.target)

    pd.testing.assert_series_equal(conns.weight, expected_weights, check_dtype=False, check_names=False)
    pd.testing.assert_series_equal(conns.delay, expected_delays, check_dtype=False, check_names=False)

    conns.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False)  # noqa: F821
