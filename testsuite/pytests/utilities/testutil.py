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

import numbers
from collections.abc import Mapping

import nest
import pandas as pd
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


def get_comparable_timesamples(resolution, actual, expected):
    """
    Return result of inner join on time stamps for actual and expected given resolution.

    `actual` and `expected` must be arrays with columns representing times (in ms) and values.
    Times will be converted to steps given the resolution and the data will then be inner-
    joined on the steps, i.e., rows with equal steps will be extracted.

    Returns two one-dimensional arrays containing the values at the joint points from
    actual and expected, respectively.
    """

    tics = nest.tics_per_ms

    actual = pd.DataFrame(actual, columns=["t", "val_a"])
    expected = pd.DataFrame(expected, columns=["t", "val_e"])

    actual["tics"] = (actual.t * tics).round().astype(int)
    expected["tics"] = (expected.t * tics).round().astype(int)

    common = pd.merge(actual, expected, how="inner", on="tics")

    return common.val_a.values, common.val_e.values


# Recursive ``pytest.approx`` comparison for nested dict / list structures.
#
# The functions below walk the structure and apply ``approx`` at the highest
# level where it is actually valid:
#
#   * a flat sequence of numbers  -> compared as ``actual == approx(expected)``
#     (one list-vs-list comparison, not element by element);
#   * a numeric scalar            -> compared as ``actual == approx(expected)``;
#   * everything else (str, bool, None, keys, lengths, types) -> exact ``==``.
#
# Dicts recurse by key, nested lists recurse element-wise until a flat numeric
# list (or a scalar) is reached, at which point ``approx`` takes over.


def _is_number(x):
    # bool is a subclass of int -> compare bools exactly, not with approx.
    return isinstance(x, numbers.Number) and not isinstance(x, bool)


def _is_sequence(x):
    # Plain ordered sequences only; strings/bytes are treated as scalars.
    return isinstance(x, (list, tuple))


def _is_flat_numeric_sequence(seq):
    return len(seq) > 0 and all(_is_number(x) for x in seq)


def compare_approx(actual, expected, rel=None, abs=None, nan_ok=False, _path="<root>"):
    """Recursively compare ``actual`` and ``expected``.

    ``approx`` is applied to flat numeric sequences and numeric scalars with the
    given ``rel`` (relative) and optional ``abs`` (absolute) tolerances.

    Returns a list of human-readable difference strings. An empty list means the
    two structures are equal within tolerance.
    """

    diffs = []

    # one is a mapping or sequence, the other a different type
    if isinstance(actual, Mapping) != isinstance(expected, Mapping) or _is_sequence(actual) != _is_sequence(expected):
        diffs.append(f"{_path}: type mismatch " f"({type(actual).__name__} vs {type(expected).__name__})")
        return diffs

    # mapping vs mapping
    if isinstance(actual, Mapping) and isinstance(expected, Mapping):
        a_keys, e_keys = set(actual), set(expected)
        for k in sorted(e_keys - a_keys, key=repr):
            diffs.append(f"{_path}[{k!r}]: missing in actual")
        for k in sorted(a_keys - e_keys, key=repr):
            diffs.append(f"{_path}[{k!r}]: unexpected in actual")
        for k in sorted(a_keys & e_keys, key=repr):
            diffs += compare_approx(actual[k], expected[k], rel, abs, nan_ok, f"{_path}[{k!r}]")
        return diffs

    # sequence vs sequence
    if _is_sequence(actual) and _is_sequence(expected):
        if len(actual) != len(expected):
            diffs.append(f"{_path}: length mismatch " f"(actual {len(actual)} vs expected {len(expected)})")
            return diffs

        # highest valid approx level: a flat list of numbers -> one comparison.
        if _is_flat_numeric_sequence(expected) and _is_flat_numeric_sequence(actual):
            if not (actual == pytest.approx(expected, rel=rel, abs=abs, nan_ok=nan_ok)):
                diffs.append(f"{_path}: {actual!r} != " f"approx({expected!r}, rel={rel}, abs={abs})")
            return diffs

        # Otherwise recurse (nested lists, lists of dicts, lists of strings...).
        for i, (a, e) in enumerate(zip(actual, expected)):
            diffs += compare_approx(a, e, rel, abs, nan_ok, f"{_path}[{i}]")
        return diffs

    # numeric scalar vs numeric scalar
    if _is_number(actual) and _is_number(expected):
        if not (actual == pytest.approx(expected, rel=rel, abs=abs, nan_ok=nan_ok)):
            diffs.append(f"{_path}: {actual!r} != " f"approx({expected!r}, rel={rel}, abs={abs})")
        return diffs

    # everything else: exact equality (str, bool, None, type mismatch)
    if actual != expected:
        diffs.append(f"{_path}: {actual!r} != {expected!r}")
    return diffs


def assert_approx_equal(actual, expected, rel=None, abs=None, nan_ok=False):
    """
    Assert that actual and expected are equal when comparing numerical values with pytest.approx().

    This is useful for nested data structures which are too complex for pytest.approx() to handle
    directly.

    For arguments, see pytest.approx() documentation.

    Raises ``AssertionError`` listing every path that differs.
    """

    __tracebackhide__ = True
    diffs = compare_approx(actual, expected, rel=rel, abs=abs, nan_ok=nan_ok)
    if diffs:
        raise AssertionError(
            f"Structures differ ({len(diffs)} difference(s)):\n" + "\n".join("  - " + d for d in diffs)
        )


# end of recursive pytest.approx --------------
