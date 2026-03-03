# -*- coding: utf-8 -*-
#
# test_global_rng.py
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

import pandas as pd
import pytest
from mpi_test_wrapper import MPITestAssertAllRanksEqual


# Parametrization over the number of nodes here only to show hat it works
@pytest.mark.skipif_incompatible_mpi
@MPITestAssertAllRanksEqual([1, 2, 4], debug=False)
def test_global_rng():
    """
    Confirm that NEST random parameter used from the Python level uses globally sync'ed RNG correctly.
    All ranks must report identical random number sequences independent of the number of ranks.

    The test compares connection data written to OTHER_LABEL.
    """

    import nest

    nest.rng_seed = 12
    p = nest.CreateParameter("uniform", {"min": 0, "max": 1})

    # Uncomment one of the two for loops to provoke failure
    # for _ in range(nest.num_processes):
    #     p.GetValue()
    # for _ in range(nest.Rank()):
    #     p.GetValue()

    vals = pd.DataFrame([p.GetValue() for _ in range(5)])
    vals.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), sep="\t")  # noqa: F821
