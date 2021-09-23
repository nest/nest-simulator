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
# Pull nest-simulator and initiate Frankenstein assembly of wheel env
subprocess.run(['git', 'clone', 'git@github.com:nest/nest-simulator', wheel_path, '--depth=1'], check=True)
# Go fish `setup.py`
shutil.copy2(pynest_path / "setup.py", wheel_path)
# Go fish nest python code and intermingle it with the regular `nest` cpp folder
copy_tree(str(module_path), str(wheel_path / "nest"))
