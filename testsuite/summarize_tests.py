# -*- coding: utf-8 -*-
#
# summarize_tests.py
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

# Invoke this script as
#
#    python3 parse_test_output.py <path to test output>.xml
#
# It will print on a single line
#
# <No of tests run> <No of skipped tests> <No of failed tests> <No of errored tests> <List of unsuccessful tests>

import glob
import os
import sys

import junitparser as jp

assert int(jp.version.split(".")[0]) >= 2, "junitparser version must be >= 2"


def parse_result_file(fname):
    try:
        results = jp.JUnitXml.fromfile(fname)
    except Exception as err:
        return {
            "Tests": 1,
            "Skipped": 0,
            "Failures": 0,
            "Errors": 1,
            "Time": 0,
            "Failed tests": [f"ERROR: XML file {fname} not parsable with error {err}"],
        }

    if isinstance(results, jp.junitparser.JUnitXml):
        # special case for pytest, which wraps all once more
        suites = list(results)
        assert len(suites) == 1, "JUnit XML files may only contain results from a single testsuite."
        results = suites[0]

    assert all(len(case.result) == 1 for case in results if case.result), "Case result has unexpected length > 1"
    failed_tests = [
        ".".join((case.classname, case.name))
        for case in results
        if case.result and not isinstance(case.result[0], jp.junitparser.Skipped)
    ]

    return {
        "Tests": results.tests,
        "Skipped": results.skipped,
        "Failures": results.failures,
        "Errors": results.errors,
        "Time": results.time,
        "Failed tests": failed_tests,
    }


if __name__ == "__main__":
    assert len(sys.argv) == 2, "summarize_tests must be called with TEST_OUTDIR."

    test_outdir = sys.argv[1]

    results = {}
    totals = {"Tests": 0, "Skipped": 0, "Failures": 0, "Errors": 0, "Time": 0, "Failed tests": []}

    for pfile in sorted(glob.glob(os.path.join(test_outdir, "*.xml"))):
        ph_name = os.path.splitext(os.path.split(pfile)[1])[0].replace("_", " ")
        try:
            ph_res = parse_result_file(pfile)
            results[ph_name] = ph_res
            for k, v in ph_res.items():
                totals[k] += v
        except Exception as err:
            msg = f"ERROR: {pfile} not parsable with error {err}"
            results[ph_name] = {"Tests": 0, "Skipped": 0, "Failures": 0, "Errors": 0, "Time": 0, "Failed tests": [msg]}
            totals["Failed tests"].append(msg)

    cols = ["Tests", "Skipped", "Failures", "Errors", "Time"]

    col_w = max(len(c) for c in cols) + 2
    first_col_w = max(len(k) for k in results.keys())

    tline = "-" * (len(cols) * col_w + first_col_w)

    print()
    print()
    print(tline)
    print("NEST Testsuite Results")

    print(tline)
    print(f"{'Phase':<{first_col_w}s}", end="")
    for c in cols:
        print(f"{c:>{col_w}s}", end="")
    print()

    print(tline)
    for pn, pr in results.items():
        print(f"{pn:<{first_col_w}s}", end="")
        if pr["Tests"] == 0 and pr["Failed tests"]:
            print(f"{'--- XML PARSING FAILURE ---':^{len(cols) * col_w}}")
        else:
            for c in cols:
                fmt = ".1f" if c == "Time" else "d"
                print(f"{pr[c]:{col_w}{fmt}}", end="")
            print()

    print(tline)
    print(f"{'Total':<{first_col_w}s}", end="")
    for c in cols:
        fmt = ".1f" if c == "Time" else "d"
        print(f"{totals[c]:{col_w}{fmt}}", end="")
    print()
    print(tline)
    print()

    # Consistency check
    assert totals["Failures"] + totals["Errors"] == len(totals["Failed tests"])

    if totals["Failures"] + totals["Errors"] > 0:
        print("THE NEST TESTSUITE DISCOVERED PROBLEMS")
        print("    The following tests failed")
        for t in totals["Failed tests"]:
            print(f"    | {t}")  # | marks line for parsing
        print()
        print("    Please report test failures by creating an issue at")
        print("        https://github.com/nest/nest-simulator/issues")
        print()
        print(tline)
        print()

        sys.exit(1)
    else:
        print("The NEST Testsuite passed successfully.")
        print()
        print(tline)
        print()
