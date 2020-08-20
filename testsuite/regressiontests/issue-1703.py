# -*- coding: utf-8 -*-
#
# issue-1703.py
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

import sys
import subprocess as sp
import shlex

EXIT_SUCCESS = 0
EXIT_SKIPPED = 200
EXIT_FAILURE = 127
EXIT_SEGFAULT = 139

try:
    import nest
    from mpi4py import MPI
except ImportError:
    # Skip test if mpi4py is not installed, or if NEST is
    # installed without Python support.
    sys.exit(EXIT_SKIPPED)

cmd = shlex.split('python -c "from mpi4py import MPI; import nest; nest.Simulate(10)"')
exit_code = sp.call(cmd)

if nest.ll_api.sli_func("statusdict/have_music ::"):
    # Expect error, not segfault
    if exit_code == 1:
        sys.exit(EXIT_SUCCESS)
    elif exit_code == -11:
        sys.exit(EXIT_SEGFAULT)
    else:
        sys.exit(EXIT_FAILURE)
else:
    if exit_code == 0:
        sys.exit(EXIT_SUCCESS)
    elif exit_code == -11:
        sys.exit(EXIT_SEGFAULT)
    else:
        sys.exit(EXIT_FAILURE)
