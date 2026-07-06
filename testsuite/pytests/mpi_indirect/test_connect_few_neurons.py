# -*- coding: utf-8 -*-
#
# test_connect_few_neurons.py
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
@pytest.mark.parametrize("connspec", ["all_to_all", "one_to_one", {"rule": "pairwise_bernoulli", "p": 1.0}])
@MPITestAssertEqual([1, 4], debug=False)
def test_connect_few_neurons(connspec):
    """
    Confirm that connections created correctly for more targets than local nodes.

    The test is performed on connection data written to OTHER_LABEL.
    """

    import nest

    nrns = nest.Create("parrot_neuron", n=4)
    nest.Connect(nrns, nrns, connspec)

    conns = nest.GetConnections().get(output="pandas").drop(labels=["target_thread", "port"], axis=1)
    conns.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False, sep="\t")  # noqa: F821
