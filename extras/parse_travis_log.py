# -*- coding: utf-8 -*-
#
# parse_travis_log.py
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
This script parses the TravisCI output as it is generated
by ./build.sh and outputs a shorter summary. It is hard-coded
to the output of ./build.sh, make installcheck,
and cmake -DCMAKE_INSTALL_PREFIX=... . Changing any of those, requires
adapting this script.
"""

INDENT = "  "


def process_until(f, final):
    """
    Run through the file f until a stripped line is equal to final.
    """
    while True:
        line = f.readline()
        if not line:
            return False, line

        if line.strip() == final:
            return True, line


def process_installcheck(f):
    """
    Parse the `make installcheck` output and return the number of tests
    and failed tests. Especially, it parses the output after
    'NEST Testsuite Summary'.
    """
    res, line = process_until(f, 'NEST Testsuite Summary')
    if res:
        f.readline()  # -------
        f.readline()  # NEST Executable: ...
        f.readline()  # SLI Executable: ...
        line = f.readline()  # Total number of tests: <int>
        total = int(line.split(' ')[-1])
        f.readline()  # Passed: <int>
        line = f.readline()  # Failed: <int> (<int> PyNEST)
        failed = [int(s) for s in line.split() if s.isdigit()][0]
        return total, failed, line


def process_changed_files(f):
    """
    Run through the file f until a line starts with '+file_names='.
    The equal sign is followed by a space separated list of changed files
    in the PR. Returns that list.
    """
    while True:
        line = f.readline()
        if not line:
            return False, line

        if line.startswith('+file_names='):
            return (filter(lambda x: x != '',
                           line.strip().split('=')[1].split(' ')), line)


def process_vera(f, filename):
    """
    Process vera++ output for a certain file.
    """
    d = {}
    res = {'vera++': d}
    while True:
        line = f.readline()
        if not line:
            return res

        # Exit condition: after vera++, cppcheck is executed
        if line.startswith('+cppcheck '):
            return res

        # Exit condition: all code checks finished, building starts
        if line.startswith('+cd build'):
            return res

        if line.startswith(filename):
            key = line.split(":")[-1].strip()
            if key not in d:
                d[key] = 0
            d[key] += 1


def process_cppcheck(f, filename):
    """
    Process cppcheck output for a certain file.
    """
    d = {}
    res = {'cppcheck': d}
    while True:
        line = f.readline()
        if not line:
            return res

        # Exit condition: after cppcheck, clang-format is executed
        if line.startswith('+clang-format'):
            return res

        # Exit condition: all code checks finished, building starts
        if line.startswith('+cd build'):
            return res

        if line.startswith('[' + filename):
            key = line[line.find('('):].strip()
            # ignore 'is never used' items
            if 'is never used' in key:
                continue
            # ignore '(information)' items
            if '(information)' in key:
                continue
            if key not in d:
                d[key] = 0   # first seen here
            d[key] += 1


def process_clang_format(f, filename):
    """
    Process clang-format output for a certain file.
    """
    d = {'Ok?': False}
    res = {'clang-format': d}
    while True:
        line = f.readline()
        if not line:
            return res

        # The diff is stored in a file, that is `cat` to the screen.
        if line.startswith('+cat '):
            line = f.readline()      # next line is file start
            diff = ''
            # when the file is finished `cat`ing, it will be removed
            # until then, concatenate the diff.
            while not line.startswith("+rm"):
                diff += line
                line = f.readline()
            if diff == '':
                d['Ok?'] = True
            else:
                d['diff'] = ('\n##############################\n' + diff +
                             '##############################')

            return res


def process_static_analysis(f, line):
    """
    Process static analysis output for a certain file.
    """
    filename = line.split(' ')[-1].strip()[0:-1]
    d = {}
    res = {filename: d}
    while True:
        line = f.readline()
        if not line:
            return res

        # Exit condition: next file is analyzed (C/CPP)
        if line.startswith('+echo Static analysis on file '):
            return res

        # Exit condition: next file is analyzed (PY)
        if line.startswith('+echo Check PEP8 on file '):
            return res

        # Exit condition: static analysis finished, remove compiled cppcheck
        if line.startswith('+rm -rf ./cppcheck'):
            return res

        # Exit condition: all code checks finished, building starts
        if line.startswith('+cd build'):
            return res

        # analyze vera++
        if line.startswith(' - vera++ for '):
            d.update(process_vera(f, filename))

        # analyze cppcheck
        if line.startswith(' - cppcheck for '):
            d.update(process_cppcheck(f, filename))

        # analyze clang-format
        if line.startswith(' - clang-format for '):
            d.update(process_clang_format(f, filename))


def process_pep8(f, line):
    """
    Process PEP8 output for a certain file.
    """
    # Check PEP8 on file path/to/python/file.py:
    filename = line.split(' ')[-1].strip()[0:-1]
    d = set()
    res = {filename: d}
    while True:
        line = f.readline()
        if not line:
            return res

        # Exit condition: next file is analyzed (C/CPP)
        if line.startswith('+echo Static analysis on file '):
            return res

        # Exit condition: next file is analyzed (PY)
        if line.startswith('+echo Check PEP8 on file '):
            return res

        # Exit condition: static analysis finished, remove compiled cppcheck
        if line.startswith('+rm -rf ./cppcheck'):
            return res

        # Exit condition: all code checks finished, building starts
        if line.startswith('+cd build'):
            return res

        # Analysis of filename starts here
        if line.startswith('+echo ' + filename):
            while True:
                line = f.readline()
                # Exit condition: read to far
                if line.startswith('+format_error_files='):
                    break
                d.add(line.strip())


def print_static_analysis(d):
    """
    Print a well formatted version of the dict returned by
    process_static_analysis(f, line).
    """
    for k1, v1 in d.iteritems():
        print(INDENT + ' --' + '-' * len(k1))
        print(INDENT + ' | ' + k1 + ':')
        for k2, v2 in v1.iteritems():
            print(INDENT + ' | ' + INDENT + ' * ' + k2 + ':')
            for k3, v3 in v2.iteritems():
                print(INDENT + ' | ' + 2 * INDENT +
                      ' - ' + k3 + ' : ' + str(v3))


def count_warnings_errors(f):
    """
    Counts compiler warnings and errors. Stops when reading '+make install'.
    """
    warn = {}
    error = {}
    while True:
        line = f.readline()
        if not line:
            return warn, error, line

        if line.strip() == '+make install':
            return warn, error, line

        if ': warning:' in line:
            file_name = line.split(':')[0]
            if file_name not in warn:
                warn[file_name] = 0
            warn[file_name] += 1

        if ': error:' in line:
            file_name = line.split(':')[0]
            if file_name not in error:
                error[file_name] = 0
            error[file_name] += 1


def print_pep8(d):
    """
    Print a well formatted version of the dict returned by
    process_pep8(f, line).
    """
    for k1, v1 in d.iteritems():
        print(INDENT + ' --' + '-' * len(k1))
        print(INDENT + ' | ' + k1 + ':')
        for v2 in v1:
            print(INDENT + ' | ' + INDENT + v2)


if __name__ == '__main__':
    from sys import argv, exit

    script, filename = argv

    configure_ok = False
    warnings = {}
    errors = {}
    make_install_ok = False
    make_installcheck_all = 0
    make_installcheck_failed = -1
    vera_init = False
    cppcheck_init = False
    changed_files = False
    static_analysis = {}
    pep8_analysis = {}
    uploading_results = True

    with open(filename, 'r') as f:
        while True:
            line = f.readline()
            if not line:
                break

            if not vera_init and line.startswith('+mkdir -p vera_home'):
                vera_init, line = process_until(f, '+cat')

            if not cppcheck_init and line.startswith(
                    '+git clone https://github.com/danmar/cppcheck.git'):
                cppcheck_init, line = process_until(f, 'Cppcheck 1.69')

            if not changed_files and line.strip() == 'Extract changed files..':
                changed_files, line = process_changed_files(f)

            if line.startswith('Static analysis on file '):
                static_analysis.update(process_static_analysis(f, line))

            if line.startswith('Check PEP8 on file '):
                pep8_analysis.update(process_pep8(f, line))

            if (not configure_ok and
                    line.startswith('+cmake -DCMAKE_INSTALL_PREFIX=')):
                configure_ok, line = process_until(
                    f, 'You can now build and install NEST with')

            if line.strip() == '+make VERBOSE=1':
                warnings, errors, line = count_warnings_errors(f)

            if not make_install_ok and line.startswith('+make install'):
                make_install_ok, line = process_until(f, '+make installcheck')

            if make_installcheck_failed == -1 and line.startswith(
                    '+make installcheck'):
                make_installcheck_all, make_installcheck_failed, line = (
                    process_installcheck(f))
            if line.startswith('WARNING: Not uploading results as this is a'):
                uploading_results = False

            if 'Skipping a deployment with the s3 provider because' in line:
                uploading_results = False

    # post process values
    actual_warnings = {k.split("nest-simulator/")[1]: v
                       for k, v in warnings.iteritems()
                       if k.startswith('/home/travis/build')}
    sum_of_warnings = sum([v for k, v in actual_warnings.iteritems()])

    sum_of_errors = sum([v for k, v in errors.iteritems()])

    print("\n--------<<<<<<<< Summary of TravisCI >>>>>>>>--------")
    print("Vera init:           " + ("Ok" if vera_init else "Error"))
    print("Cppcheck init:       " + ("Ok" if cppcheck_init else "Error"))
    print("Changed files:       " + str(changed_files))
    print("Formatting:          " +
          ("Ok" if all([i['clang-format']['Ok?']
                        for i in static_analysis.itervalues()]) else "Error"))
    print("PEP8:                " +
          ("Ok" if all([len(v) == 0
                        for v in pep8_analysis.values()]) else "Error"))
    print("Configure:           " + ("Ok" if configure_ok else "Error"))
    print("Make:                " + ("Ok" if sum_of_errors == 0 else
                                     "Error(" + str(sum_of_errors) + ")") +
          " ( " + str(sum_of_warnings) + " warnings ).")
    print("Make install:        " + ("Ok" if make_install_ok else "Error"))
    print("Make installcheck:   " + ("Ok (" if make_installcheck_failed == 0
                                     else "Error (") +
          str(make_installcheck_failed) + " / " +
          str(make_installcheck_all) + ")")
    print("Logs uploaded to S3: " + ("Yes" if uploading_results else "No"))

    if static_analysis:
        print("\nStatic analysis:")
        print_static_analysis(static_analysis)

    if actual_warnings:
        print("\nWarnings:")
        for k, v in actual_warnings.iteritems():
            print(" - {}: {}".format(k, v))

    # Print PEP8 analysis only if files have been checked and problems found
    if pep8_analysis and max(len(res) for res in pep8_analysis.values()) > 0:
        print("\nPEP8 analysis: (only first occurrence)")
        print_pep8(pep8_analysis)

    print("\n--------<<<<<<<< Summary of TravisCI >>>>>>>>--------")

    if not (vera_init and
            cppcheck_init and
            configure_ok and
            sum_of_errors == 0 and
            make_install_ok and
            make_installcheck_failed == 0 and
            all([i['clang-format']['Ok?']
                 for i in static_analysis.itervalues()]) and
            all([len(v) == 0 for v in pep8_analysis.values()])):
        exit(1)
