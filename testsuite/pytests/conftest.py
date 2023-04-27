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
import dataclasses
import pytest
import nest
import sys
import pathlib

# Make all modules in the `utilities` folder available to import in any test
sys.path.append(str(pathlib.Path(__file__).parent / "utilities"))
# Ignore it during test collection
collect_ignore = ["utilities"]

import testutil, testsimulation

_have_mpi = nest.ll_api.sli_func("statusdict/have_mpi ::")
_have_gsl = nest.ll_api.sli_func("statusdict/have_gsl ::")
_have_threads = nest.ll_api.sli_func("statusdict/threading ::") != "no"


@pytest.fixture(scope="module", autouse=True)
def safety_reset():
    """
    Reset the NEST kernel for each module.

    This fixture only applies on the module level. It prevents leakage of
    kernel states between different test modules (test files).

    .. note::
        To reset the kernel between tests inside a module, call
        `nest.ResetKernel()` within the module itself.
    """
    nest.ResetKernel()


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


@pytest.fixture(autouse=True)
def simulation_class(request):
    return getattr(request, "param", testsimulation.Simulation)


@pytest.fixture
def simulation(request):
    marker = request.node.get_closest_marker("simulation")
    sim_cls = marker.args[0] if marker else testsimulation.Simulation
    sim = sim_cls(
        *(request.getfixturevalue(field.name) for field in dataclasses.fields(sim_cls))
    )
    nest.ResetKernel()
    if getattr(sim, "set_resolution", True):
        nest.resolution = sim.resolution
    nest.local_num_threads = sim.local_num_threads
    return sim


# Inject the root simulation fixtures into this module to be always available.
testutil.create_dataclass_fixtures(testsimulation.Simulation, __name__)
