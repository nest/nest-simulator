# -*- coding: utf-8 -*-
#
# issue-779-1016.py
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
This script ensures that NEST parses commandline arguments correctly
and makes all of them available in the argv array in the statusdict.

This is a regression test for GitHub issues 779 and 1016.
"""

import sys
from subprocess import STDOUT, check_output
from tempfile import mktemp

EXIT_SUCCESS = 0
EXIT_FAILURE = 126

nestscript = mktemp(".sli")
nestcmd = ["nest", "-d", "--verbosity=ALL", nestscript]

with open(nestscript, "w") as f:
    f.write("statusdict/argv :: == M_FATAL setverbosity")

raw_output = check_output(nestcmd, stderr=STDOUT)
output = [x for x in raw_output.decode("utf-8").split("\n") if x != ""]

expected = "[(" + ") (".join(nestcmd) + ")]"

if output[-1] == expected:
    sys.exit(EXIT_SUCCESS)
else:
    sys.exit(EXIT_FAILURE)
