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

import inspect
import subprocess
import tempfile
import textwrap
from pathlib import Path

import pandas as pd
import pytest
from decorator import decorator


class MPITestWrapper:
    """-
    Base class that parses the test module to retrieve imports, test code and
    test parametrization.
    """

    RUNNER = "runner.py"
    SPIKE_LABEL = "spikes-{}"

    RUNNER_TEMPLATE = textwrap.dedent(
        """\
        SPIKE_LABEL = '{spike_lbl}'

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

    def _func_without_decorators(self, func):
        return "".join(line for line in inspect.getsourcelines(func)[0] if not line.startswith("@"))

    def _params_as_str(self, *args, **kwargs):
        return ", ".join(
            part
            for part in (
                ", ".join(f"{arg}" for arg in args),
                ", ".join(f"{key}={value}" for key, value in kwargs.items()),
            )
            if part
        )

    def _write_runner(self, tmpdirpath, func, *args, **kwargs):
        with open(tmpdirpath / self.RUNNER, "w") as fp:
            fp.write(
                self.RUNNER_TEMPLATE.format(
                    spike_lbl=self.SPIKE_LABEL,
                    fcode=self._func_without_decorators(func),
                    fname=func.__name__,
                    params=self._params_as_str(*args, **kwargs),
                )
            )

    def __call__(self, func):
        def wrapper(func, *args, **kwargs):
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
                print(f"\n\nTMPDIR: {tmpdirpath}\n\n")
                print(res)

            self.collect_results(tmpdirpath)
            self.assert_correct_results()

        return decorator(wrapper, func)

    def collect_results(self, tmpdirpath):
        self._spikes = {
            n_procs: [
                pd.read_csv(f, sep="\t", comment="#")
                for f in tmpdirpath.glob(f"{self.SPIKE_LABEL.format(n_procs)}-*.dat")
            ]
            for n_procs in self._procs_lst
        }

    def assert_correct_results(self):
        assert False, "Test-specific checks not implemented"


class MPITestAssertEqual(MPITestWrapper):
    """
    Assert that combined, sorted output from all VPs is identical for all numbers of MPI ranks.
    """

    def assert_correct_results(self):
        res = [
            pd.concat(spikes).sort_values(by=["time_step", "time_offset", "sender"]) for spikes in self._spikes.values()
        ]

        for r in res[1:]:
            pd.testing.assert_frame_equal(res[0], r)
