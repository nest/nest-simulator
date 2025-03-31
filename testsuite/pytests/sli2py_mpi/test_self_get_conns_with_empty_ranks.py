# -*- coding: utf-8 -*-
#
# test_self_get_conns_with_empty_ranks.py
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
import pandas
import pytest
from mpi_test_wrapper import MPITestAssertEqual


# Parametrization over the number of nodes here only to show hat it works
@MPITestAssertEqual([1, 2, 4], debug=False)
def test_self_get_conns_with_empty_ranks():
    """
    Selftest: Confirm that connections can be gathered correctly even if some ranks have no neurons.
    """

    import nest
    import pandas as pd

    nrns = nest.Create("parrot_neuron", n=2)
    nest.Connect(nrns, nrns)

    conns = nest.GetConnections().get(output="pandas").drop(labels=["target_thread", "port"], axis=1, errors="ignore")
    conns.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False)  # noqa: F821
