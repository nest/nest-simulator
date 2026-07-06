# -*- coding: utf-8 -*-
#
# generate_exception_header.py
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

"""Script to generate a list of all exceptions used in NEST.

This script is called during the run of CMake and generates a C++
header file with a single variable, nest_exceptions, that lists all
exceptions used in NEST by their name. This file is used by the
wrapper to map all of NEST's exceptions into Python exceptions and
avoid a redundant specification therein.

"""

import argparse
from pathlib import Path


def parse_commandline():
    """Parse the commandline arguments and put them into variables.

    There are two arguments to this script that can be given either as
    positional arguments or by their name.

    1. srcdir: the path to the top-level NEST source directory
    2. blddir: the path to the NEST build directory (-DCMAKE_INSTALL_PREFIX)

    This function does not return anything, but instead it checks the
    commandline arguments and makes them available as global variables
    of the script as given.
    """

    global srcdir, blddir

    description = "Generate a header listing all NEST exceptions."
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("srcdir", type=str, help="the source directory of NEST")
    parser.add_argument("blddir", type=str, help="the build directory of NEST")
    args = parser.parse_args()

    srcdir = args.srcdir
    blddir = args.blddir


def generate_exception_header():
    """Write the exception list out to file.

    This is a very straightforward function that prints the copyright
    header followed by a std::vector<std::string> with the names of
    all NEST exceptions to `blddir/pynest/nest_exception_list.h`.
    `blddir` is handed as a commandline argument to the script.

    """

    fname = Path(srcdir) / "doc" / "copyright_header.cpp"
    with open(fname, "r") as file:
        copyright_header = file.read()

    exceptions = []
    fname = Path(srcdir) / "nestkernel" / "exceptions.h"
    with open(fname, "r") as file:
        for line in file:
            if line.strip().endswith(": public KernelException"):
                exceptions.append(line.split()[1])

    fname = "nest_exception_names.h"
    pynestdir = Path(blddir) / "pynest"
    pynestdir.mkdir(parents=True, exist_ok=True)
    with open(pynestdir / fname, "w") as file:
        file.write(copyright_header.replace("{{file_name}}", fname))
        file.write("\nstd::vector< std::string > nest_exceptions = {\n")
        for exception in exceptions:
            file.write(f'  "{exception}",\n')
        file.write("};")


if __name__ == "__main__":
    parse_commandline()
    generate_exception_header()
