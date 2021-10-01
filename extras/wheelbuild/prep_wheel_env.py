# prep_wheel_env.py
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

import subprocess
import re
from pathlib import Path
import sys
import os
import shutil
from _vendored import copy_tree

# A tiny build system for the build system of our build system. Patent pending.

built_path = Path("build")
wheel_path = Path(sys.argv[1])

def fish_cmake_vars(*vars):
    """
    Go fishing in the output of `cmake -LAH` for CMAKE variables.

    Assumes you're in the build folder.
    """
    pat = re.compile(f"^({'|'.join(vars)})\\:\\w+=(.+)")
    p = subprocess.run(f'cmake -LAH', shell=True, capture_output=True)
    values = [m.group(2) for o in p.stdout.decode().split("\n") if (m := pat.match(o))]
    return values

def get_origin_url():
    p = subprocess.run(
        "git remote -v"
        + " | grep origin"
        + " | grep fetch",
        shell=True,
        capture_output=True
    )
    remote = p.stdout.decode().split("\t")[1].split(" ")[0]
    return remote

def get_current_commit():
    p = subprocess.run(
        "git rev-parse HEAD",
        shell=True,
        capture_output=True
    )
    return p.stdout.decode().split("\n")[0]

curr = Path.cwd()
# Go to the CMake build folder to fish out the CMade Python files from the build files
os.chdir(built_path)
# Collect all the CMade pieces
vars = ("CMAKE_INSTALL_PREFIX", "PYEXECDIR")
cmip, pkgdir = vals = fish_cmake_vars(*vars)
print("Found CMake vars:", "\n".join(f"{k}={v}" for k, v in zip(vars, vals)), end="\n")
cmake_path = Path(cmip)
pynest_path = built_path / "pynest"
module_path = cmake_path / pkgdir / "nest"
# Go back to the CI root dir
os.chdir(curr)
if "GHA" not in sys.argv:
    # Pull nest-simulator and initiate Frankenstein assembly of wheel env
    remote = get_origin_url()
    commit = get_current_commit()
    print("Cloning NEST repo from", remote)
    subprocess.run(['git', 'clone', remote, wheel_path], check=True)
    os.chdir(wheel_path)
    print("Checking out", commit)
    subprocess.run(['git', 'checkout', commit], check=True)
    os.chdir(curr)
# Go fish `setup.py`
shutil.copy2(pynest_path / "setup.py", wheel_path)
# Go fish nest python code and intermingle it with the regular `nest` cpp folder
print(f"Copying Python files from `{module_path}`")
copy_tree(str(module_path), str(wheel_path / "nest"))
