# -*- coding: utf-8 -*-
#
# test_all_to_all.py
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


from mpi_test_wrapper import MPITestAssertEqual


@MPITestAssertEqual([1, 4], debug=False)
def test_all_to_all():
    """
    Confirm that all-to-all connections created correctly for more targets than local nodes.
    """

    import nest
    import pandas as pd

    nest.ResetKernel()

    nrns = nest.Create("parrot_neuron", n=4)
    nest.Connect(nrns, nrns, "all_to_all")

    conns = nest.GetConnections().get(output="pandas").drop(labels=["target_thread", "port"], axis=1)
    conns.to_csv(OTHER_LABEL.format(nest.num_processes) + f"-{nest.Rank()}.dat", index=False)  # noqa: F821
