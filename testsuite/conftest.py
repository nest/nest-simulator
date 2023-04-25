# -*- coding: utf-8 -*-
#
# conftest.py
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
Fixtures available to the entire testsuite directory.

`skipif_missing_*` usage:

    import pytest

    @pytest.mark.skipif_missing_gsl()
    def test_gsl():
        pass
"""

import pytest
import nest

_have_mpi = nest.ll_api.sli_func("statusdict/have_mpi ::")
_have_gsl = nest.ll_api.sli_func("statusdict/have_gsl ::")
_have_threads = nest.ll_api.sli_func("statusdict/threading ::") != "no"


@pytest.fixture(autouse=True)
def skipif_missing_gsl(request):
    """
    Globally applied fixture that skips tests marked to be skipped when GSL is missing.
    """
    if not _have_gsl and request.node.get_closest_marker("skipif_missing_gsl"):
        pytest.skip("skipped because missing GSL support.")


@pytest.fixture(autouse=True)
def skipif_missing_mpi(request):
    """
    Globally applied fixture that skips tests marked to be skipped when MPI is missing.
    """
    if not _have_mpi and request.node.get_closest_marker("skipif_missing_mpi"):
        pytest.skip("skipped because missing MPI support.")


@pytest.fixture(autouse=True)
def skipif_missing_threads(request):
    """
    Globally applied fixture that skips tests marked to be skipped when multithreading
    support is missing.
    """
    if not _have_threads and request.node.get_closest_marker("skipif_missing_threads"):
        pytest.skip("skipped because missing multithreading support.")
