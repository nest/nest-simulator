# -*- coding: utf-8 -*-
#
# test_nest_set_get.py
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
Test the ``nest`` module's setters and getters.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_nest_get():
    """
    Test the ``nest`` module's getter.

    The test checks the ``nest`` module's ``.get`` function and ``KernelAttribute``
    access. The ``reset`` fixture resets the kernel so kernel attributes should
    be set to defaults. In general, the test should also fail if there is a
    problem with the ``.get`` mechanism.
    """

    kst = nest.get("keep_source_table")
    assert kst == nest.keep_source_table
    assert kst == type(nest).keep_source_table._default


def test_nest_get_unknown_attr_raises():
    """
    Ensure the ``nest`` module's getter raises an exception on unknown attribute access.

    Getting the value of unknown attributes should raise an exception. The test
    should also fail if there is a problem with possible ``__getattr__``
    implementations.
    """

    with pytest.raises(AttributeError):
        nest.accessAbsolutelyUnknownThingOnNestModule

    with pytest.raises(KeyError):
        nest.get("accessAbsolutelyUnknownKernelAttribute")


def test_kernel_attributes_missing_in_build():
    """
    Ensure that declared kernel attributes that this build does not report are not available.

    The kernel reports some attributes only if the build contains the feature, for example ``cycle_time_log``
    only with ``-Dwith-cycle-timers=ON``. Such an attribute must raise ``AttributeError``, so that ``hasattr``
    returns ``False``, and must not be listed in ``__all__``. Regression test for #4012.
    """

    status = nest.GetKernelStatus()
    missing = {k for k in nest._kernel_attr_names if k not in status and k != "kernel_status"}
    for name in missing:
        assert not hasattr(nest, name)
        assert name not in nest.__all__
        assert name not in dir(nest)


def test_star_import():
    """
    Ensure that ``from nest import *`` works in every build. Regression test for #4012.
    """

    exec("from nest import *", {})


def test_nest_set():
    """
    Test the ``nest`` module's setter.

    The test checks the ``nest`` module's ``.set`` function and ``KernelAttribute``
    assignment.
    """

    nest.set(rng_seed=12345)
    assert nest.rng_seed == 12345

    nest.set(rng_seed=345678)
    assert nest.rng_seed == 345678


def test_nest_set_unknown_attr_raises():
    """Ensure the ``nest`` module's setter raises exception on unknown attribute assignment."""

    # Setting the value of unknown attributes should error. Prevents user errors.
    with pytest.raises(AttributeError):
        nest.accessAbsolutelyUnknownThingOnNestModule = 5

    # Don't allow non-KernelAttributes to be replaced on the module.
    with pytest.raises(AttributeError):
        nest.get = 5
