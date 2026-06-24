# -*- coding: utf-8 -*-
#
# test_issue_3228.py
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


import pytest

"""
Issue 3228: Ensure that Normal and Lognormal Parameters trigger assertion if created before threads are set.

Note: This test will fail if NEST is compiled with NDEBUG.
"""


import multiprocessing
import signal

# We need to indirect via multiprocessing to catch the C-style assertion thrown


def set_threads_before_param(distribution):
    import nest

    p = nest.CreateParameter(distribution, {"mean": 0, "std": 1})
    nest.local_num_threads = 2
    p.GetValue()


@pytest.mark.parametrize("distribution", ["normal", "lognormal"])
def test_params_raise_assertion(distribution):
    """
    Confirm that Normal and Lognormal Parameters trigger assertion if created before threads are set.
    """

    proc = multiprocessing.Process(target=set_threads_before_param, args=(distribution,))
    proc.start()
    proc.join()

    # On Unix, a negative returncode means it was killed by signal -returncode
    assert proc.exitcode == -signal.SIGABRT, f"Expected SIGABRT, got exit code {proc.exitcode}"
