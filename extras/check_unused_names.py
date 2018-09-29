# -*- coding: utf-8 -*-
#
# check_unused_names.py
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

from __future__ import print_function

import os
import sys
import re
from subprocess import check_output


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


# Use encoding-aware Py3 open also in Py2
if sys.version_info[0] < 3:
    from io import open

EXIT_SUCCESS = 0
EXIT_NAME_H_CPP_MISMATCH = 30
EXIT_UNUSED_NAME = 31
EXIT_NO_SOURCE = 126

try:
    source_dir = os.environ['NEST_SOURCE']
except KeyError:
    eprint("Please make NEST_SOURCE environment variable to point to " +
           "the source tree you want to check!")
    sys.exit(EXIT_NO_SOURCE)

# Base names of files that contain const Name definitions
# (i.e. filenames without filename extension)
names_files = [
    "librandom/librandom_names",
    "nestkernel/nest_names",
    "topology/topology_names",
]


def get_names(fname, pattern):
    names = []
    with open(fname) as names_file:
        lines = names_file.readlines()
        for line in lines:
            match = re.search(pattern, line)
            if match:
                names.append(match.group(1))
    names.sort()
    return names


names_defined = set()
for names_file in names_files:
    fname = os.path.join(source_dir, names_file)

    names_header = get_names(fname + ".h", "extern\s+const\s+Name\s+(\w+)\s*;")
    names_source = get_names(fname + ".cpp", "const\s+Name\s+(\w+)\(.*")

    for h, s in zip(names_header, names_source):
        if h != s:
            eprint("[NAME] {}: inconsistent declaration: {} != {}".format(
                names_file, h, s))
            print("... {}\\n".format(names_file))
            sys.exit(EXIT_NAME_H_CPP_MISMATCH)
        else:
            names_defined.add(h)


# We call to recursive grep in the shell here, because that's much
# simpler than recursive file iteration and regex matching in Python
grep_cmd = ["grep", "-ro", "names::[A-Za-z0-9_]*", source_dir]
names_used_raw = check_output(grep_cmd)

if isinstance(names_used_raw, bytes):
    names_used_raw = str(names_used_raw.decode())

names_used = set()
for line in names_used_raw.split("\n"):
    if "::" in line:
        name = line.split("::")[1]
        if name != "":
            names_used.add(name)


names_unused = names_defined - names_used
if len(names_unused) != 0:
    msg = "unused Name definition(s): " + ", ".join(names_unused)
    eprint("[NAME] " + msg)
    print("... {}\\n".format(msg))
    sys.exit(EXIT_UNUSED_NAME)


sys.exit(EXIT_SUCCESS)
