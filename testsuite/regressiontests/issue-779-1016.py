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

from subprocess import check_output, STDOUT
from os.path import join
from tempfile import mktemp
from sys import exit, version_info

EXIT_SUCCESS = 0
EXIT_FAILURE = 126


def decode(arg):
    if version_info < (3,):
        return arg
    else:
        return arg.decode("utf8")


nestscript = mktemp(".sli")
nestcmd = ["nest", "-d", "--verbosity=ALL", nestscript]

with open(nestscript, "w") as f:
    f.write("statusdict/argv :: ==")

raw_output = check_output(nestcmd, stderr=STDOUT)
output = [x for x in decode(raw_output).split("\n") if x != ""]

expected = "[(" + ") (".join(nestcmd) + ")]"

if output[-1] == expected:
    exit(EXIT_SUCCESS)
else:
    exit(EXIT_FAILURE)
