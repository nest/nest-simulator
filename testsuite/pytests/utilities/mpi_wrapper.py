# -*- coding: utf-8 -*-
#
# mpi_wrapper.py
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

import ast
import functools
import inspect
import os
import subprocess
import sys
import tempfile
from pathlib import Path

import numpy as np
import pandas as pd
import pytest
from decorator import decorator

# from mpi4py import MPI


class DecoratorParserBase:
    """
    Base class that parses the test module to retrieve imports, test code and
    test parametrization.
    """

    def __init__(self):
        pass

    def _parse_import_statements(self, caller_fname):
        with open(caller_fname, "r") as f:
            tree = ast.parse(f.read(), caller_fname)

        modules = []
        for node in ast.walk(tree):
            if isinstance(node, (ast.Import, ast.ImportFrom)) and not node.names[0].name.startswith("mpi_assert"):
                modules.append(ast.unparse(node).encode())
        return modules

    def _parse_func_source(self, func):
        lines, _ = inspect.getsourcelines(func)
        # remove decorators and encode
        func_src = [line.encode() for line in lines if not line.startswith("@")]
        return func_src

    def _params_as_str(self, *args, **kwargs):
        params = ""
        if args:
            params += ", ".join(f"{arg}" for arg in args)
        if kwargs:
            if args:
                params += ", "
            params += ", ".join(f"{key}={value}" for key, value in kwargs.items())
        return params


class mpi_assert_equal_df(DecoratorParserBase):
    """
    docs
    """

    def __init__(self, procs_lst):
        if not isinstance(procs_lst, list):
            # TODO: Instead of passing the number of MPI procs to run explicitly, i.e., [1, 2, 4], another
            # option is to pass the max number, e.g., 4, and handle the range internally
            msg = "'mpi_assert_equal_df' decorator requires the number of MPI procs to test to be passed as a list"
            raise TypeError(msg)

        # TODO: check if len(procs_lst) >= 2 (not yet implemented for debugging purposes)

        self._procs_lst = procs_lst
        self._caller_fname = inspect.stack()[1].filename

    def __call__(self, func):
        def wrapper(func, *args, **kwargs):
            # TODO: replace path setup below with the following:
            # with tempfile.TemporaryDirectory() as tmpdir:
            self._path = Path("./tmpdir")
            self._path.mkdir(parents=True, exist_ok=True)

            # Write the relevant code from test module to a new, temporary (TODO)
            # runner script
            with open(self._path / "runner.py", "wb") as fp:
                self._write_runner(fp, func, *args, **kwargs)

            for procs in self._procs_lst:
                # TODO: MPI and subprocess does not seem to play well together
                res = subprocess.run(["mpirun", "-np", str(procs), "python", "runner.py"], check=True, cwd=self._path)

                print(res)

            self._check_equal()

        # Here we need to assert that dfs from all runs are equal

        return decorator(wrapper, func)

    def _main_block_equal_df(self, func, *args, **kwargs):
        main_block = "\n"
        main_block += "if __name__ == '__main__':"
        main_block += "\n\t"
        main_block += f"df = {func.__name__}({self._params_as_str(*args, **kwargs)})"
        main_block += "\n\t"

        main_block += f"path = '{self._path}'"
        main_block += "\n\t"

        # Write output df to csv (will be compared later)
        # main_block += "df.to_csv(f'{path}/df_{nest.NumProcesses()}-{nest.Rank()}.csv', index=False)"

        return main_block.encode()

    def _write_runner(self, fp, func, *args, **kwargs):
        # TODO: most of this can probably be in base class
        fp.write(b"\n".join(self._parse_import_statements(self._caller_fname)))
        fp.write(b"\n\n")
        fp.write(b"".join(self._parse_func_source(func)))

        # TODO: only the main block needs to be changed between runner runs
        fp.write(self._main_block_equal_df(func, *args, **kwargs))

    def _check_equal(self):
        res = [
            pd.concat(pd.read_csv(f, sep="\t", comment="#") for f in self._path.glob(f"sr_{n:02d}*.dat")).sort_values(
                by=["time_ms", "sender"]
            )
            for n in self._procs_lst
        ]

        for r in res[1:]:
            pd.testing.assert_frame_equal(res[0], r)
