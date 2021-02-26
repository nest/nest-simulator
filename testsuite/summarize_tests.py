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

import junitparser as jp
import glob
import os
import sys


def parse_result_file(fname):

    results = jp.JUnitXml.fromfile(fname)

    failed_tests = ['.'.join((case.classname, case.name)) for case in results
                    if case.result and not isinstance(case.result, jp.junitparser.Skipped)]

    return {'Tests': results.tests,
            'Skipped': results.skipped,
            'Failures': results.failures,
            'Errors': results.errors,
            'Time': results.time,
            'Failed tests': failed_tests}


if __name__ == '__main__':

    assert len(sys.argv) == 2, 'summarize_tests must be called with TEST_OUTDIR.'

    test_outdir = sys.argv[1]

    results = {}
    totals = {'Tests': 0, 'Skipped': 0,
              'Failures': 0, 'Errors': 0,
              'Time': 0, 'Failed tests': []}

    for pfile in sorted(glob.glob(os.path.join(test_outdir, '*.xml'))):

        ph_name = os.path.splitext(os.path.split(pfile)[1])[0].replace('_', ' ')
        ph_res = parse_result_file(pfile)
        results[ph_name] = ph_res
        for k, v in ph_res.items():
            totals[k] += v

    cols = ['Tests', 'Skipped', 'Failures', 'Errors', 'Time']
    tline = '-' * (len(cols) * 10 + 20)

    print()
    print()
    print(tline)
    print('NEST Testsuite Results')

    print(tline)
    print('{:<20s}'.format('Phase'), end='')
    for c in cols:
        print('{:>10s}'.format(c), end='')
    print()

    print(tline)
    for pn, pr in results.items():
        print('{:<20s}'.format(pn), end='')
        for c in cols:
            fstr = '{:10.1f}' if c == 'Time' else '{:10d}'
            print(fstr.format(pr[c]), end='')
        print()

    print(tline)
    print('{:<20s}'.format('Total'), end='')
    for c in cols:
        fstr = '{:10.1f}' if c == 'Time' else '{:10d}'
        print(fstr.format(totals[c]), end='')
    print()
    print(tline)
    print()

    if totals['Failures'] + totals['Errors'] > 0:
        print('THE NEST TESTSUITE DISCOVERED PROBLEMS')
        print('    The following tests failed')
        for t in totals['Failed tests']:
            print('    | {:s}'.format(t))   # | marks line for parsing
        print()
        print('    Please report test failures by creating an issue at')
        print('        https://github.com/nest/nest_simulator/issues')
        print()
        print(tline)
        print()

        sys.exit(1)
    else:
        print('The NEST Testsuite passed successfully.')
        print()
        print(tline)
        print()
