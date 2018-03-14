# -*- coding: utf-8 -*-
#
# test_unused_names.py
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

import os
import sys

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
    print("Please make NEST_SOURCE environment variable to point to " +
          "the source tree you want to check!")
    sys.exit(EXIT_NO_SOURCE)

# Base names of files that contain const Name definitions
# (i.e. filenames without filename extension)
names_files = [
    "librandom/librandom_names",
    "nestkernel/nest_names",
    "topology/topology_names",
]

names_defined = set()
for names_file in names_files:
    fname = os.path.join(source_dir, names_file)

    names_header = []
    with open(fname + ".h") as names_file:
        lines = names_file.readlines()
        for line in lines:
            if "extern const Name" in line:
                names_header.append(line.split("Name ")[1].split(";")[0])
    names_header.sort()

    names_source = []
    with open(fname + ".cpp") as names_file:
        lines = names_file.readlines()
        for line in lines:
            if "const Name" in line:
                names_source.append(line.split("Name ")[1].split("(")[0])
    names_source.sort()

    for h, s in zip(names_header, names_source):
        if h != s:
            msg = "Inconsistent Name definition/declaration: "
            print(msg + "{} != {}".format(h, s))
            sys.exit(EXIT_NAME_H_CPP_MISMATCH)
        else:
            names_defined.add(h)

names_defined = list(names_defined)
names_defined.sort()


from subprocess import check_output

grep_cmd = ["grep", "-ro", "names::[A-Za-Z0-9_]*", source_dir]
names_used_raw = check_output(grep_cmd)

names_used = set()
for line in names_used_raw.split("\n"):
    if "::" in line:
        name = line.split("::")[1]
        if name != "":
            names_used.add(name)

names_used = list(names_used)
names_used.sort()

for d, u in zip(names_defined, names_used):
    if d != u:
        print("Unused Name definition: {}".format(d))
        sys.exit(EXIT_UNUSED_NAME)

sys.exit(EXIT_SUCCESS)
