# -*- coding: utf-8 -*-
#
# clean_source_dirs.py
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
import pathlib
import shutil
from glob import glob

for dir_ in ("auto_examples", "models"):
    for file_ in glob(str(pathlib.Path(__file__).parent / dir_ / "*")):
        if not any(file_.endswith(f) for f in (".gitignore", "index.rst")):
            try:
                try:
                    os.unlink(file_)
                except OSError:
                    shutil.rmtree(file_)
            except Exception:
                print(f"Couldn't remove '{file_}'")
