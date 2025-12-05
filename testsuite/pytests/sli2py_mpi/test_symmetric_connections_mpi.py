# -*- coding: utf-8 -*-
#
# test_symmetric_connections_mpi.py
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


def assert_symmetric(all_res):
    for conns in all_res["other"]:
        conns.set_index(["source", "target"], inplace=True)
        assert all(all(conns.loc[(s, t)] == conns.loc[(t, s)]) for (s, t) in conns.index)


@pytest.mark.skipif_incompatible_mpi
@pytest.mark.skipif_missing_threads
@MPITestAssertEqual([1, 2, 4], debug=False, specific_assert=assert_symmetric)
def test_symmetric_connections_mpi():
    """
    Confirm that symmetric connections are created correctly.
    """

    import nest
    import pandas as pd  # noqa: F811

    nest.total_num_virtual_procs = 4

    N = 5
    pop1 = nest.Create("parrot_neuron", 5)
    pop2 = nest.Create("parrot_neuron", 5)

    nest.Connect(
        pop1,
        pop2,
        {"rule": "one_to_one", "make_symmetric": True},
        {
            "synapse_model": "stdp_synapse",
            "weight": np.linspace(1, 5, num=N),
            "delay": np.linspace(11, 15, num=N),
            "alpha": np.linspace(21, 25, num=N),
        },
    )

    conns = pd.DataFrame.from_dict(nest.GetConnections().get(["source", "target", "weight", "delay", "alpha"]))

    conns.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False, sep="\t")  # noqa: F821
