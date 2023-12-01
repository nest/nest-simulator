# -*- coding: utf-8 -*-
#
# test_mpi_dev.py
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


# TODO: delete this development file

# import nest
import pandas as pd
import pytest
from mpi_wrapper import mpi_assert_equal_df
from mpi_wrapper2 import mpi_wrapper

"""
@pytest.mark.parametrize("n_nrns", [2])
@mpi_assert_equal_df([1, 2])
def test_func(n_nrns):
    nest.ResetKernel()
    nest.total_num_virtual_procs = 2
    nrns = nest.Create("iaf_psc_alpha", n_nrns)
    sinj = nest.Create("spike_train_injector", params={"spike_times": [1.0, 3.0, 5.0]})
    srec = nest.Create("spike_recorder")

    nest.Connect(sinj, nrns, syn_spec={"weight": 2000.0})
    nest.Connect(nrns, srec)

    nest.Simulate(10.0)

    df = pd.DataFrame.from_records(srec.events)

    return df



# @pytest.mark.parametrize("n_nrns", [2])
@mpi_wrapper
def test_func():
    n_nrns = 2
    nest.ResetKernel()
    nest.total_num_virtual_procs = 2
    nrns = nest.Create("iaf_psc_alpha", n_nrns)
    sinj = nest.Create("spike_train_injector", params={"spike_times": [1.0, 3.0, 5.0]})
    srec = nest.Create("spike_recorder")

    nest.Connect(sinj, nrns, syn_spec={"weight": 2000.0})
    nest.Connect(nrns, srec)

    nest.Simulate(10.0)

    df = pd.DataFrame.from_records(srec.events)

    return df
"""


# @mpi_wrapper
@mpi_assert_equal_df([1])
def test_func2():
    a = 2
    b = 4
    # print("I AM TEST")

    return a + b
