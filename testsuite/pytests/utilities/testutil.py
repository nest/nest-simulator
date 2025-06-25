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

import numpy as np
import pytest


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
    """
    Determine whether each element in array A approximately exists in array B within a given tolerance.

    Parameters
    ----------
    A : array-like
       Input array of values to check for approximate membership in B.
    B : array-like
       Reference array to check against.
    tol : float, optional
       Absolute tolerance within which two values are considered equal. Default is 1e-6.

    Returns
    -------
    np.ndarray
       A boolean array of the same shape as A, where each element is True if the corresponding
       value in A is within `tol` of any value in B.

    Notes
    -----
    - Uses a vectorized, sorted search to efficiently find nearest neighbors in B for each value in A.
    - Both lower and upper neighbor distances are computed to detect proximity.
    - Especially useful when comparing floating-point timestamps or simulation outputs where exact
     equality is unrealistic due to numerical noise.
    """

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
    """
    Filter and align time-series data from simulation and reference arrays
    for meaningful comparison with approximate equality.

    Parameters
    ----------
    actual : np.ndarray
        A 2D array of shape (N, 2), where the first column is time and the second is the simulated value.
    expected : np.ndarray
        A 2D array of shape (M, 2), with the same structure as `actual`, representing expected reference data.

    Returns
    -------
    Tuple[np.ndarray, pytest.approx]
        A pair of arrays aligned by approximately matching time samples:
        - A filtered version of `actual` at time points close to those in `expected`.
        - A `pytest.approx`-wrapped array of `expected` values corresponding to the matched points.

    Raises
    ------
    AssertionError
        If no matching time points are found between `actual` and `expected`, the test fails early.

    Notes
    -----
    This function supports robust comparison of simulation outputs against expected results
    by:
    - Tolerantly aligning timestamps via approximate equality (`isin_approx`).
    - Ensuring non-empty overlap to prevent false test passes on mismatched data.
    - Returning results in a format suitable for `assert actual == expected`.

    Typical use-case is inside a pytest unit test that compares time-series simulation output
    to a precomputed reference array, using `pytest.approx` to allow for small numerical errors.
    """
    simulated_points = isin_approx(actual[:, 0], expected[:, 0])
    expected_points = isin_approx(expected[:, 0], actual[:, 0])
    assert len(actual[simulated_points]) > 0, "The recorded data did not contain any relevant timesamples"
    return actual[simulated_points], pytest.approx(expected[expected_points])
