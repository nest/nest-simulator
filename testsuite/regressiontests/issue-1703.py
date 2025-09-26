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

import os
import shlex
import subprocess
import sys

import nest

EXIT_CODE_SUCCESS = 0
EXIT_CODE_ERROR = 1
EXIT_CODE_SEGFAULT = -11

EXIT_SUCCESS = 0
EXIT_SKIPPED = 200
EXIT_FAILURE = 127
EXIT_SEGFAULT = 139

# Check that NEST is installed with MPI support and mpi4py is available.
# Check that NEST is installed with MPI support and mpi4py is available.
# If mpi4py is missing, we get an ImportError
# If mpi4py is installed but libmpi is missing, we get a RuntimeError.
# This only happens if we explicitly import MPI.
try:
    from mpi4py import MPI

    HAVE_MPI4PY = True
except (ImportError, RuntimeError):
    HAVE_MPI4PY = False

if not HAVE_MPI4PY:
    sys.exit(EXIT_SKIPPED)

# Attempt to import MPI from mpi4py before NEST. Running the script in a separate process,
# in case it ends in a segmentation fault.
cmd = shlex.split('python3 -c "from mpi4py import MPI; import nest; nest.Simulate(10)"')
my_env = os.environ.copy()
exit_code = subprocess.call(cmd, env=my_env)

if nest.build_info["have_music"]:
    # Expect error, not segfault
    if exit_code == EXIT_CODE_ERROR:
        sys.exit(EXIT_SUCCESS)
    elif exit_code == EXIT_CODE_SEGFAULT:
        sys.exit(EXIT_SEGFAULT)
    else:
        sys.exit(EXIT_FAILURE)
else:
    if exit_code == EXIT_CODE_SUCCESS:
        sys.exit(EXIT_SUCCESS)
    elif exit_code == EXIT_CODE_SEGFAULT:
        sys.exit(EXIT_SEGFAULT)
    else:
        sys.exit(EXIT_FAILURE)
