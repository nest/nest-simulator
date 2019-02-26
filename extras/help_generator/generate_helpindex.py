#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# generate_helpindex.py
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
Generate NEST helpindex
=======================

Generate the helpindex containing all help files in the given
help_dir.
"""

import os
import sys
from writers import write_helpindex

if len(sys.argv) != 2:
    print("Usage: python generate_helpindex.py <help_dir>")
    sys.exit(1)

help_dir = os.path.join(sys.argv[1], "help")
write_helpindex(help_dir)
