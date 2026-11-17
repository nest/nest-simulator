# -*- coding: utf-8 -*-
#
# add_header_tus.py
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
Extend a compilation database with one stub translation unit per header.

include-what-you-use (IWYU) analyses a source file ``foo.cpp`` together with its
header ``foo.h``. Headers without such a source file (header-only code and the
``*_impl.h`` files with template definitions) would never be analysed. For each
of them, this script writes a stub ``<out-dir>/<dir>/<name>.iwyu.cpp`` that only
includes the header with ``// IWYU pragma: associated``. This also checks that
every header compiles on its own.

Usage: add_header_tus.py <build-dir> <source-dir> <out-dir> <subdir> ...

Writes ``<out-dir>/compile_commands.json`` with the entries of
``<build-dir>/compile_commands.json`` plus the stubs. Each stub is compiled with
the flags of a source file from the same subdirectory.
"""

import json
import os
import shlex
import sys


def main(build_dir, source_dir, out_dir, subdirs):
    with open(os.path.join(build_dir, "compile_commands.json")) as f:
        database = json.load(f)

    template = {}
    for entry in database:
        subdir = os.path.relpath(os.path.dirname(entry["file"]), source_dir)
        template.setdefault(subdir, entry)

    entries = list(database)
    for subdir in subdirs:
        tmpl = template[subdir]
        for name in sorted(os.listdir(os.path.join(source_dir, subdir))):
            if not name.endswith(".h"):
                continue
            stem = name[:-2]
            if os.path.exists(os.path.join(source_dir, subdir, stem + ".cpp")):
                # Analysed together with its .cpp (or the .cpp is not built in this configuration).
                continue
            stub = os.path.join(out_dir, subdir, stem + ".iwyu.cpp")
            os.makedirs(os.path.dirname(stub), exist_ok=True)
            with open(stub, "w") as f:
                f.write(f'#include "{os.path.join(source_dir, subdir, name)}" // IWYU pragma: associated\n')

            args = shlex.split(tmpl["command"]) if "command" in tmpl else list(tmpl["arguments"])
            args = [stub if a == tmpl["file"] else a for a in args]
            if "-o" in args:
                args[args.index("-o") + 1] = stub + ".o"
            entries.append({"directory": tmpl["directory"], "file": stub, "arguments": args})

    with open(os.path.join(out_dir, "compile_commands.json"), "w") as f:
        json.dump(entries, f, indent=1)
    print(f"Added {len(entries) - len(database)} header stub translation units.")


if __name__ == "__main__":
    if len(sys.argv) < 5:
        sys.exit(__doc__)
    main(os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2]), os.path.abspath(sys.argv[3]), sys.argv[4:])
