# -*- coding: utf-8 -*-
#
# mpi_tests.py
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
import tempfile


def check_output(cmd):

    cmd = shlex.split(cmd)
    stdout = subprocess.STDOUT
    output = subprocess.check_output(cmd)
    return output.decode("utf-8")


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


failing = False

failure_xml = """<?xml version="1.0" encoding="utf-8"?>\
<testsuite name="pytest" failures="1" tests="1" time="1">\
<testcase classname="pynest.mpi_tests.{0}" file="{0}">\
<system-out>{1}</system-out>\
</testcase>\
</testsuite>\
"""

junitxml_filename = sys.argv[1]
pytest_options = sys.argv[2]

with open(junitxml_filename, "w+") as junitxml:
    for script_name in scripts:
        script = os.path.join(script_dir, script_name)
        tmpfile = tempfile.mktemp(".xml")
        pytest_cmd = "python {} -v --junitxml={}".format(pytest_options, tmpfile)
        cmd = "sli -c '2 ({}) ({}) mpirun =only'".format(pytest_cmd, script)
        test_cmd = check_output(cmd)

        try:
            eprint(script_name)
            output = check_output(test_cmd)
        except subprocess.CalledProcessError as e:
            failing = True
            eprint(e.output)
            with open(tmpfile, "w") as errfile:
                errfile.write(failure_xml.format(script_name, repr(e.output)))

        junitxml.write(repr(open(tmpfile, "r").read()))

        if os.path.exists(tmpfile):
            os.remove(tmpfile)

if failing:
    sys.exit(1)
