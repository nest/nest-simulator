# -*- coding: utf-8 -*-
#
# mpi_test_wrapper.py
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
Support for NEST-style MPI Tests.

NEST-style MPI tests run the same simulation script for different number of MPI
processes and then compare results. Often, the number of virtual processes will
be fixed while the number of MPI processes is varied, but this is not required.

- The process is managed by subclasses of the `MPITestWrapper` base class
- Each test file must contain exactly one test function
    - The test function must be decorated with a subclass of `MPITestWrapper`
    - The wrapper will write a modified version of the test file as `runner.py`
      to a temporary directory and mpirun it from there; results are collected
      in the temporary directory
    - The test function can be decorated with other pytest decorators. These
      are evaluated in the wrapping process
    - No decorators are written to the `runner.py` file.
    - Test files **must not import nest** outside the test function
    - In `runner.py`, the following constants are defined:
         - `SPIKE_LABEL`
         - `MULTI_LABEL`
         - `OTHER_LABEL`
      They must be used as `label` for spike recorders and multimeters, respectively,
      or for other files for output data (CSV files). They are format strings expecting
      the number of processes with which NEST is run as argument.
- `conftest.py` must not be loaded, otherwise mpirun will return a non-zero exit code;
  use `pytest --noconftest`
- Set `debug=True` on the decorator to see debug output and keep the
  temporary directory that has been created (latter works only in
  Python 3.12 and later)
- Evaluation criteria are determined by the `MPITestWrapper` subclass

This is still work in progress.
"""

import ast
import inspect
import subprocess
import tempfile
import textwrap
from functools import wraps
from pathlib import Path

import pandas as pd
import pytest


class _RemoveDecoratorsAndMPITestImports(ast.NodeTransformer):
    """
    Remove any decorators set on function definitions and imports of MPITest* entities.

    Returning None (falling off the end) of visit_* deletes a node.
    See https://docs.python.org/3/library/ast.html#ast.NodeTransformer for details.

    """

    def visit_FunctionDef(self, node):
        """Remove any decorators"""

        node.decorator_list.clear()
        return node

    def visit_Import(self, node):
        """Drop import"""
        if not any(alias.name.startswith("MPITest") for alias in node.names):
            return node

    def visit_ImportFrom(self, node):
        """Drop from import"""
        if not any(alias.name.startswith("MPITest") for alias in node.names):
            return node


class MPITestWrapper:
    """
    Base class that parses the test module to retrieve imports, test code and
    test parametrization.
    """

    RUNNER = "runner.py"
    SPIKE_LABEL = "spike-{}"
    MULTI_LABEL = "multi-{}"
    OTHER_LABEL = "other-{}"

    RUNNER_TEMPLATE = textwrap.dedent(
        """\
        SPIKE_LABEL = '{spike_lbl}'
        MULTI_LABEL = '{multi_lbl}'
        OTHER_LABEL = '{other_lbl}'

        {fcode}

        if __name__ == '__main__':
            {fname}({params})
        """
    )

    def __init__(self, procs_lst, debug=False):
        try:
            iter(procs_lst)
        except TypeError:
            raise TypeError("procs_lst must be a list of numbers")

        self._procs_lst = procs_lst
        self._debug = debug
        self._spike = None
        self._multi = None
        self._other = None

    @staticmethod
    def _pure_test_func(func):
        source_file = inspect.getsourcefile(func)
        tree = ast.parse(open(source_file).read())
        _RemoveDecoratorsAndMPITestImports().visit(tree)
        return ast.unparse(tree)

    def _params_as_str(self, *args, **kwargs):
        return ", ".join(
            part
            for part in (
                ", ".join(f"{arg if not inspect.isfunction(arg) else arg.__name__}" for arg in args),
                ", ".join(
                    f"{key}={value if not inspect.isfunction(value) else value.__name__}"
                    for key, value in kwargs.items()
                ),
            )
            if part
        )

    def _write_runner(self, tmpdirpath, func, *args, **kwargs):
        with open(tmpdirpath / self.RUNNER, "w") as fp:
            fp.write(
                self.RUNNER_TEMPLATE.format(
                    spike_lbl=self.SPIKE_LABEL,
                    multi_lbl=self.MULTI_LABEL,
                    other_lbl=self.OTHER_LABEL,
                    fcode=self._pure_test_func(func),
                    fname=func.__name__,
                    params=self._params_as_str(*args, **kwargs),
                )
            )

    def __call__(self, func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            # "delete" parameter only available in Python 3.12 and later
            try:
                tmpdir = tempfile.TemporaryDirectory(delete=not self._debug)
            except TypeError:
                tmpdir = tempfile.TemporaryDirectory()

            # TemporaryDirectory() is not os.PathLike, so we need to define a Path explicitly
            # To ensure that tmpdirpath has the same lifetime as tmpdir, we define it as a local
            # variable in the wrapper() instead of as an attribute of the decorator.
            tmpdirpath = Path(tmpdir.name)
            self._write_runner(tmpdirpath, func, *args, **kwargs)

            res = {}
            for procs in self._procs_lst:
                res[procs] = subprocess.run(
                    ["mpirun", "-np", str(procs), "--oversubscribe", "python", self.RUNNER],
                    check=True,
                    cwd=tmpdirpath,
                    capture_output=self._debug,
                )

            if self._debug:
                print("\n\n")
                print(res)
                print(f"\n\nTMPDIR: {tmpdirpath}\n\n")

            self.assert_correct_results(tmpdirpath)

        return wrapper

    def _collect_result_by_label(self, tmpdirpath, label):
        try:
            next(tmpdirpath.glob(f"{label.format('*')}.dat"))
        except StopIteration:
            return None  # no data for this label

        return {
            n_procs: [pd.read_csv(f, sep="\t", comment="#") for f in tmpdirpath.glob(f"{label.format(n_procs)}-*.dat")]
            for n_procs in self._procs_lst
        }

    def collect_results(self, tmpdirpath):
        """
        For each of the result types, build a dictionary mapping number of MPI procs to a list of
        dataframes, collected per rank or VP.
        """

        self._spike = self._collect_result_by_label(tmpdirpath, self.SPIKE_LABEL)
        self._multi = self._collect_result_by_label(tmpdirpath, self.MULTI_LABEL)
        self._other = self._collect_result_by_label(tmpdirpath, self.OTHER_LABEL)

    def assert_correct_results(self, tmpdirpath):
        assert False, "Test-specific checks not implemented"


class MPITestAssertEqual(MPITestWrapper):
    """
    Assert that combined, sorted output from all VPs is identical for all numbers of MPI ranks.
    """

    def assert_correct_results(self, tmpdirpath):
        self.collect_results(tmpdirpath)

        all_res = []
        if self._spike:
            # For each number of procs, combine results across VPs and sort by time and sender
            all_res.append(
                [
                    pd.concat(spikes, ignore_index=True).sort_values(
                        by=["time_step", "time_offset", "sender"], ignore_index=True
                    )
                    for spikes in self._spike.values()
                ]
            )

        if self._multi:
            raise NotImplementedError("MULTI is not ready yet")

        if self._other:
            # For each number of procs, combine across ranks or VPs (depends on what test has written) and
            # sort by all columns so that if results for different proc numbers are equal up to a permutation
            # of rows, the sorted frames will compare equal

            # next(iter(...)) returns the first value in the _other dictionary
            # [0] then picks the first DataFrame from that list
            # columns need to be converted to list() to be passed to sort_values()
            all_columns = list(next(iter(self._other.values()))[0].columns)
            all_res.append(
                [
                    pd.concat(others, ignore_index=True).sort_values(by=all_columns, ignore_index=True)
                    for others in self._other.values()
                ]
            )

        assert all_res, "No test data collected"

        for res in all_res:
            assert len(res) == len(self._procs_lst), "Could not collect data for all procs"

            for r in res[1:]:
                pd.testing.assert_frame_equal(res[0], r)
