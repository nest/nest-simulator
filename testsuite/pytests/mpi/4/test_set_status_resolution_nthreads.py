# -*- coding: utf-8 -*-
#
# test_set_status_resolution_nthreads.py
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

import nest
import pytest

HAVE_OPENMP = nest.ll_api.sli_func("is_threaded")


@pytest.fixture
def reset():
    nest.ResetKernel()


@pytest.mark.skipif(not HAVE_OPENMP, reason='NEST was compiled without multi-threading')
def testSetStatus_resolution_before_nthreads(reset):
    """Test if resolution can be set before number of threads."""

    nest.resolution = 0.5
    nest.local_num_threads = 4
    nest.Simulate(100)
    assert nest.resolution == 0.5
    assert nest.local_num_threads == 4


@pytest.mark.skipif(not HAVE_OPENMP, reason='NEST was compiled without multi-threading')
def testSetStatus_nthreads_before_resolution(reset):
    """Test if number of threads can be set before resolution."""

    nest.local_num_threads = 4
    nest.resolution = 0.5
    nest.Simulate(100)
    assert nest.resolution == 0.5
    assert nest.local_num_threads == 4
