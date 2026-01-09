# -*- coding: utf-8 -*-
#
# test_issue_659.py
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
Regression test for Issue #659 (GitHub).

This test checks that calling Run or Cleanup without calling Prepare first results in an error.
"""

import nest
import pytest


def test_run_without_prepare():
    """
    Test that calling Run without Prepare results in an error.
    """
    nest.ResetKernel()
    with pytest.raises(nest.kernel.NESTError):
        nest.Run(10.0)


def test_cleanup_without_prepare():
    """
    Test that calling Cleanup without Prepare results in an error.
    """
    nest.ResetKernel()
    with pytest.raises(Exception):
        nest.Cleanup()


def test_prepare_twice():
    """
    Test that calling Prepare twice results in an error.
    """
    nest.ResetKernel()
    nest.Prepare()
    with pytest.raises(Exception):
        nest.Prepare()


def test_run_after_cleanup_without_prepare():
    """
    Test that calling Run after Cleanup, without Prepare, results in an error.
    """
    nest.ResetKernel()
    nest.Prepare()
    nest.Run(10.0)
    nest.Cleanup()
    with pytest.raises(Exception):
        nest.Run(10.0)
