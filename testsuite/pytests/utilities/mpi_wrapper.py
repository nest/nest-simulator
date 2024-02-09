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
import textwrap
from pathlib import Path

import numpy as np
import pandas as pd
import pytest
from decorator import decorator


class MPIWrapper:
    """
    Base class that parses the test module to retrieve imports, test code and
    test parametrization.
    """

    def __init__(self, procs_lst):
        try:
            iter(procs_lst)
        except TypeError:
            raise TypeError("procs_lst must be a list of numbers")

        self._procs_lst = procs_lst
        self._caller_fname = inspect.stack()[1].filename
        self._tmpdir = None

    def _parse_func_source(self, func):
        func_src = (line.encode() for line in inspect.getsourcelines(func)[0] if not line.startswith("@"))
        return func_src

    def _params_as_str(self, *args, **kwargs):
        return ", ".join(
            part
            for part in (
                ", ".join(f"{arg}" for arg in args),
                ", ".join(f"{key}={value}" for key, value in kwargs.items()),
            )
            if part
        )

    def _main_block(self, func, *args, **kwargs):
        main_block = f"""
           if __name__ == '__main__':
               {func.__name__}({self._params_as_str(*args, **kwargs)})
               """

        return textwrap.dedent(main_block).encode()

    def _write_runner(self, func, *args, **kwargs):
        with open(self._tmpdir / "runner.py", "wb") as fp:
            fp.write(b"".join(self._parse_func_source(func)))
            fp.write(self._main_block(func, *args, **kwargs))

    def __call__(self, func):
        def wrapper(func, *args, **kwargs):
            with tempfile.TemporaryDirectory() as tmpdirname:
                self._tmpdir = Path(tmpdirname)
                self._write_runner(func, *args, **kwargs)

                for procs in self._procs_lst:
                    subprocess.run(["mpirun", "-np", str(procs), "python", "runner.py"], check=True, cwd=self._tmpdir)

                self.assert_correct_results()

        return decorator(wrapper, func)

    def assert_correct_results():
        assert False, "Test-specific checks not implemented"


class MPIAssertEqual(MPIWrapper):
    """
    Assert that combined, sorted output from all VPs is identical for all numbers of MPI ranks.
    """

    def assert_correct_results(self):
        res = [
            pd.concat(pd.read_csv(f, sep="\t", comment="#") for f in self._tmpdir.glob(f"sr_{n:02d}*.dat")).sort_values(
                by=["time_ms", "sender"]
            )
            for n in self._procs_lst
        ]

        for r in res[1:]:
            pd.testing.assert_frame_equal(res[0], r)
