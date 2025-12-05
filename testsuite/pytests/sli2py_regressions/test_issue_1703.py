# -*- coding: utf-8 -*-
#
# test_issue_1703.py
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
This script ensures that importing NEST after mpi4py does not lead
to a segmentation fault when simulating.

This is a regression test for GitHub issue 1703.
"""

import os
import shlex
import subprocess
import sys

import nest
import pytest


@pytest.mark.skipif_missing_mpi4py
@pytest.mark.parametrize("nest_first", [True, False])
def test_issue_1703(nest_first):
    """
    Confirm that mpi4py and nest can be imported in any order.
    """

    EXIT_CODE_SUCCESS = 0
    EXIT_CODE_ERROR = 1

    imports = ["import nest", "from mpi4py import MPI"]
    import_cmds = ";".join(imports if nest_first else reversed(imports))

    cmd = shlex.split(f'python3 -c "{import_cmds}; nest.Simulate(10)"')
    my_env = os.environ.copy()
    exit_code = subprocess.call(cmd, env=my_env)

    if nest.ll_api.sli_func("statusdict/have_music ::"):
        assert exit_code == EXIT_CODE_ERROR
    else:
        assert exit_code == EXIT_CODE_SUCCESS
