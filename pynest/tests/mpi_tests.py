# -*- coding: utf-8 -*-
#
# test_connect_all_patterns.py
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

from __future__ import print_function
import sys
import os
import subprocess
import shlex
from tempfile import mktemp


def check_output(cmd):
    return subprocess.check_output(shlex.split(cmd)).decode("utf-8")


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


cmd = "sli -c 'statusdict/have_mpi :: =only'"
HAVE_MPI = check_output(cmd) == "true"

if not HAVE_MPI:
    eprint("Script cannot be run if NEST was compiled without MPI. Exiting.")
    sys.exit(1)


script_dir = os.path.dirname(os.path.realpath(__file__))

scripts = [
    "test_connect_all_to_all.py",
    "test_connect_one_to_one.py",
    "test_connect_fixed_indegree.py",
    "test_connect_fixed_outdegree.py",
    "test_connect_fixed_total_number.py",
    "test_connect_pairwise_bernoulli.py",
    "test_sp/mpitest_issue_578_sp.py",
]


failing_scripts = []
failing_outputs = []

with open(sys.argv[1], "w") as junitxml:
    for script_name in scripts:
        script = os.path.join(script_dir, script_name)

        tmpfile = mktemp(".xml")
        pytest_cmd = "python {} --junitxml={}".format(sys.argv[2], tmpfile)
        cmd = "nest -c '2 ({}) ({}) mpirun =only'".format(pytest_cmd, script)
        test_cmd = check_output(cmd)

        try:
            output = check_output(test_cmd)
            eprint(output)
        except subprocess.CalledProcessError as e:
            failing_scripts.append(script_name)
            failing_outputs.append(e.output)
            continue

        junitxml.write(open(tmpfile, "r").read())

        if os.path.exists(tmpfile):
            os.remove(tmpfile)

if failing_scripts:
    eprint("There were scripts that failed when running with MPI:")
    for script, output in zip(failing_scripts, failing_outputs):
        error_str = "{}\n{}\n========================================"
        eprint(error_str.format(script, output))
    sys.exit(1)
