# -*- coding: utf-8 -*-
#
# test_issue_2119.py
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

# use default parameters for double-valued params
random_params = [
    ["exponential", {}],
    ["normal", {}],
    ["lognormal", {}],
    ["uniform", {}],
    ["uniform_int", {"max": 10}],
]


@pytest.mark.skipif_incompatible_mpi
@pytest.mark.parametrize(["kind", "specs"], random_params)
@MPITestAssertEqual([1, 2, 4])
def test_issue_2119(kind, specs):
    """
    Confirm that randomized node parameters work correctly under MPI and OpenMP.

    The test is performed on GID-V_m data written to OTHER_LABEL.
    """

    import nest
    import pandas as pd

    nest.ResetKernel()
    nest.total_num_virtual_procs = 4

    nrn = nest.Create("iaf_psc_alpha", n=4, params={"V_m": nest.CreateParameter(kind, specs)})

    pd.DataFrame.from_dict(nrn.get(["global_id", "V_m"])).dropna().to_csv(
        OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False, sep="\t"  # noqa: F821
    )
