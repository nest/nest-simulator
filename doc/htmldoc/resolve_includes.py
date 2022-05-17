# -*- coding: utf-8 -*-
#
# resolve_includes.py
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


"""Script to replace include roles in RST files by included file.

To increase the modularity and reusability of documentation snippets,
model documentation files might be composed from multiple different
reStructuredText files by means of '.. include::' roles.

In order to make the non-rendered help texts also complete and useful
when viewed via PyNEST's help() command, we replace lines with
'include' roles by the content of the included file.

"""

import os
import re
import sys
import glob
from fileinput import FileInput

pattern = re.compile('^.. include:: (.*)')
path = sys.argv[1]

for rst_fname in glob.glob(os.path.join(path, 'spike_recorder.rst')):
    with FileInput(rst_fname, inplace=True, backup='.bak') as rst_file:
        for line in rst_file:
            match = pattern.match(line)
            if match:
                include_fname = os.path.join(path, match.group(1))
                with open(include_fname) as include_file:
                    print(include_file.read(), end='')
            else:
                print(line, end='')
