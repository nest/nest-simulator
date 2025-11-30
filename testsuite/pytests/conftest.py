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

import os
import pathlib
import subprocess
import sys

import nest
import pytest

# Make all modules in the `utilities` folder available to import in any test
sys.path.append(str(pathlib.Path(__file__).parent / "utilities"))
# Ignore it during test collection
collect_ignore = ["utilities"]

import testutil  # noqa


def pytest_configure(config):
    """
    Add NEST-specific markers.

    See https://docs.pytest.org/en/8.0.x/how-to/mark.html.
    """
    config.addinivalue_line(
        "markers",
        "requires_many_cores: mark tests as needing many cores (deselect with '-m \"not requires_many_cores\"')",
    )


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


@pytest.fixture(scope="session")
def have_threads():
    return nest.ll_api.sli_func("is_threaded")


@pytest.fixture(scope="session")
def report_dir() -> pathlib.Path:
    return pathlib.Path(os.environ.get("REPORTDIR", ""))


@pytest.fixture(autouse=True)
def skipif_missing_threads(request, have_threads):
    """
    Globally applied fixture that skips tests marked to be skipped when
    multithreading support is missing.
    """
    if not have_threads and request.node.get_closest_marker("skipif_missing_threads"):
        pytest.skip("skipped because missing multithreading support.")


@pytest.fixture(scope="session")
def have_mpi():
    return nest.ll_api.sli_func("statusdict/have_mpi ::")


@pytest.fixture(autouse=True)
def skipif_missing_mpi(request, have_mpi):
    """
    Globally applied fixture that skips tests marked to be skipped when MPI
    support is missing.
    """
    if not have_mpi and request.node.get_closest_marker("skipif_missing_mpi"):
        pytest.skip("skipped because missing MPI support.")


@pytest.fixture(scope="session")
def have_gsl():
    return nest.ll_api.sli_func("statusdict/have_gsl ::")


@pytest.fixture(autouse=True)
def skipif_missing_gsl(request, have_gsl):
    """
    Globally applied fixture that skips tests marked to be skipped when GSL is
    missing.
    """
    if not have_gsl and request.node.get_closest_marker("skipif_missing_gsl"):
        pytest.skip("skipped because missing GSL support.")


@pytest.fixture(scope="session")
def have_hdf5():
    return nest.ll_api.sli_func("statusdict/have_hdf5 ::")


@pytest.fixture(autouse=True)
def skipif_missing_hdf5(request, have_hdf5):
    """
    Globally applied fixture that skips tests marked to be skipped when HDF5 is
    missing.
    """
    if not have_hdf5 and request.node.get_closest_marker("skipif_missing_hdf5"):
        pytest.skip("skipped because missing HDF5 support.")


@pytest.fixture(scope="session")
def have_plotting():
    try:
        import matplotlib

        matplotlib.use("Agg")  # backend without window
        import matplotlib.pyplot as plt

        # make sure we can open a window; DISPLAY may not be set
        fig = plt.figure()
        plt.close(fig)
        return True
    except Exception:
        return False


@pytest.fixture(scope="session")
def subprocess_compatible_mpi():
    """
    Until at least OpenMPI 4.1.6, the following fails due to a bug in OpenMPI,
    from 5.0.7 is definitely safe.
    """
    try:
        res = subprocess.run(["mpirun", "-np", "1", "echo"])
        return 0 == res.returncode
    except FileNotFoundError:
        return False


@pytest.fixture(autouse=True)
def skipif_incompatible_mpi(request, subprocess_compatible_mpi):
    """
    Globally applied fixture that skips tests marked to be skipped when MPI is
    not compatible with subprocess.
    """

    if not subprocess_compatible_mpi and request.node.get_closest_marker("skipif_incompatible_mpi"):
        pytest.skip("skipped because MPI is incompatible with subprocess")
