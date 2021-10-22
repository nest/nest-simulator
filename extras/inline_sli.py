#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# inline.py
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

import re
import sys
from pathlib import Path

SLI_LIBPATH = Path(__file__).parent.parent / "lib" / "sli"


def replaced(filename, noinclude=None):
    '''
    Recursively replace "(xy) run" SLI commands.
    '''
    if noinclude is None:
        noinclude = []
    sliimport = re.compile(r'\((?P<name>[^)]+)\) run')
    for no, line in enumerate(filename.open('r', encoding="utf8")):
        found = False
        for match in sliimport.finditer(line):
            incfile = (SLI_LIBPATH / match.group('name')).with_suffix(".sli")
            if incfile in noinclude:
                continue
            if not incfile.is_file():
                yield(f"%%% INSERT FAILED FOR >>> {incfile} <<<\n")
                yield(f"%%% {line}")
                continue
            yield(f"%%% INSERT {incfile} LITERALLY\n")
            yield(f"%%% L{no}: {line}")
            noinclude.append(incfile)
            for includedline in replaced(incfile, noinclude):
                yield includedline.replace('"', "'")
            yield(f"%%% END {incfile}\n")
            found = True
        if not found:
            yield f"{line}"


def inline(files):
    '''
    SLI code inlining.
    '''
    for arg in files:
        filename = Path(arg)
        newfile = filename.with_suffix(".new")
        with newfile.open("w", encoding="utf8") as outfile:
            for line in replaced(filename):
                outfile.write(line)
        newfile.rename(filename)
