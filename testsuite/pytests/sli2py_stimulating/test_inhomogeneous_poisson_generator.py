# -*- coding: utf-8 -*-
#
# test_inhomogeneous_poisson_generator.py
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

"""
This test checks the inhomogeneous_poisson_generator device and its consistency with the nest simulation kernel.
"""

import nest
import pytest


@pytest.fixture
def prepare_kernel():
    nest.ResetKernel()
    nest.resolution = 0.1
    nest.local_num_threads = 1


def test_rate_times_and_values_match(prepare_kernel):
    inh_pg = nest.Create("inhomogeneous_poisson_generator")

    with pytest.raises(nest.kernel.NESTErrors.BadProperty, match="Rate times and values must be reset together"):
        inh_pg.set(rate_values=[10.0])


def test_rate_times_len_and_values_match(prepare_kernel):
    inh_pg = nest.Create("inhomogeneous_poisson_generator")

    with pytest.raises(nest.kernel.NESTErrors.BadProperty, match="Rate times and values have to be the same size"):
        inh_pg.set(rate_times=[1.0, 2.0], rate_values=[10.0])


def test_rate_times_strictly_increasing(prepare_kernel):
    inh_pg = nest.Create("inhomogeneous_poisson_generator")

    with pytest.raises(nest.kernel.NESTErrors.BadProperty, match="Rate times must be strictly increasing"):
        inh_pg.set(rate_times=[1.0, 5.0, 3.0], rate_values=[10.0, 20.0, 5.0])


def test_offgrid_time_point(prepare_kernel):
    inh_pg = nest.Create("inhomogeneous_poisson_generator")

    with pytest.raises(
        nest.kernel.NESTErrors.BadProperty, match="Time point 1.23 is not representable in current resolution"
    ):
        inh_pg.set(rate_times=[1.23], rate_values=[10.0])


def test_allow_offgrid_time_point(prepare_kernel):
    inh_pg = nest.Create("inhomogeneous_poisson_generator", {"allow_offgrid_times": True})
    inh_pg.set(rate_times=[1.23], rate_values=[10.0])
    defaults = inh_pg.get()

    # assert that the rate time is rounded up to the next step
    assert defaults["rate_times"] == 1.3


def test_no_allow_offgrid_times_after_rate_set(prepare_kernel):
    inh_pg = nest.Create("inhomogeneous_poisson_generator")
    inh_pg.set(rate_times=[1.2], rate_values=[10.0])

    with pytest.raises(
        nest.kernel.NESTErrors.BadProperty,
        match="Option can only be set together with rate times or if no rate times have been set",
    ):
        inh_pg.set(allow_offgrid_times=True)


def test_allow_offgrid_times_modified_after_rate_set(prepare_kernel):
    inh_pg = nest.Create("inhomogeneous_poisson_generator", {"allow_offgrid_times": True})
    inh_pg.set(rate_times=[1.23], rate_values=[10.0])

    with pytest.raises(
        nest.kernel.NESTErrors.BadProperty, match="Time point 1.25 is not representable in current resolution"
    ):
        inh_pg.set(allow_offgrid_times=False, rate_times=[1.25], rate_values=[10.0])


def test_time_points_in_future(prepare_kernel):
    inh_pg = nest.Create("inhomogeneous_poisson_generator")
    with pytest.raises(nest.kernel.NESTErrors.BadProperty, match="Time points must lie strictly in the future."):
        inh_pg.set(rate_times=[0.0], rate_values=[30.0])
