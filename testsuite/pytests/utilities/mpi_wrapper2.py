# -*- coding: utf-8 -*-
#
# mpi_wrapper2.py
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

import inspect
import os
import subprocess
from functools import wraps


# NOTE: This approach doesn't seem to work
def mpi_wrapper(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        num_processes = 1

        file = inspect.getfile(func)
        module = func.__module__
        path = os.path.abspath(file)

        print("module.count('.'):", module.count("."))

        for _ in range(module.count(".")):
            print("path =", path)
            path = os.path.split(path)[0]

        command = [
            "mpiexec",
            "-n",
            str(num_processes),
            "-wdir",
            path,
            "python",
            "-c",
            f"from {module} import *; {func.__name__}()",
        ]

        # f"import {module} as module; module.{func.__name__}()",

        print(func.__name__)
        # print(module.func.__name__.original())

        subprocess.run(command, capture_output=True, check=True, env=os.environ)

        print("args:", args, "kwargs:", kwargs)

        # return func(*args, **kwargs)

    return wrapper
