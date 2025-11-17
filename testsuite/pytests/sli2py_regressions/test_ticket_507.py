# -*- coding: utf-8 -*-
#
# test_ticket_507.py
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
Regression test for Ticket #507.

Test ported from SLI regression test
Ensure that spike_generator throws exception on SetStatus if off-grid times are set,
unless precise_times is set.
"""
import nest
import pytest


@pytest.fixture(autouse=True)
def prepare_kernel():
    nest.ResetKernel()
    nest.resolution = 0.1


def test_spike_generator_on_grid_time():
    """
    Setting a single on-grid spike time with precise_times=False should succeed.
    """
    sg = nest.Create("spike_generator")
    sg.set({"spike_times": [0.2], "precise_times": False})


def test_spike_generator_off_grid_times_should_fail():
    """
    Setting multiple spike times, some off-grid, with precise_times=False should fail.
    """
    sg = nest.Create("spike_generator")
    with pytest.raises(nest.NESTError):
        sg.set({"spike_times": [0.1, 0.123456789, 0.22345567854], "precise_times": False})


def test_spike_generator_single_off_grid_time_should_fail():
    """
    Setting a single off-grid spike time with precise_times=False should fail.
    """
    sg = nest.Create("spike_generator")
    with pytest.raises(nest.NESTError):
        sg.set({"spike_times": [0.123456789], "precise_times": False})


def test_spike_generator_precise_times_true_should_pass():
    """
    Setting a single off-grid spike time with precise_times=True should succeed.
    """
    sg = nest.Create("spike_generator")
    sg.set({"spike_times": [0.123456789], "precise_times": True})
