# -*- coding: utf-8 -*-
#
# testutil.py
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

import dataclasses
import sys

import numpy as np
import pytest


def parameter_fixture(name, default_factory=lambda: None):
    return pytest.fixture(autouse=True, name=name)(lambda request: getattr(request, "param", default_factory()))


def dict_is_subset_of(small, big):
    """
    Return true if dict `small` is subset of dict `big`.

    `small` must contain all keys in `big` with the same values.
    """

    # See
    # https://stackoverflow.com/questions/20050913/python-unittests-assertdictcontainssubset-recommended-alternative
    # https://peps.python.org/pep-0584/
    #
    # Note: | is **not** a symmetric operator for dicts. `small` must be the second operand to | as it determines
    #       the value of joint keys in the merged dictionary.

    return big == big | small


def isin_approx(A, B, tol=1e-06):
    A = np.asarray(A)
    B = np.asarray(B)

    Bs = np.sort(B)  # skip if already sorted
    idx = np.searchsorted(Bs, A)

    linvalid_mask = idx == len(B)
    idx[linvalid_mask] = len(B) - 1
    lval = Bs[idx] - A
    lval[linvalid_mask] *= -1

    rinvalid_mask = idx == 0
    idx1 = idx - 1
    idx1[rinvalid_mask] = 0
    rval = A - Bs[idx1]
    rval[rinvalid_mask] *= -1
    return np.minimum(lval, rval) <= tol


def get_comparable_timesamples(actual, expected):
    simulated_points = isin_approx(actual[:, 0], expected[:, 0])
    expected_points = isin_approx(expected[:, 0], actual[:, 0])
    assert len(actual[simulated_points]) > 0, "The recorded data did not contain any relevant timesamples"
    return actual[simulated_points], pytest.approx(expected[expected_points])


def create_dataclass_fixtures(cls, module_name=None):
    for field, type_ in getattr(cls, "__annotations__", {}).items():
        if isinstance(field, dataclasses.Field):
            name = field.name
            if field.default_factory is not dataclasses.MISSING:
                default = field.default_factory
            else:

                def default(d=field.default):
                    return d

        else:
            name = field
            attr = getattr(cls, field)
            # We may be receiving a mixture of literal default values, field defaults,
            # and field default factories.
            if isinstance(attr, dataclasses.Field):
                if attr.default_factory is not dataclasses.MISSING:
                    default = attr.default_factory
                else:

                    def default(d=attr.default):
                        return d

            else:

                def default(d=attr):
                    return d

        setattr(
            sys.modules[module_name or cls.__module__],
            name,
            parameter_fixture(name, default),
        )


def use_simulation(cls):
    # If `mark` receives one argument that is a class, it decorates that arg.
    return pytest.mark.simulation(cls, "")
