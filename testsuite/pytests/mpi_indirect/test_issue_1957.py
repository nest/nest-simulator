# -*- coding: utf-8 -*-
#
# test_issue_1957.py
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


@pytest.mark.skipif_incompatible_mpi
@MPITestAssertEqual([2, 4])
def test_issue_1957():
    """
    Confirm that GetConnections works in parallel without hanging if not all ranks have connections.

    The test is performed on connection data written to OTHER_LABEL.
    """

    import nest
    import pandas as pd

    nest.ResetKernel()

    nrn = nest.Create("parrot_neuron")

    # Create two connections so we get lists back from pre_conns.get() and can build a DataFrame
    nest.Connect(nrn, nrn)
    nest.Connect(nrn, nrn)

    pre_conns = nest.GetConnections()
    if pre_conns:
        # need to do this here, Disconnect invalidates pre_conns
        df = pd.DataFrame.from_dict(pre_conns.get()).drop(labels="target_thread", axis=1)
        df.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False, sep="\t")  # noqa: F821

    nest.Disconnect(nrn, nrn)
    nest.Disconnect(nrn, nrn)
    post_conns = nest.GetConnections()
    assert not post_conns
